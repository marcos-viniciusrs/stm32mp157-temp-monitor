/** @file main.cpp
 *  @brief Aquisição contínua do ADC a 4 Hz, com log opcional no terminal.
 */
#include "TemperatureSensor.hpp"
#include "UdpSender.hpp"

#include <exception>
#include <chrono>
#include <memory>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <thread>

/** @brief Encapsula os argumentos e a apresentação da temperatura. */
class SensorApplication {
public:
    /** @brief Lê o ADC a cada 250 ms até interrupção ou erro.
     *  @param argc Número de argumentos.
     *  @param argv Nome, caminho IIO, maior código do ADC e opção --log.
     *  @return Um em erro de aquisição/conversão ou dois em uso/configuração incorretos.
     *  @note Use Ctrl+C para interromper. Sem --log, apenas erros são exibidos.
     */
    static int run(int argc, char* argv[]) {
        if (argc < 3) {
            std::cerr << "Uso: " << argv[0] << " <arquivo_raw> <adc_max> [--log] [--beta K] [--r0 ohms] [--series ohms] [--ntc gnd|vcc] [--udp IP PORTA]\n";
            return 2;
        }
        try {
            const std::string maxArgument(argv[2]);
            std::size_t consumed = 0;
            const int adcMax = std::stoi(maxArgument, &consumed);
            if (consumed != maxArgument.size()) {
                throw std::invalid_argument("adc_max deve ser um inteiro.");
            }
            if (adcMax < 1) throw std::invalid_argument("adc_max deve ser positivo.");
            ThermistorConfig config;
            bool logEnabled = false;
            std::unique_ptr<UdpSender> udp;
            for (int i = 3; i < argc; ++i) {
                const std::string option(argv[i]);
                if (option == "--log") { logEnabled = true; continue; }
                if (option == "--udp") {
                    if (i + 2 >= argc) throw std::invalid_argument("--udp exige IP e porta.");
                    const std::string address(argv[++i]);
                    const std::string portText(argv[++i]);
                    std::size_t portUsed = 0;
                    const unsigned long port = std::stoul(portText, &portUsed);
                    if (portUsed != portText.size() || port < 1 || port > 65535)
                        throw std::invalid_argument("Porta UDP invalida.");
                    udp.reset(new UdpSender(address, static_cast<std::uint16_t>(port)));
                    continue;
                }
                if (option != "--beta" && option != "--r0" &&
                    option != "--series" && option != "--ntc") {
                    throw std::invalid_argument("Opcao desconhecida: " + option);
                }
                if (++i >= argc) throw std::invalid_argument("Falta valor para " + option);
                const std::string argument(argv[i]);
                if (option == "--ntc") {
                    if (argument != "gnd" && argument != "vcc")
                        throw std::invalid_argument("--ntc deve ser gnd ou vcc.");
                    config.ntcToGround = argument == "gnd";
                } else {
                    const double value = std::stod(argument, &consumed);
                    if (consumed != argument.size())
                        throw std::invalid_argument("Valor numerico invalido: " + argument);
                    if (option == "--beta") config.beta = value;
                    if (option == "--r0") config.nominalResistance = value;
                    if (option == "--series") config.seriesResistance = value;
                }
            }
            const TemperatureSensor sensor(argv[1], adcMax, config);
            return acquire(sensor, logEnabled, udp.get());
        } catch (const std::exception& error) {
            std::cerr << "Erro de uso: " << error.what() << '\n';
            return 2;
        }
    }

private:
    /** @brief Executa aquisição e conversão da mesma amostra a 4 Hz.
     *  @param sensor Fonte das amostras ADC.
     *  @param logEnabled Habilita saída de ADC e °C.
     *  @param udp Destino UDP opcional para cada temperatura.
     *  @return Um em erro de aquisição ou conversão.
     */
    static int acquire(const TemperatureSensor& sensor, bool logEnabled, const UdpSender* udp) {
        try {
            const auto period = std::chrono::milliseconds(250);
            auto nextRead = std::chrono::steady_clock::now();
            while (true) {
                const int raw = sensor.readRaw();
                const double celsius = sensor.toCelsius(raw);
                if (udp) udp->send(celsius);
                if (logEnabled) {
                    std::cout << "ADC: " << raw << " | Temperatura: "
                              << std::fixed << std::setprecision(2) << celsius
                              << " °C" << std::endl;
                }
                nextRead += period;
                const auto now = std::chrono::steady_clock::now();
                // Evita rajadas de leituras se a aquisição ou a saída atrasar.
                if (nextRead <= now) {
                    nextRead = now + period;
                }
                std::this_thread::sleep_until(nextRead);
            }
        } catch (const std::exception& error) {
            std::cerr << "Erro: " << error.what() << '\n';
            return 1;
        }
    }
};

/** @brief Ponto de entrada; delega a execução à classe da aplicação.
 *  @param argc Número de argumentos.
 *  @param argv Argumentos de linha de comando.
 *  @return Código de saída da aplicação.
 */
int main(int argc, char* argv[]) {
    return SensorApplication::run(argc, argv);
}

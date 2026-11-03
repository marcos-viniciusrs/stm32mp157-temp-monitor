/** @file main.cpp
 *  @brief Aquisição contínua do ADC a 4 Hz, com log opcional no terminal.
 */
#include "TemperatureSensor.hpp"

#include <exception>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>

/** @brief Encapsula os argumentos e a apresentação da leitura bruta. */
class SensorApplication {
public:
    /** @brief Lê o ADC a cada 250 ms até interrupção ou erro.
     *  @param argc Número de argumentos.
     *  @param argv Nome, caminho IIO, maior código do ADC e opção --log.
     *  @return Um em erro de aquisição/configuração ou dois em uso incorreto.
     *  @note Use Ctrl+C para interromper. Sem --log, apenas erros são exibidos.
     */
    static int run(int argc, char* argv[]) {
        if ((argc != 3 && argc != 4) ||
            (argc == 4 && std::string(argv[3]) != "--log")) {
            std::cerr << "Uso: " << argv[0] << " <arquivo_raw> <adc_max> [--log]\n";
            return 2;
        }
        try {
            const std::string maxArgument(argv[2]);
            std::size_t consumed = 0;
            const int adcMax = std::stoi(maxArgument, &consumed);
            if (consumed != maxArgument.size()) {
                throw std::invalid_argument("adc_max deve ser um inteiro.");
            }
            const TemperatureSensor sensor(argv[1], adcMax);
            const bool logEnabled = argc == 4;
            const auto period = std::chrono::milliseconds(250);
            auto nextRead = std::chrono::steady_clock::now();
            while (true) {
                const int raw = sensor.readRaw();
                if (logEnabled) {
                    std::cout << "ADC: " << raw << std::endl;
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

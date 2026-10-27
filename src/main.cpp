/** @file main.cpp
 *  @brief Exemplo de aquisição de uma amostra bruta do ADC.
 */
#include "TemperatureSensor.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

/** @brief Encapsula os argumentos e a apresentação da leitura bruta. */
class SensorApplication {
public:
    /** @brief Executa uma leitura do ADC e apresenta seu código bruto.
     *  @param argc Número de argumentos.
     *  @param argv Nome do programa, caminho IIO e maior código do ADC.
     *  @return Zero em sucesso, um em erro e dois em quantidade incorreta de argumentos.
     */
    static int run(int argc, char* argv[]) {
        if (argc != 3) {
            std::cerr << "Uso: " << argv[0] << " <arquivo_raw> <adc_max>\n";
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
            const int raw = sensor.readRaw();
            std::cout << "ADC: " << raw << '\n';
            return 0;
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

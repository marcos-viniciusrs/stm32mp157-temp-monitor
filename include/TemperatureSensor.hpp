/** @file TemperatureSensor.hpp
 *  @brief Aquisição da leitura bruta do KY-013 pelo ADC Linux IIO.
 */
#ifndef TEMPERATURE_SENSOR_HPP
#define TEMPERATURE_SENSOR_HPP

#include <string>

/** @brief Lê o valor bruto do ADC conectado ao sensor KY-013.
 *
 * O driver IIO e o canal ADC devem estar configurados no Linux da placa.
 * Cada chamada a readRaw() reabre o arquivo do canal para obter uma nova amostra.
 * O resultado é um código inteiro do ADC, sem unidade física.
 */
class TemperatureSensor {
public:
    /** @brief Guarda e valida os parâmetros de aquisição, sem acessar o hardware.
     *  @param rawPath Caminho completo do arquivo IIO in_voltageX_raw.
     *  @param adcMax Maior código do ADC: 2^N - 1 para resolução de N bits.
     *  @throws std::invalid_argument Se o caminho estiver vazio ou adcMax for menor que um.
     */
    TemperatureSensor(std::string rawPath, int adcMax);

    /** @brief Adquire uma amostra bruta do canal ADC.
     *  @return Inteiro entre zero e adcMax, inclusive.
     *  @throws std::runtime_error Se o arquivo estiver inacessível ou malformado.
     *  @throws std::out_of_range Se o código estiver fora da escala configurada.
     *  @note Zero e fundo de escala são leituras válidas nesta etapa.
     */
    int readRaw() const;

private:
    std::string rawPath_; /**< Caminho do arquivo de leitura do canal IIO. */
    int adcMax_; /**< Maior código permitido pela configuração do ADC. */
};

#endif

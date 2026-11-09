/** @file TemperatureSensor.hpp
 *  @brief Aquisição e conversão de temperatura do KY-013 pelo ADC Linux IIO.
 */
#ifndef TEMPERATURE_SENSOR_HPP
#define TEMPERATURE_SENSOR_HPP

#include <string>

/** @brief Parâmetros do modelo Beta; confirmar no módulo utilizado.
 *  @note A alimentação do divisor deve ser igual à referência do ADC.
 */
struct ThermistorConfig {
    double beta = 3950.0; /**< Coeficiente Beta em kelvin. */
    // Ajuste de um ponto: ADC 55321/65535 = 22 °C, Beta 3950, Rf 10k, NTC para GND.
    double nominalResistance = 47338.20493605491; /**< R0 efetivo calibrado a 25 °C, em ohms; não é uma medição do componente. */
    double seriesResistance = 10000.0; /**< Resistor fixo do divisor, em ohms. */
    bool ntcToGround = true; /**< true: NTC entre sinal e GND; false: sinal e VCC. */
};

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
     *  @param config Parâmetros positivos e finitos do modelo Beta.
     *  @throws std::invalid_argument Se o caminho estiver vazio ou adcMax for menor que um.
     */
    TemperatureSensor(std::string rawPath, int adcMax,
                      ThermistorConfig config = ThermistorConfig{});

    /** @brief Adquire uma amostra bruta do canal ADC.
     *  @return Inteiro entre zero e adcMax, inclusive.
     *  @throws std::runtime_error Se o arquivo estiver inacessível ou malformado.
     *  @throws std::out_of_range Se o código estiver fora da escala configurada.
     *  @note Zero e fundo de escala são leituras válidas nesta etapa.
     */
    int readRaw() const;

    /** @brief Converte uma amostra para °C sem realizar outra leitura.
     *  @param raw Código ADC estritamente entre zero e adcMax.
     *  @return Temperatura estimada em graus Celsius pelo modelo Beta.
     *  @throws std::out_of_range Se a amostra estiver nos limites ou fora da escala.
     *  @throws std::runtime_error Se o cálculo não produzir temperatura física finita.
     */
    double toCelsius(int raw) const;

private:
    ThermistorConfig config_; /**< Parâmetros do divisor e do NTC. */
    std::string rawPath_; /**< Caminho do arquivo de leitura do canal IIO. */
    int adcMax_; /**< Maior código permitido pela configuração do ADC. */
};

#endif

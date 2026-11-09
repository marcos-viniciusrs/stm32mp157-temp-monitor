/** @file TemperatureSensor.cpp
 *  @brief Implementação da leitura bruta do ADC via arquivo IIO.
 */
#include "TemperatureSensor.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>
#include <utility>

TemperatureSensor::TemperatureSensor(std::string rawPath, int adcMax,
                                     ThermistorConfig config)
    : config_(config), rawPath_(std::move(rawPath)), adcMax_(adcMax) {
    for (double value : {config_.beta, config_.nominalResistance, config_.seriesResistance}) {
        if (!std::isfinite(value) || value <= 0.0) {
            throw std::invalid_argument("Parametros do NTC devem ser positivos e finitos.");
        }
    }
    if (rawPath_.empty() || adcMax_ < 1) {
        throw std::invalid_argument("Informe um caminho ADC e uma escala positiva.");
    }
}

int TemperatureSensor::readRaw() const {
    std::ifstream input(rawPath_);
    if (!input.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o ADC: " + rawPath_);
    }

    // O arquivo deve conter um inteiro; espacos e quebras de linha sao aceitos.
    long long raw = 0;
    if (!(input >> raw)) {
        throw std::runtime_error("Leitura do ADC ausente ou invalida: " + rawPath_);
    }
    input >> std::ws;
    if (input.bad() || !input.eof()) {
        throw std::runtime_error("Conteudo inesperado no arquivo do ADC: " + rawPath_);
    }
    if (raw < 0 || raw > adcMax_) {
        throw std::out_of_range("Leitura do ADC fora da escala configurada.");
    }
    return static_cast<int>(raw);
}


double TemperatureSensor::toCelsius(int raw) const {
    if (raw <= 0 || raw >= adcMax_) {
        throw std::out_of_range("ADC nos limites ou fora da escala: temperatura indefinida.");
    }
    // VCC = VREF: a razao de tensoes e raw/adcMax.
    double ratio = static_cast<double>(raw) / (adcMax_ - raw);
    if (!config_.ntcToGround) ratio = 1.0 / ratio;
    // Logaritmos separados evitam overflow no produto das resistencias.
    const double logResistanceRatio = std::log(config_.seriesResistance)
        - std::log(config_.nominalResistance) + std::log(ratio);
    const double inverseKelvin = 1.0 / 298.15 + logResistanceRatio / config_.beta;
    const double kelvin = 1.0 / inverseKelvin;
    if (!std::isfinite(kelvin) || kelvin <= 0.0) {
        throw std::runtime_error("Temperatura invalida para os parametros do NTC.");
    }
    return kelvin - 273.15;
}

/** @file TemperatureSensor.cpp
 *  @brief Implementação da leitura bruta do ADC via arquivo IIO.
 */
#include "TemperatureSensor.hpp"

#include <fstream>
#include <stdexcept>
#include <utility>

TemperatureSensor::TemperatureSensor(std::string rawPath, int adcMax)
    : rawPath_(std::move(rawPath)), adcMax_(adcMax) {
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

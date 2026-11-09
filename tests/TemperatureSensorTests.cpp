#include "TemperatureSensor.hpp"
#include <cmath>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unistd.h>

// Fixture exclusiva por execução e removida automaticamente ao sair do escopo.
class AdcFixture {
public:
    AdcFixture() {
        char pattern[] = "/tmp/ky013-test-XXXXXX";
        const int descriptor = mkstemp(pattern);
        if (descriptor < 0) throw std::runtime_error("Falha ao criar fixture.");
        close(descriptor);
        path_ = pattern;
    }
    ~AdcFixture() { std::remove(path_.c_str()); }
    AdcFixture(const AdcFixture&) = delete;
    AdcFixture& operator=(const AdcFixture&) = delete;
    const std::string& path() const { return path_; }
    void write(const std::string& value) const {
        std::ofstream output(path_);
        output << value;
        output.close();
        if (!output) throw std::runtime_error("Falha ao escrever fixture.");
    }
private:
    std::string path_;
};

void near(double actual, double expected, double tolerance) {
    if (!std::isfinite(actual) || std::abs(actual - expected) > tolerance)
        throw std::runtime_error("Conversao incorreta: " + std::to_string(actual)
                                 + " esperado " + std::to_string(expected));
}

template<class Exception, class Function>
void rejects(Function function) {
    try { function(); } catch (const Exception&) { return; }
    throw std::runtime_error("Entrada invalida nao gerou erro.");
}

class TemperatureSensorTests {
public:
    static int run() {
        try {
            AdcFixture adc;
            const TemperatureSensor sensor(adc.path(), 4095);

            // Leitura normal e limites da escala.
            for (const int value : {2048, 0, 4095}) {
                adc.write(std::to_string(value) + "\n");
                if (sensor.readRaw() != value) {
                    throw std::runtime_error("Valor lido diferente do esperado.");
                }
            }

            // O arquivo deve ser lido novamente a cada aquisicao.
            adc.write("1000\n");
            if (sensor.readRaw() != 1000) {
                throw std::runtime_error("A leitura nao foi atualizada.");
            }

            // Uma falha de acesso deve ser informada ao chamador.
            const TemperatureSensor missing(adc.path() + "/ausente", 4095);
            bool failed = false;
            try {
                missing.readRaw();
            } catch (const std::runtime_error&) {
                failed = true;
            }
            if (!failed) {
                throw std::runtime_error("Arquivo inacessivel nao gerou erro.");
            }

            // Pontos analíticos independentes do conversor: R/R0 = 1, 3, 1/3.
            ThermistorConfig nominal;
            nominal.nominalResistance = 10000;
            const TemperatureSensor model(adc.path(), 4000, nominal);
            near(model.toCelsius(2000), 25.0, 1e-9);
            near(model.toCelsius(3000), 2.16931942031232, 1e-6);
            near(model.toCelsius(1000), 51.95949982577457, 1e-6);
            const TemperatureSensor adc16(adc.path(), 65535);
            near(adc16.toCelsius(55321), 22.0, 1e-9);
            if (!(adc16.toCelsius(55300) > 22.0 && adc16.toCelsius(55340) < 22.0))
                throw std::runtime_error("Sentido da curva calibrada incorreto.");
            ThermistorConfig config = nominal;
            config.ntcToGround = false;
            const TemperatureSensor inverted(adc.path(), 4000, config);
            near(inverted.toCelsius(1000), model.toCelsius(3000), 1e-9);
            near(inverted.toCelsius(3000), model.toCelsius(1000), 1e-9);
            config.ntcToGround = true;
            config.seriesResistance = 30000;
            near(TemperatureSensor(adc.path(), 4000, config).toCelsius(1000), 25.0, 1e-9);
            config.seriesResistance = 10000;
            config.nominalResistance = 30000;
            near(TemperatureSensor(adc.path(), 4000, config).toCelsius(3000), 25.0, 1e-9);
            config = nominal;
            config.beta = 3435;
            near(TemperatureSensor(adc.path(), 4000, config).toCelsius(3000),
                 -0.955634820897501, 1e-6);
            for (int raw : {-1, 0, 4000, 4001}) {
                rejects<std::out_of_range>([&] { model.toCelsius(raw); });
            }
            for (double value : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                                 std::numeric_limits<double>::quiet_NaN()}) {
                config = nominal;
                config.beta = value;
                rejects<std::invalid_argument>([&] { TemperatureSensor t(adc.path(), 4000, config); });
                config = nominal;
                config.nominalResistance = value;
                rejects<std::invalid_argument>([&] { TemperatureSensor t(adc.path(), 4000, config); });
                config = nominal;
                config.seriesResistance = value;
                rejects<std::invalid_argument>([&] { TemperatureSensor t(adc.path(), 4000, config); });
            }
            config = nominal;
            config.beta = 1;
            rejects<std::runtime_error>([&] {
                TemperatureSensor(adc.path(), 4000, config).toCelsius(1000);
            });
            for (const std::string value : {"", "abc", "10x", "1 2", "-1", "4096"}) {
                adc.write(value);
                rejects<std::exception>([&] { sensor.readRaw(); });
            }
            std::cout << "OK: aquisicao, modelo Beta, orientacoes, escalas e erros.\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "FALHA: " << error.what() << '\n';
            return 1;
        }
    }
};

int main() { return TemperatureSensorTests::run(); }

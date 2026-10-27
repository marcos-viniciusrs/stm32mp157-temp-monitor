#include "TemperatureSensor.hpp"
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

            std::cout << "OK: leitura, atualizacao e falha de acesso.\n";
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "FALHA: " << error.what() << '\n';
            return 1;
        }
    }
};

int main() { return TemperatureSensorTests::run(); }

// OLED SSD1306 128x64: ADC e temperatura da mesma amostra, a 2 Hz.
// Conversao compartilhada com temp_monitor; ver docs/OLED.md.
#include "TemperatureSensor.hpp"
#include <array>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <csignal>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <unistd.h>

class OledApplication {
    using Frame = std::array<std::uint8_t, 1024>;

// Fonte original 3x5, ampliada em 2x. '*' representa o glifo de grau.
    static std::array<unsigned, 5> glyph(char c) {
    switch (c) {
    case '0': return {{7,5,5,5,7}}; case '1': return {{2,6,2,2,7}};
    case '2': return {{7,1,7,4,7}}; case '3': return {{7,1,7,1,7}};
    case '4': return {{5,5,7,1,1}}; case '5': return {{7,4,7,1,7}};
    case '6': return {{7,4,7,5,7}}; case '7': return {{7,1,2,2,2}};
    case '8': return {{7,5,7,5,7}}; case '9': return {{7,5,7,1,7}};
    case 'A': return {{2,5,7,5,5}}; case 'D': return {{6,5,5,5,6}};
    case 'C': return {{7,4,4,4,7}}; case 'T': return {{7,2,2,2,2}};
    case 'E': return {{7,4,6,4,7}}; case 'M': return {{5,7,7,5,5}};
    case 'P': return {{6,5,6,4,4}}; case 'R': return {{6,5,6,5,5}};
    case 'O': return {{7,5,5,5,7}};
    case 'B': return {{6,5,6,5,6}}; case 'U': return {{5,5,5,5,7}};
    case ':': return {{0,2,0,2,0}}; case '.': return {{0,0,0,0,2}};
    case '-': return {{0,0,7,0,0}}; case '*': return {{2,5,2,0,0}};
    default: return {{0,0,0,0,0}};
    }
}
    static void pixel(Frame& frame, int x, int y) {
    if (x < 0 || x >= 128 || y < 0 || y >= 64)
        throw std::out_of_range("Pixel fora do OLED.");
    frame.at(x + (y/8)*128) |= 1u << (y%8);
}
    static void text(Frame& frame, int center, int y, const std::string& value, int scale) {
    const int width = (static_cast<int>(value.size())*4-1)*scale;
    int x = center - width/2;
    for (char c : value) {
        const auto rows = glyph(c);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 3; ++col)
                if (rows[row] & (1u << (2-col)))
                    for (int dy = 0; dy < scale; ++dy)
                        for (int dx = 0; dx < scale; ++dx)
                            pixel(frame, x+col*scale+dx, y+row*scale+dy);
        x += 4*scale;
    }
}
    static void valueText(Frame& frame, int center, std::string value) {
    // Cinco caracteres em 3x; negativos/valores maiores usam 2x, sem cortar.
    if (value.size() > 7) value = "ERRO";
    const int scale = value.size() <= 5 ? 3 : 2;
    text(frame, center, 36-(5*scale)/2, value, scale);
}
    static Frame sampleFrame(const TemperatureSensor& sensor) {
    std::string rawText = "ERRO", temperatureText = "--";
    try {
        const int raw = sensor.readRaw();
        rawText = std::to_string(raw);
        try {
            std::ostringstream output;
            output.imbue(std::locale::classic());
            output << std::fixed << std::setprecision(2) << sensor.toCelsius(raw);
            temperatureText = output.str();
        } catch (const std::exception& e) {
            temperatureText = "ERRO";
            std::cerr << "Conversao: " << e.what() << '\n';
        }
    } catch (const std::exception& e) {
        std::cerr << "Aquisicao: " << e.what() << '\n';
    }
    Frame frame{};
    // Duas colunas equilibradas, com margens e separadores discretos.
    for (int y = 5; y <= 58; ++y) pixel(frame, 63, y);
    for (int x = 6; x <= 57; ++x) pixel(frame, x, 19);
    for (int x = 70; x <= 121; ++x) pixel(frame, x, 19);
    text(frame, 31, 5, "ADC", 2);
    text(frame, 95, 5, "TEMP", 2);
    valueText(frame, 31, rawText);
    valueText(frame, 95, temperatureText);
    text(frame, 31, 53, "BRUTO", 1);
    text(frame, 95, 51, "*C", 2);
    return frame;
}
    static void preview(const Frame& frame, const std::string& path) {
    std::ofstream out(path);
    out << "P1\n128 64\n";
    for (int y = 0; y < 64; ++y) {
        for (int x = 0; x < 128; ++x)
            out << ((frame[x+(y/8)*128] >> (y%8)) & 1) << ' ';
        out << '\n';
    }
    out.close();
    if (!out) throw std::runtime_error("Falha ao gravar preview.");
}
    class Oled {
    int fd_;
    void send(const std::uint8_t* bytes, std::size_t size) {
        // Uma transacao inclui o byte de controle. Nao repetir escritas parciais.
        if (::write(fd_, bytes, size) != static_cast<ssize_t>(size))
            throw std::runtime_error("Falha na escrita I2C do OLED.");
    }
public:
    explicit Oled(const std::string& bus) : fd_(::open(bus.c_str(), O_RDWR | O_CLOEXEC)) {
        if (fd_ < 0) throw std::runtime_error("Nao foi possivel abrir I2C: " + bus);
        try {
            if (::ioctl(fd_, I2C_SLAVE, 0x3c) < 0)
                throw std::runtime_error("Falha ao selecionar OLED 0x3c.");
            const std::uint8_t init[] = {0x00, 0xae, 0xd5, 0x80, 0xa8, 0x3f,
                0xd3, 0x00, 0x40, 0x8d, 0x14, 0x20, 0x00, 0xa1, 0xc8,
                0xda, 0x12, 0x81, 0x7f, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6};
            send(init, sizeof(init));
            show(Frame{});
            const std::uint8_t on[] = {0x00, 0xaf};
            send(on, sizeof(on));
        } catch (...) { ::close(fd_); throw; }
    }
    ~Oled() { ::close(fd_); }
    Oled(const Oled&) = delete;
    Oled& operator=(const Oled&) = delete;
    void show(const Frame& frame) {
        const std::uint8_t window[] = {0x00, 0x21, 0, 127, 0x22, 0, 7};
        send(window, sizeof(window));
        for (std::size_t offset = 0; offset < frame.size(); offset += 16) {
            std::array<std::uint8_t, 17> packet{};
            packet[0] = 0x40;
            std::copy_n(frame.data()+offset, 16, packet.data()+1);
            send(packet.data(), packet.size());
        }
    }
};
    static double number(const std::string& argument) {
    std::size_t used = 0;
    const double value = std::stod(argument, &used);
    if (used != argument.size() || !std::isfinite(value) || value <= 0)
        throw std::invalid_argument("Numero deve ser positivo e finito.");
    return value;
}
public:
    static int run(int argc, char** argv) {
    std::string path = "/sys/bus/iio/devices/iio:device0/in_voltage13_raw";
    std::string previewPath;
    std::string bus = "/dev/i2c-1";
    int maximum = 65535;
    ThermistorConfig config;
    try {
        for (int i = 1; i < argc; ++i) {
            const std::string option(argv[i]);
            if (option == "--help") {
                std::cout << "Uso: pc3_oled_arm [--i2c dispositivo] [--raw arquivo] [--adc-max inteiro] "
                    "[--beta K] [--r0 ohms] [--series ohms] [--ntc gnd|vcc] "
                    "[--preview arquivo.pbm]\n";
                return 0;
            }
            if (++i >= argc) throw std::invalid_argument("Falta valor para " + option);
            const std::string value(argv[i]);
            if (option == "--i2c") bus = value;
            else if (option == "--raw") path = value;
            else if (option == "--preview") previewPath = value;
            else if (option == "--adc-max") {
                std::size_t used = 0;
                maximum = std::stoi(value, &used);
                if (used != value.size() || maximum <= 0)
                    throw std::invalid_argument("Escala ADC invalida.");
            } else if (option == "--beta") config.beta = number(value);
            else if (option == "--r0") config.nominalResistance = number(value);
            else if (option == "--series") config.seriesResistance = number(value);
            else if (option == "--ntc") {
                if (value != "gnd" && value != "vcc")
                    throw std::invalid_argument("--ntc deve ser gnd ou vcc.");
                config.ntcToGround = value == "gnd";
            } else throw std::invalid_argument("Opcao desconhecida: " + option);
        }
        const TemperatureSensor validation(path, maximum, config);
    } catch (const std::exception& e) {
        std::cerr << "Configuracao: " << e.what() << '\n';
        return 2;
    }
    try {
        const TemperatureSensor sensor(path, maximum, config);
        if (!previewPath.empty()) {
            preview(sampleFrame(sensor), previewPath);
            return 0;
        }
        Oled oled(bus);
        auto next = std::chrono::steady_clock::now();
        while (true) {
            oled.show(sampleFrame(sensor));
            next += std::chrono::milliseconds(500);
            const auto now = std::chrono::steady_clock::now();
            if (next <= now) next = now + std::chrono::milliseconds(500);
            std::this_thread::sleep_until(next);
        }
    } catch (const std::exception& e) {
        std::cerr << "OLED: " << e.what() << '\n';
        return 1;
    }
}
};

int main(int argc, char** argv) { return OledApplication::run(argc, argv); }

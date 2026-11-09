#include "UdpSender.hpp"
#include <arpa/inet.h>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

UdpSender::UdpSender(const std::string& address, std::uint16_t port)
    : socket_(-1), destination_{} {
    if (!port || inet_pton(AF_INET, address.c_str(), &destination_.sin_addr) != 1)
        throw std::invalid_argument("Endereco IPv4 ou porta UDP invalidos.");
    destination_.sin_family = AF_INET;
    destination_.sin_port = htons(port);
    socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) throw std::runtime_error("Falha ao criar socket UDP.");
}
UdpSender::~UdpSender() { if (socket_ >= 0) ::close(socket_); }
void UdpSender::send(double celsius) const {
    if (!std::isfinite(celsius)) throw std::invalid_argument("Temperatura nao finita.");
    std::ostringstream packet;
    packet.imbue(std::locale::classic());
    packet << "KY-013," << std::fixed << std::setprecision(2) << celsius << ",C\n";
    const std::string message = packet.str();
    const ssize_t sent = ::sendto(socket_, message.data(), message.size(), 0,
        reinterpret_cast<const sockaddr*>(&destination_), sizeof(destination_));
    if (sent != static_cast<ssize_t>(message.size()))
        throw std::runtime_error("Falha no envio UDP.");
}

#ifndef UDP_SENDER_HPP
#define UDP_SENDER_HPP
#include <cstdint>
#include <string>
#include <netinet/in.h>

/** Envia uma leitura de temperatura em CSV por UDP. */
class UdpSender {
public:
    /** Cria o socket UDP para o IPv4 e porta indicados. */
    UdpSender(const std::string& address, std::uint16_t port);
    /** Fecha o socket. */
    ~UdpSender();
    /** Cria o socket UDP para o IPv4 e porta indicados. */
    /** Evita cópia do socket. */
    UdpSender(const UdpSender&) = delete;
    /** Evita atribuição do socket. */
    UdpSender& operator=(const UdpSender&) = delete;
    /** Envia uma temperatura em graus Celsius. */
    void send(double celsius) const;
private:
    int socket_; /**< Socket de envio. */
    sockaddr_in destination_; /**< Endereço e porta de destino. */
};
#endif

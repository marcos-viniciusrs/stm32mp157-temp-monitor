# Protocolo UDP — Entrega 3

A aplicação embarcada envia uma mensagem por amostra (4 Hz) a um endereço IPv4 e
porta configurados por `--udp IP PORTA`. Exemplo: `--udp 10.0.1.201 5005`.
O envio é por socket UDP, sem confirmação ou retransmissão.

Formato ASCII CSV de uma linha, com três campos:

```text
KY-013,22.00,C
```

A mensagem termina com uma quebra de linha (`\n`).
Campos: identificador `KY-013`; valor atual da temperatura em °C com duas casas
decimais e ponto decimal; unidade `C`. Timestamp não é enviado. O código ADC
bruto continua disponível no terminal com `--log`.

Na ligação RJ45 direta, a placa está em `10.0.1.200/24` e o PC em
`10.0.1.201/24`. A conexão do PC foi configurada no NetworkManager como
`Conexão cabeada 1`, sem rota padrão pela Ethernet. O ping PC → placa passou.
Para receber, execute `socat -u UDP4-RECV:5005,bind=10.0.1.201 STDOUT`
ou `python3 tools/udp_receiver.py --bind 10.0.1.201 --port 5005`.

## Validação na placa

Executada pela Ethernet direta em 09/11/2026. O PC recebeu oito datagramas
consecutivos da placa `10.0.1.200`; os valores corresponderam às saídas do
terminal e seguiram o intervalo de 250 ms da aplicação. Exemplo real:

```text
10.0.1.200:47495 KY-013,20.99,C
10.0.1.200:47495 KY-013,21.45,C
10.0.1.200:47495 KY-013,20.56,C
```

O executável ARM instalado tem SHA-256
`cad5f67fbb216846923e32d931cbfacf5275d09b75162292451f8a4319eca5c0`.
A versão anterior foi salva em
`/root/stm32-temp-monitor/temp_monitor_arm.before-udp`.
O serviço `S91tempudp` inicia o envio automaticamente quando a placa liga.
Verifique com `/etc/init.d/S91tempudp status`.

A captura da recepção real via `socat` está no
[README](../README.md#evidências-da-entrega-3).

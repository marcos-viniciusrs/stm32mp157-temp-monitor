# stm32mp157-temp-monitor
Sistema embarcado de monitoramento de temperatura utilizando STM32MP1 e sensor KY-013, desenvolvido em C++, com aquisição de dados em tempo real, transmissão via UDP e interface gráfica para visualização e registro das 

## 1. Conexão do sensor

Antes de ligar a placa, faça as conexões do sensor com o kit.

### Ligações
- **VCC** do sensor → **3.3V** da placa  
- **GND** do sensor → **GND** da placa  
- **Sinal (S)** do sensor → **entrada ADC** da placa  

> **Importante:** o sensor deve ser alimentado com **3.3V**, conforme orientação do projeto.

### Foto da conexão

![Conexão do sensor com o STM32MP1](docs/conexao_sensor.jpg)

Na montagem:
- **fio vermelho** = alimentação (**VCC**)
- **fio marrom/cinza** = **GND**
- **fio laranja** = **sinal do sensor**

---

## 2. Objetivo do projeto

Este projeto tem como objetivo monitorar a temperatura de uma carga sensível utilizando o sensor **KY-013** conectado ao kit **STM32MP1-DK1**.

O sistema:
- lê os dados do sensor;
- interpreta o valor medido;
- exibe a temperatura;
- e pode enviar os dados via rede para monitoramento remoto.

---

## 3. Estrutura do projeto

```bash
.
├── src/
│   └── main.cpp
├── include/
│   └── TemperatureSensor.hpp
├── docs/
│   └── conexao_sensor.jpg
└── README.md
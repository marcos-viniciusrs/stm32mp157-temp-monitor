# STM32MP157 Temperature Monitor

Sistema de monitoramento de temperatura em desenvolvimento, com sensor KY-013
e STM32MP1, utilizando C++ sobre Linux embarcado.

Atualmente, o programa adquire o valor bruto do ADC pelo subsistema IIO e exibe
uma amostra no terminal. As próximas etapas incluem conversão para temperatura,
transmissão UDP e visualização dos dados.

## Hardware

| KY-013 | STM32MP157C-DK2 utilizada no teste |
| --- | --- |
| VCC | 3,3 V |
| GND | GND |
| Sinal (S) | PC3 — ADC1, canal 13 |

Faça as conexões com a placa desligada. O canal ADC deve estar habilitado no Linux.

## Compilação

Dependência: compilador C++ com suporte a C++14, como o `g++`.

```bash
g++ -std=c++14 -Wall -Wextra -Iinclude src/main.cpp src/TemperatureSensor.cpp -o temp_monitor
```

## Uso

Informe o arquivo do canal IIO e o maior valor da escala do ADC:

```bash
./temp_monitor /sys/bus/iio/devices/iio:device0/in_voltage13_raw 65535
```

Ajuste o dispositivo, o canal e a escala conforme a configuração da placa.
Por exemplo, um ADC de 12 bits utiliza `4095`; um de 16 bits, `65535`.

```text
ADC: 2048
```

Cada execução faz uma leitura e termina. O valor apresentado é o código bruto
do ADC. Falhas de acesso ou dados inválidos são informados no terminal.

## Estrutura

```text
include/TemperatureSensor.hpp      Interface da classe de leitura
src/TemperatureSensor.cpp          Aquisição do ADC via IIO
src/main.cpp                       Aplicação de linha de comando
tests/TemperatureSensorTests.cpp   Testes básicos de aquisição
Doxyfile                           Configuração do Doxygen
```

A classe `TemperatureSensor` recebe o caminho do canal e a escala do ADC.
O método `readRaw()` retorna uma amostra inteira entre zero e o máximo configurado.

## Compilação para ARM

Defina o caminho do compilador da toolchain instalada:

```bash
/caminho/da/toolchain/bin/arm-buildroot-linux-gnueabihf-g++ \
  -std=c++14 -Wall -Wextra -Iinclude \
  src/main.cpp src/TemperatureSensor.cpp -o temp_monitor_arm
```

Transfira `temp_monitor_arm` para a placa e execute com o canal e a escala
correspondentes à sua configuração, usando `./temp_monitor_arm <arquivo_raw> <adc_max>`.

## Testes

Os testes usam um arquivo temporário para verificar a aquisição sem hardware:

```bash
g++ -std=c++14 -Wall -Wextra -Iinclude tests/TemperatureSensorTests.cpp src/TemperatureSensor.cpp -o temperature_sensor_tests
./temperature_sensor_tests
```

A aquisição física foi executada na STM32MP157C-DK2, usando PC3/ADC1_IN13
e resolução de 16 bits. O executável ARM leu oito amostras com código de saída `0`.
Consulte o [registro da leitura na placa](docs/teste_adc_pc3.log) e as
[instruções de configuração do PC3](docs/PC3.md).

O [registro de execução](docs/teste_adc.log) contém os comandos, as saídas e os
códigos de retorno de um teste local com entrada ADC simulada, incluindo a
compilação ARM. Os valores foram gravados em um arquivo temporário.

## Documentação

```bash
doxygen Doxyfile
```

A documentação da classe fica em `docs/doxygen/html/index.html`.

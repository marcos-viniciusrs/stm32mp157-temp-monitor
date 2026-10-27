# Verificação da aquisição

A implementação atual realiza a leitura bruta do ADC via Linux IIO.

## Testes locais

```bash
g++ -std=c++14 -Wall -Wextra -Iinclude tests/TemperatureSensorTests.cpp src/TemperatureSensor.cpp -o temperature_sensor_tests
./temperature_sensor_tests
```

Os testes usam um arquivo temporário e verificam três comportamentos:

- Leitura de valores na escala do ADC, incluindo zero e máximo.
- Atualização da amostra após alteração do arquivo.
- Tratamento de falha ao acessar o arquivo do canal.

Esses testes locais usam arquivos temporários. A aquisição no ADC físico também
foi executada e está registrada abaixo.

## Registro da execução

O arquivo [teste_adc.log](teste_adc.log) registra uma execução real no computador
com entrada simulada. Foram usados os códigos `2048`, `2035`, `2010`, `1987`, `0`
e `4095`, além de `4096` para verificar a rejeição de um valor acima da escala.
Esses números foram escritos em um arquivo temporário e lidos pelo executável;
não representam variações de temperatura medidas no sensor.

O registro também inclui a execução dos testes básicos e a compilação ARM.

## Compilação e documentação

A compilação direta com `g++` e com o compilador ARM da toolchain Buildroot
foi verificada, sem sistema de build adicional.
A documentação é gerada com `doxygen Doxyfile` e fica em
`docs/doxygen/html/index.html`.

## Aquisição na placa

O executável compilado com a toolchain Buildroot foi transferido por serial e
executado na STM32MP157C-DK2 com Linux 6.9.8. O sensor foi conectado ao PC3,
conforme informado pelo usuário. Após habilitar o ADC1_IN13 no Device Tree e
reiniciar a placa, o driver passou a expor `iio:device0/in_voltage13_raw`.

O formato `le:u16/16>>0` confirmou os 16 bits usados na aquisição. Foram
registradas oito leituras físicas do ADC, todas com código de saída zero.
O [registro da execução](teste_adc_pc3.log) contém comandos, valores e identificação
do binário. Não houve injeção de valores em arquivo temporário nessa aquisição.
Os resultados validam a leitura bruta; conversão e calibração de temperatura
continuam fora do escopo atual.

A [configuração aplicada e a recuperação](PC3.md) estão documentadas.

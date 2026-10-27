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

Esses testes não utilizam o ADC físico. A execução no kit continua pendente.

## Compilação e documentação

A compilação direta com `g++` e com o compilador ARM da toolchain Buildroot
foi verificada, sem sistema de build adicional.
A documentação é gerada com `doxygen Doxyfile` e fica em
`docs/doxygen/html/index.html`.

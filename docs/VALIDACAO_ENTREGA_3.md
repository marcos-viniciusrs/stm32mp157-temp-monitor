# Validação da Entrega 3 — temperatura e envio UDP

O enunciado oficial exige envio UDP da leitura para o servidor e documentação
do protocolo. A conversão para °C e o OLED são recursos adicionais.

## Verificações locais executadas em 09/11/2026

Compilação e testes C++:

```bash
g++ -std=c++14 -Wall -Wextra -Werror -Iinclude \
  tests/TemperatureSensorTests.cpp src/TemperatureSensor.cpp \
  -o /tmp/temperature_sensor_tests
/tmp/temperature_sensor_tests
```

Resultado:

```text
OK: aquisicao, modelo Beta, orientacoes, escalas e erros.
```

Os testes verificam 25 °C em meia escala, pontos acima e abaixo de 25 °C,
resolução de 16 bits, inversão do divisor, parâmetros personalizados,
parâmetros não finitos/não positivos, extremos do ADC, arquivos malformados
e falha de acesso. Os valores de referência para M = 4000, NTC para GND,
R₀ = Rf = 10 kΩ e Beta = 3950 K são:

| Código ADC simulado | Rntc | Temperatura esperada |
| --- | --- | --- |
| 1000 | 3333,333… Ω | 51,9594998258 °C |
| 2000 | 10000 Ω | 25 °C |
| 3000 | 30000 Ω | 2,1693194203 °C |

Esses pontos são resultados matemáticos, não medições do sensor.

Teste de integração (Python 3, sem dependências adicionais):

```bash
g++ -std=c++14 -Wall -Wextra -Werror -pthread -Iinclude \
  src/main.cpp src/TemperatureSensor.cpp src/UdpSender.cpp -o /tmp/temp_monitor_entrega3
python3 tests/test_cli.py /tmp/temp_monitor_entrega3
```

Resultado observado:

```text
OK: 5 amostras simuladas; intervalo primeira/ultima = 1.000 s
OK: CLI, modo silencioso, parametros, limites e codigos de saida.
```

O teste cria um arquivo temporário, verifica saída e cadência, opções do modelo,
modo sem log e erros. A tolerância temporal considera atrasos do escalonador;
não certifica tempo real nem desempenho no kit.

## Compilação ARM

Executada com o SDK OpenSTLinux instalado:

```bash
source /home/marcos/sdk-stm32mp157/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
$CXX $CPPFLAGS $CXXFLAGS -std=c++14 -Wall -Wextra -Werror -pthread -Iinclude \
  src/main.cpp src/TemperatureSensor.cpp src/UdpSender.cpp $LDFLAGS -o /tmp/temp_monitor_entrega3_arm
file /tmp/temp_monitor_entrega3_arm
```

Compilação concluída sem erros; `file` identificou ELF de 32 bits, ARM EABI5,
com interpretador `/lib/ld-linux-armhf.so.3`. O binário em `/tmp` é temporário.
A versão com UDP foi instalada e testada na placa; consulte a seção de rede abaixo.

## Comparação metrológica pendente

1. Confirmar a posição do NTC no divisor, seu valor nominal, Beta e resistor fixo.
2. Confirmar alimentação de 3,3 V, referência do ADC igual à alimentação do
   divisor e escala de 16 bits. Usar os parâmetros adequados à montagem.
3. Usar o executável já instalado conforme o README. Executar:

   ```bash
   ./temp_monitor_arm /sys/bus/iio/devices/iio:device0/in_voltage13_raw 65535 --log
   ```

4. Registrar ADC, temperatura e leitura de um termômetro de referência após
   estabilização. Repetir em diferentes temperaturas e confirmar o sentido
   da variação ao aquecer o sensor. Selecionar `--ntc vcc` se essa for a
   posição física do NTC; não trocar a alimentação como correção de software.
5. Registrar parâmetros, erros em relação à referência, cadência, comando,
   identificação do executável e evidências do terminal no repositório.

| Ponto | ADC | Temperatura calculada | Referência | Erro (calculada − referência) |
| --- | --- | --- | --- | --- |
| Ambiente estabilizado | Pendente | Pendente | Pendente | Pendente |
| Segundo ponto estabilizado | Pendente | Pendente | Pendente | Pendente |
| Terceiro ponto de verificação | Pendente | Pendente | Pendente | Pendente |

As evidências da Entrega 2 comprovam aquisição bruta e não validam a conversão
em °C. A nova implementação local do OLED mostra ADC e °C, com testes simulados
e compilação ARM aprovados; a instalação na placa foi concluída.
Consulte [OLED.md](OLED.md).


## Ajuste posterior de um ponto

Referência fornecida pelo usuário: ADC 55321 (escala 65535) corresponde a 22 °C.
O R₀ efetivo padrão foi ajustado para 47338,20493605491 Ω. Os pontos nominais
da tabela anterior continuam testados com R₀ explicitamente definido em 10 kΩ.
O teste C++ verifica o novo ponto com tolerância de 1e-9 °C e o sentido da curva;
a integração verifica `ADC: 55321 | Temperatura: 22.00 °C` e o teste do OLED
compara o quadro de temperatura com uma referência independente de 22 °C.
Isso valida o cálculo com a referência informada, sem constituir nova medição
física nem validação da curva inteira.


## Execução física posterior em 09/11/2026

Instalados `temp_monitor_arm` e `pc3_oled_arm`, com hashes conferidos e backup
registrado em [OLED.md](OLED.md). O terminal adquiriu oito amostras físicas em
aproximadamente dois segundos, de 55587 a 55954, convertidas em 21,31 a 20,35 °C.
O serviço OLED permaneceu ativo. Em teste separado com arquivo simulado na
placa, ADC 55321 produziu 22,00 °C. Veja o
[log completo](teste_entrega3_placa.log).
A execução confirma funcionamento no alvo, mas não substitui comparação com
termômetro de referência. A foto do OLED está no README.


## UDP na Ethernet direta

Com a placa em `10.0.1.200` e o PC em `10.0.1.201`, o monitor ARM enviou
leituras físicas para a porta UDP 5005. O PC recebeu oito mensagens CSV
consecutivas do identificador `KY-013`, com valores em °C correspondentes aos
do terminal. Consulte [UDP.md](UDP.md) para o protocolo, comando e exemplos.
O serviço `S91tempudp` inicia a transmissão no boot. A validação da entrega
inclui compilação ARM, teste no hardware e recepção no PC.

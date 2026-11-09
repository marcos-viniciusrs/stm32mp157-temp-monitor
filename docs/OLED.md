# OLED: ADC bruto e temperatura

`tools/pc3_oled.cpp` é uma nova implementação local para substituir o auxiliar
`pc3_oled_arm`. O fonte do executável antigo não estava disponível. A versão
nova reutiliza `TemperatureSensor::readRaw()` e `toCelsius()` e apresenta:

```text
  ADC    |   TEMP
---------+---------
 55321   |  22.00
 BRUTO   |    °C
```

O layout usa duas colunas, ADC à esquerda e temperatura à direita, com
títulos centralizados e divisórias finas. Os valores usam fonte ampliada em 3x;
valores negativos ou mais longos usam 2x para caber sem cortar.

Cada atualização usa uma única amostra para os dois valores. A tela SSD1306
128×64 é atualizada a 2 Hz, em `/dev/i2c-1`, endereço `0x3c` (barramento configurável com `--i2c`), conforme a montagem
registrada no README. A fonte inclui um glifo próprio para o símbolo de grau.
Os comandos seguem o [datasheet do SSD1306 da Solomon Systech](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf),
com endereçamento horizontal e envio de um quadro de 1024 bytes em blocos.

Em zero ou fundo de escala, mostra o ADC e `ERRO` para a temperatura. Se a leitura
falhar, mostra `ERRO` no ADC e `-- °C`. Tenta novamente na atualização seguinte;
os detalhes vão para stderr. Falhas I2C encerram com código 1; parâmetros
inválidos retornam 2. SIGINT/SIGTERM encerram o programa; na próxima inicialização a tela é limpa.

## Compilar

No computador, na raiz do repositório:

```bash
g++ -std=c++14 -Wall -Wextra -Werror -pthread -Iinclude \
  tools/pc3_oled.cpp src/TemperatureSensor.cpp -o /tmp/pc3_oled
python3 tests/test_oled.py /tmp/pc3_oled
```

Para ARM:

```bash
source /home/marcos/sdk-stm32mp157/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
$CXX $CPPFLAGS $CXXFLAGS -std=c++14 -Wall -Wextra -Werror -pthread -Iinclude \
  tools/pc3_oled.cpp src/TemperatureSensor.cpp $LDFLAGS -o /tmp/pc3_oled_arm
```

Sem argumentos, usa PC3/ADC1_IN13, escala 65535 e os parâmetros padrão do modelo.
Ajustes disponíveis:

```bash
./pc3_oled_arm --raw /sys/bus/iio/devices/iio:device0/in_voltage13_raw \
  --adc-max 65535 --ntc gnd --beta 3950 --r0 47338.20493605491 --series 10000
```

Use os mesmos parâmetros do `temp_monitor`. Consulte as hipóteses e a
referência de um ponto (55321 = 22 °C) e validação física pendente em [MODELO_KY013.md](MODELO_KY013.md).

## Prévia sem hardware

```bash
printf '55321\n' > /tmp/oled_adc
/tmp/pc3_oled --raw /tmp/oled_adc --preview /tmp/oled.pbm
```

`--preview` salva um único quadro PBM e sai sem abrir o I2C. Os pixels pretos
no PBM representam os pixels acesos do OLED. Um quadro de erro também é uma
prévia válida e retorna 0; os detalhes da falha aparecem em stderr.

## Procedimento de instalação e conferência na placa

Transfira `/tmp/pc3_oled_arm` do computador para `/tmp/pc3_oled_arm` na placa,
por SCP ou serial. Depois, no console da placa:

```bash
/etc/init.d/S90pc3oled stop
chmod +x /tmp/pc3_oled_arm
/tmp/pc3_oled_arm
```

Confira ADC e °C juntos, aqueça levemente o sensor e verifique a evolução da
temperatura. Encerre com Ctrl+C. Para instalar após a conferência:

```bash
cd /root/stm32-temp-monitor
cp -p pc3_oled_arm "pc3_oled_arm.backup-$(date +%s)"
cp /tmp/pc3_oled_arm pc3_oled_arm.new
chmod +x pc3_oled_arm.new
mv pc3_oled_arm.new pc3_oled_arm
cat /etc/init.d/S90pc3oled
```

Confira o comando do serviço: o novo executável aceita nenhum argumento ou
as opções nomeadas acima. Caso o serviço antigo use argumentos posicionais,
adapte-os para `-- --i2c "$oled_bus"`; se o sensor exigir parâmetros diferentes,
inclua-os no comando.
Em seguida:

```bash
/etc/init.d/S90pc3oled start
/etc/init.d/S90pc3oled status
```

Não execute o serviço e uma segunda instância do OLED ao mesmo tempo.

## Verificações realizadas

Em 09/11/2026, passaram a compilação local e ARM com `-Wall -Wextra -Werror`,
os testes de quadros simulados (ADC e temperatura, divisor invertido,
valor negativo, extremos e falhas de leitura) e a inspeção visual da prévia
55321 / 22,00 °C. O binário ARM foi gerado em `/tmp/pc3_oled_arm`.
A primeira verificação ocorreu sem placa. Posteriormente, a versão foi
instalada e o serviço foi testado no hardware, conforme registrado abaixo.


## Instalação realizada em 09/11/2026

Os executáveis calibrados do OLED e do terminal foram transferidos por serial,
com SHA-256 conferido antes e depois da instalação. O serviço foi adaptado para
`-- --i2c "$oled_bus"`, preservando a descoberta automática do controlador I2C5.
O processo permaneceu ativo após o teste de aquisição física no terminal.
A tela física foi fotografada; veja as evidências da Entrega 3 no README.

Backup dos dois executáveis e do serviço:
`/root/stm32-temp-monitor/backup-entrega3-946685285`.
O número do backup usa o relógio não sincronizado da placa.

SHA-256 dos executáveis instalados:

```text
1e344dbcabbab2b37137a4c5e7935041d706017d2924cfc83bcbad577379635a  pc3_oled_arm
e6bd3fd637dcf9dbf3896f5e69e6147558ac65550bf1433051c0d3628e1318b3  temp_monitor_arm
```

Evidências: [teste_entrega3_placa.log](teste_entrega3_placa.log).

## Layout de duas colunas instalado

Em 09/11/2026, a versão com ADC à esquerda e temperatura à direita foi
compilada, testada em simulação e instalada na placa. O serviço reiniciou e
permaneceu ativo. A prévia do quadro foi inspecionada no computador; a tela
física foi fotografada e está no README. A calibração 55321 = 22 °C permanece igual.

Backup anterior: `/root/stm32-temp-monitor/pc3_oled_arm.before-columns`.
SHA-256 da versão atual:

```text
396aea27e6c37ebba9b82b2e749f5f6ac91309333c3f4ce5547f2d9498363a41  pc3_oled_arm
```


## Revisão para orientação a objetos

A geração do quadro e o controle do OLED foram reunidos na classe
`OledApplication`, sem estado global. A nova versão foi compilada para ARM,
instalada e o serviço `S90pc3oled` reiniciou com sucesso. O serviço UDP
`S91tempudp` também permaneceu ativo.

SHA-256 do OLED instalado:
`dc014d062e9d9aa057819b8835ab840de577f689709522fd860044db3f5bc5af`.
A revisão anterior ficou em
`/root/stm32-temp-monitor/pc3_oled_arm.before-oo`.

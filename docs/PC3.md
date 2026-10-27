# PC3 — ADC1, canal 13

A placa utilizada na validação é uma STM32MP157C-DK2 com imagem Buildroot e Linux
6.9.8. O PC3 corresponde ao ADC1_IN13, conforme o
[manual UM2534 da ST](https://www.st.com/resource/en/user_manual/um2534-discovery-kits-with-stm32mp157-mpus-stmicroelectronics.pdf).

A imagem original habilitava os canais 18 e 19 do ADC1 para a alimentação USB-C.
Foi adicionado o canal 13, mantendo os existentes. O grupo de pinos do ADC recebeu
PC3 em modo analógico (`0x2311`). A alteração está em `pc3-device-tree.patch`.

## Configuração aplicada

- Original preservado: `/boot/stm32mp157c-dk2.dtb`.
- Novo Device Tree: `/boot/stm32mp157c-dk2-pc3.dtb`.
- Backup adicional: `/boot/stm32mp157c-dk2.dtb.pre-pc3`.
- Backup da inicialização: `/boot/extlinux/extlinux.conf.pre-pc3`.
- Entrada padrão: `stm32mp157c-dk2-pc3`.
- Entrada original mantida: `stm32mp157c-dk2-buildroot`.

O novo Device Tree foi transferido por serial e conferido por SHA-256 antes de
ativar a configuração. A placa reiniciou normalmente e expôs:

```text
/sys/bus/iio/devices/iio:device0/in_voltage13_raw
```

A resolução exposta em `scan_elements/in_voltage13_type` é `le:u16/16>>0`.

## Executar na placa

O executável foi instalado em `/root/stm32-temp-monitor/temp_monitor`:

```bash
/root/stm32-temp-monitor/temp_monitor /sys/bus/iio/devices/iio:device0/in_voltage13_raw 65535
```

O registro das oito amostras está em `teste_adc_pc3.log`. A data do registro é do
computador conectado à serial; o relógio da placa não estava sincronizado.

## Voltar à configuração anterior

Na placa, para restaurar a inicialização original:

```bash
cp /boot/extlinux/extlinux.conf.pre-pc3 /boot/extlinux/extlinux.conf
sync
reboot
```

Isso desabilita novamente o canal PC3. O Device Tree original também permanece
selecionável pelo menu de inicialização.

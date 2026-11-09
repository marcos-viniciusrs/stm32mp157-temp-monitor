# Modelo de conversão do KY-013

## Fonte e escolha

O [exemplo atual da Joy-IT para o KY-013](https://sensorkit.joy-it.net/en/sensors/ky-013)
usa o modelo Beta com R₀ = 10 000 Ω, T₀ = 298,15 K e B = 3950 K.
Adotamos o modelo Beta configurável, com o ajuste de R₀ descrito abaixo. B tem unidade kelvin;
o valor 3950 não é uma resistência, apesar do rótulo na tabela da página.
Parâmetros nominais não substituem a identificação e calibração do módulo usado.

## Divisor resistivo e ADC

Considere n como código ADC, M = 2ᴺ − 1 como maior código e Rf como resistor fixo.
A [derivação da Adafruit](https://learn.adafruit.com/thermistor/using-a-thermistor)
relaciona a tensão no divisor à resistência do NTC. Com VCC = VREF:

```text
x = Vout / VCC ≈ n / M

VCC — Rf — sinal — NTC — GND  (--ntc gnd)
Rntc = Rf × x / (1 − x) = Rf × n / (M − n)

VCC — NTC — sinal — Rf — GND  (--ntc vcc)
Rntc = Rf × (1 − x) / x = Rf × (M − n) / n
```

As duas expressões correspondem a posições diferentes do NTC. Escolha a que
representa a montagem real. Com NTC para GND, aquecer reduz n; com NTC para VCC,
aquecer aumenta n. Nas duas configurações corretas a temperatura calculada aumenta.

Se VREF diferir de VCC, x = (n/M) × (VREF/VCC); a implementação atual exige
VREF = VCC. Confirmar essa condição na placa antes de validar temperaturas.
M é configurável: 4095 para 12 bits e 65535 para os 16 bits deste projeto.
A aproximação usa o maior código como escala, sem correção de quantização do ADC.

## Resistência para temperatura

Modelo Beta empregado:

```text
R(T) = R₀ × exp[B × (1/T − 1/T₀)]
T(K) = 1 / [1/T₀ + ln(Rntc/R₀)/B]
T(°C) = T(K) − 273,15
```

ln é o logaritmo natural. A implementação usa double e separa os logaritmos
das resistências para evitar overflow de produtos intermediários.

Steinhart–Hart é uma alternativa de três coeficientes:
`1/T = A + Bsh × ln(R) + C × [ln(R)]³`, descrita no
[guia da Adafruit](https://learn.adafruit.com/thermistor/using-a-thermistor).
Ela exige coeficientes próprios do termistor; Bsh não é o Beta acima.
Não misturamos coeficientes de outros sensores com o modelo escolhido.

## Limites e calibração

A conversão rejeita n ≤ 0 e n ≥ M: os extremos tornam a resistência nula ou
infinita. Isso pode indicar saturação ou problema de conexão, mas o software
não distingue a causa. Parâmetros devem ser positivos e finitos; resultados
não finitos ou com temperatura absoluta não positiva também geram erro.

O modelo matemático não comprova a faixa útil nem a exatidão do conjunto físico.
Tolerâncias do NTC e do resistor, referência do ADC e autoaquecimento afetam
as medições. A orientação, R₀ e Beta precisam ser confirmados na montagem.
Compare pontos estabilizados com um termômetro de referência e registre os
erros; não declare calibração com base apenas em testes simulados.

Para ajustar Beta a duas resistências medidas R₁ e R₂ em temperaturas conhecidas
T₁ e T₂ (kelvin), a própria equação acima fornece:
`B = ln(R₁/R₂) / (1/T₁ − 1/T₂)`.
Depois, `R₀ = R₁ / exp[B × (1/T₁ − 1/298,15)]`.
Use os valores obtidos com `--beta` e `--r0`, e o resistor fixo medido em `--series`.


## Referência fornecida pelo usuário: 55321 = 22 °C

O padrão local foi ajustado para essa referência, na escala de 16 bits
(M = 65535), mantendo B = 3950 K, Rf = 10000 Ω e NTC para GND.
Foi aplicado o ajuste de R₀, sem somar um deslocamento constante em Celsius:

```text
Rref = 10000 × 55321 / (65535 − 55321)
R₀ efetivo = Rref / exp[3950 × (1/295,15 − 1/298,15)]
R₀ efetivo = 47338,20493605491 Ω
```

Esse R₀ é um parâmetro efetivo que faz a curva passar pelo ponto informado;
não comprova que o componente tenha essa resistência nominal. O ajuste é
compartilhado pelo terminal e pelo OLED. Para recuperar o modelo nominal de
10 kΩ, use `--r0 10000`. Alterar Beta, resistor fixo ou orientação exige rever
a calibração; o mesmo ponto não fica garantido com parâmetros diferentes.
A validação em outros pontos e a instalação na placa continuam pendentes.

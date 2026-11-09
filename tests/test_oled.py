#!/usr/bin/env python3
"""Verifica quadros OLED sem I2C; recebe o executável compilado."""
import pathlib
import subprocess
import sys
import tempfile

def column(pixels, right=False):
    return [pixels[y*128+x] for y in range(25, 47)
            for x in range(65 if right else 0, 128 if right else 62)]

binary = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory() as directory:
    raw = pathlib.Path(directory) / 'adc'
    image = pathlib.Path(directory) / 'frame.pbm'
    def render(value, *options):
        raw.write_text(str(value))
        result = subprocess.run([binary, '--raw', str(raw), '--adc-max', '4000',
                                 '--preview', str(image), *options],
                                capture_output=True, timeout=2)
        assert result.returncode == 0, result.stderr
        words = image.read_text().split()
        assert words[:3] == ['P1', '128', '64']
        pixels = list(map(int, words[3:]))
        assert len(pixels) == 8192 and set(pixels) == {0, 1}
        assert any(column(pixels))  # ADC visível
        assert any(column(pixels, True))  # temperatura/erro visível
        assert not any(pixels[61*128:])   # margem inferior preservada
        return pixels, result.stderr
    calibrated, err = render(55321, '--adc-max', '65535')
    assert not err
    # Compara os pixels com 22 °C produzidos por outro ponto parametrizado.
    reference, err = render(2000, '--r0', '8740.12445937103')
    assert column(calibrated, True) == column(reference, True)
    normal, err = render(2000)
    assert not err
    hot, err = render(1000)
    inverse, err = render(3000, '--ntc', 'vcc')
    assert column(hot, True) == column(inverse, True)
    assert column(hot) != column(inverse)
    negative, err = render(3000, '--beta', '3435', '--r0', '10000')
    assert column(negative, True) != column(normal, True) and not err
    low, err = render(0)
    assert b'Conversao' in err
    high, err = render(4000)
    assert b'Conversao' in err
    assert column(low, True) == column(high, True)
    assert column(low) != column(high)
    invalid, err = render('abc')
    assert b'Aquisicao' in err
    assert column(invalid) != column(low)
    for options in [['--beta', 'nan'], ['--adc-max', '0'], ['--ntc', 'bad']]:
        result = subprocess.run([binary, *options], capture_output=True, timeout=2)
        assert result.returncode == 2
print('OK: OLED simulado, ADC e Celsius, inversao, negativos e erros.')

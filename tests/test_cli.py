#!/usr/bin/env python3
"""Integração da CLI usando ADC simulado; recebe o caminho do executável."""
import os
import subprocess
import sys
import tempfile
import time
import select

binary = os.path.abspath(sys.argv[1])
with tempfile.NamedTemporaryFile(mode='w+') as adc:
    def write(value):
        adc.seek(0)
        adc.truncate()
        adc.write(value)
        adc.flush()

    write('55321\n')
    process = subprocess.Popen([binary, adc.name, '65535', '--log'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stamps = []
    try:
        for _ in range(5):
            assert select.select([process.stdout], [], [], 2)[0], 'Sem amostra em 2 s'
            line = process.stdout.readline().decode()
            stamps.append(time.monotonic())
            assert line.strip() == 'ADC: 55321 | Temperatura: 22.00 °C', line
    finally:
        process.terminate()
        process.communicate(timeout=2)
    elapsed = stamps[-1] - stamps[0]
    assert 0.8 <= elapsed <= 1.5, f'Cadencia inesperada: {elapsed}'
    print(f'OK: 5 amostras simuladas; intervalo primeira/ultima = {elapsed:.3f} s')

    process = subprocess.Popen([binary, adc.name, '65535'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        time.sleep(0.6)
        assert process.poll() is None, 'Modo silencioso encerrou'
    finally:
        process.terminate()
        stdout, stderr = process.communicate(timeout=2)
    assert not stdout and not stderr, (stdout, stderr)

    for options in [['--unknown'], ['--beta'], ['--beta', 'nan'],
                    ['--beta', 'inf'], ['--r0', '0'], ['--series', '-1'],
                    ['--ntc', 'invalid'], ['--beta', '3950x']]:
        result = subprocess.run([binary, adc.name, '65535'] + options,
                                capture_output=True, timeout=2)
        assert result.returncode == 2 and result.stderr, options
    for scale in ['0', '-1', '65535x', 'abc']:
        result = subprocess.run([binary, adc.name, scale], capture_output=True, timeout=2)
        assert result.returncode == 2, scale
    for value in ['0', '65535', '-1', '65536', 'abc', '']:
        write(value)
        result = subprocess.run([binary, adc.name, '65535'], capture_output=True, timeout=2)
        assert result.returncode == 1 and result.stderr, value
    result = subprocess.run([binary, adc.name + '/missing', '65535'],
                            capture_output=True, timeout=2)
    assert result.returncode == 1 and result.stderr
    # Verifica passagem dos parâmetros pela CLI com divisor invertido.
    write('1000')
    process = subprocess.Popen([binary, adc.name, '4000', '--ntc', 'vcc',
                                '--r0', '30000', '--series', '10000',
                                '--beta', '3435', '--log'],
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        assert select.select([process.stdout], [], [], 2)[0]
        assert process.stdout.readline().decode().strip() == 'ADC: 1000 | Temperatura: 25.00 °C'
    finally:
        process.terminate()
        process.communicate(timeout=2)
print('OK: CLI, modo silencioso, parametros, limites e codigos de saida.')

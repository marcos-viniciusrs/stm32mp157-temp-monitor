#!/usr/bin/env python3
"""Receptor de validação UDP da Entrega 3 (sem interface gráfica)."""
import argparse
import csv
import io
import socket

parser = argparse.ArgumentParser()
parser.add_argument('--bind', default='10.0.1.201')
parser.add_argument('--port', type=int, default=5005)
args = parser.parse_args()
with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as server:
    server.bind((args.bind, args.port))
    print(f'Escutando {args.bind}:{args.port}', flush=True)
    while True:
        data, peer = server.recvfrom(512)
        try:
            row, = list(csv.reader(io.StringIO(data.decode('ascii'))))
            if len(row) != 3 or row[0] != 'KY-013' or row[2] != 'C':
                raise ValueError('campos')
            float(row[1])
            print(f'{peer[0]}:{peer[1]} {row[0]} {row[1]} °C', flush=True)
        except (UnicodeDecodeError, ValueError) as error:
            print(f'Pacote invalido de {peer}: {error}', flush=True)

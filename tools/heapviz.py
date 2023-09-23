#!/usr/bin/env python3

import argparse, csv, pathlib, sys

parser = argparse.ArgumentParser(prog='Heap Visualizer')
parser.add_argument('filename', type=pathlib.Path)
parser.add_argument('-o', '--output', type=pathlib.Path, default=None)
args = parser.parse_args()

output = open(args.output, 'w') if args.output else sys.stdout

WIDTH = 2048
HEIGHT = 64


with open(args.filename) as csvfile:
    output.write(f'<svg viewBox="0 0 {WIDTH} {HEIGHT+80}" xmlns="http://www.w3.org/2000/svg">\n')

    used = 0
    level = 0
    occupied = 0
    cache = 0
    free = 0

    reader = csv.reader(csvfile, skipinitialspace=True)
    blocks = [[int(col, 0) for col in block] for block in reader]
    start = blocks[0][0]
    end = blocks[-1][0] + blocks[-1][1]
    total = end - start
    color_variance = 0

    for pos, size, user, tag, frame in blocks:
        x = ((pos - start) / total) * WIDTH
        width = (size / total) * WIDTH
        if tag & 1:
            used += size
            color = 0xff0000
        elif tag & 2:
            used += size
            level += size
            color = 0x0000ff
        elif tag & 4:
            used += size
            level += size
            color = 0x00ffff
        elif tag & 8:
            cache += size
            color = 0x00ff00
        elif tag & 16:
            cache += size
            color = 0xff00ff
        else:
            free += size
            color = 0xffffff

        if tag:
            occupied += size
            fac = (64 - color_variance) / 64
            color = (int((color & 0xff0000) * fac) & 0xff0000) \
                    | (int((color & 0xff00) * fac) & 0xff00) \
                    | (int((color & 0xff) * fac) & 0xff)
            color_variance = (color_variance + 1) & 7

        output.write(f'  <rect x="{x}" y="0" width="{width}" height="{HEIGHT}" stroke="transparent" fill="#{color:06x}"/>\n')

    stats = [('Total', total), ('Used' , used), ('Occupied', occupied),
             ('Level', level), ('Cache', cache), ('Free', free)]
    for i, (name, val) in enumerate(stats):
        output.write(f'<text x="8" y="{HEIGHT + 76 - i*12}">{name} {val}</text>\n')
    output.write('</svg>\n')


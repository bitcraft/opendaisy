# -*- coding: utf-8 -*-
"""
Generate keymap.h
"""
from collections import defaultdict
from functools import partial


def rotate(first, string):
    n = string.index(first)
    return string[n:] + string[:n]


# homes on 'w', but leaving LEOM it is moved to '
# engage ccw   ' '
# engage down w '

# this is the data on the wheel, reading the glyphs clockwise
km = "-*+¢^Ç!$[@¿#]é`½~¼¶§KFZVEYQNPGHDBJROMIWAUSCXTLrfg'wduaoienthslcybp.m,kv_\"01x528934zj67q:)(&?;%/="
#                                                      ^^
# km = km[::-1]

# rotate the keymap to the zeroth index when the wheel is homed
# just a convience function, while debugging
km = rotate("g", km)


index_list = list()
symbol_list = list()


with open('keymap.h', 'w') as fp:
    # print = partial(print, file=fp)

    # col_width = max((len(i) for i in nnn.values()))
    for index, key in enumerate(km):
        as_value = ord(key)
        if as_value > 255:
            print(as_value)
        # print('#define {:<8} 0x{:<3X} /* {} */'.format(fmt, counter, key))
        index_list.append(as_value)

    # print("\n")
    print("const uint8_t ascii[] = {", end="")
    print(*index_list, sep=', ', end="")
    print("};")


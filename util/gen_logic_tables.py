"""
State machine development and debugging
"""
# unsigned long gives 32 bits

bools = [
    'INIT',
    'ESCAPE',
    'HOMING1',
    'HOMING2',
    'HOMING3',
    'HOMING4',
    'NORMAL',
    'LEOM',
    'DASY_MOVING',
    'CARR_MOVING',
    'HAMMER_RDY',
    'MARGIN_LEFT',
    'MARGIN_RIGHT',
    'CHAR_WAITING',
]

dfn = '#define'
name_col_width = max((len(i) for i in bools))

token = 'RESET'
value = 0
fmt = '{dfn} {:<{w}}  0b{v:0>32b}  // {v}'
print(fmt.format(token, dfn=dfn, v=value, w=name_col_width))

for index, token in enumerate(bools):
    value = pow(2, index)
    fmt = '{dfn} {:<{w}}  0b{v:0>32b}  // {v}'
    print(fmt.format(token, dfn=dfn, v=value, w=name_col_width))

print()

for index, token in enumerate(bools):
    fmt = '{dfn} {:<{w}}  {v}'
    print(fmt.format(token, dfn=dfn, v=index, w=name_col_width))

print()

def loop():
    stack = list()
    code = int(input("code? "))
    for index, token in reversed(list(enumerate(bools))):
        value = pow(2, index)
        if value <= code:
            code -= value
            print(token)
            stack.append(token)
    print()

    if stack:
        stack.reverse()
        print("case {}:".format(" + ".join(stack)))
        print()

while(1):
    loop()

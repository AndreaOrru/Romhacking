import re

# Instructions whose size depends on the value of P.m:
opA = ['ADC', 'AND', 'ASL', 'BIT', 'CMP', 'DEC', 'EOR', 'INC', 'LDA', 'LSR', 'ORA',
       'PHA', 'PLA', 'ROL', 'ROR', 'SBC', 'STA', 'STZ', 'TRB', 'TSB']

# Instructions whose size depends on the value of P.x:
opX = ['CPX', 'CPY', 'DEX', 'DEY', 'INX', 'INY', 'LDX', 'LDY', 'PHX', 'PHY', 'PLX',
       'PLY', 'STX', 'STY', 'TSX']

# Convert an ASM instruction to C++:
def parse(ea, sz, mnem, m, x, params):
    # Special cases:
    if mnem == 'PHK':
        return 'PHK(0x%X);' % (ea >> 16)
    elif mnem == 'JSR' and re.match(r"\w+", params):
        return '%s();' % params
    
    # Substitute various symbols:
    ds = False  # ds = do the parameters include D or S?
    if re.match(r"(D|S),(.*?)", params):
        params = re.sub(r"(D|S),(.*?)", r"\1 + \2", params)     
        ds = True
    params = re.sub(r"(.*?),(X|Y)", r"\1 + \2%s" % ('.l' if x else '.w'), params)
    params = re.sub(r"\((.*?)\)", r"rd_w(\1)", params)
    params = re.sub(r"\[(.*?)\]", r"rd_l(\1)", params)

    if (mnem in opA) or (mnem in opX):
        if len(params) > 0:
            # If the parameter is a constant:
            if params[0] == '#':
                mnem += '_imm'
            # If the address is not fully specified:
            elif sz < 4 and not ds:
                params = re.sub(r"\$(\w+)", r"B + $\1", params)
        # Specify the size of the instruction target:
        mnem += '_b' if (m if mnem in opA else x) else '_w'
    
    # Substitute constant and hex symbols:
    params = params.replace('$', '0x')
    params = params.replace('#', '')
    return '%s(%s);' % (mnem, params)

# Build a dictionary of function names -> list of instructions:
functions = dict()
for f in Functions():
    fname = GetFunctionName(f)
    functions[fname] = list()
    for ea in Heads(f, FindFuncEnd(f)):
        functions[fname].append((ea, DecodeInstruction(ea).size,
                                 GetReg(ea, 'm') == 1, GetReg(ea, 'x') == 1,
                                 GetDisasm(ea)))

# Write the C++ source code of the ROM:
out = open('source.cpp', 'w')
for f in functions:
    out.write('void %s()\n{\n' % f)
    for (ea, sz, m, x, disasm) in functions[f]:
        # Write the label if there is one:
        if (Name(ea)):
            out.write('%s:\n' % Name(ea))
        # Split the line in its components:
        mnem, params, comment = re.match(r"(\w+)\s+(.*?)\s*(;.*)", disasm).groups()[:3]
        
        # Get the C++ equivalent:
        if mnem == 'NOP':
            continue
        elif mnem in ('RTS', 'RTL'):
            opcode = 'return;'
        else:
            opcode = parse(ea, sz, mnem, m, x, params)
        
        comment = comment[comment.rfind(';')+1:]
        out.write('    {0: <40}//{1}\n'.format(opcode, comment))
    out.write('}\n\n')
out.close()
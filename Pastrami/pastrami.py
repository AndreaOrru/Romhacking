import re

opA = ['ADC', 'AND', 'ASL', 'BIT', 'CMP', 'DEC', 'EOR', 'INC', 'LDA', 'LSR', 'ORA',
       'PHA', 'PLA', 'ROL', 'ROR', 'SBC', 'STA', 'STZ', 'TRB', 'TSB']

opX = ['CPX', 'CPY', 'DEX', 'DEY', 'INX', 'INY', 'LDX', 'LDY', 'PHX', 'PHY', 'PLX',
       'PLY', 'STX', 'STY', 'TSX']

def parse(ea, sz, mnem, m, x, params):
	if mnem == 'JSR' and re.match(r"\w+", params):
		return '%s();' % params
	elif mnem == 'PHK':
		return 'PHK(0x%X);' % (ea >> 16)
		
	ds = False
	if re.match(r"(D|S),(.*?)", params):
		params = re.sub(r"(D|S),(.*?)", r"\1 + \2", params)		
		ds = True
	params = re.sub(r"(.*?),(X|Y)", r"\1 + \2%s" % ('.l' if x else '.w'), params)
	params = re.sub(r"\((.*?)\)", r"ind(\1)", params)
	params = re.sub(r"\[(.*?)\]", r"ind_long(\1)", params)

	if (mnem in opA) or (mnem in opX):
		if len(params) > 0:
			if params[0] == '#':
				mnem += '_imm'
			elif sz < 4 and not ds:
				params = re.sub(r"\$(\w+)", r"B + $\1", params)
		mnem += '_b' if (m if mnem in opA else x) else '_w'
		
	params = params.replace('$', '0x')
	params = params.replace('#', '')
	return '%s(%s);' % (mnem, params)

functions = dict()

for f in Functions():
	fname = GetFunctionName(f)
	functions[fname] = list()
	for ea in Heads(f, FindFuncEnd(f)):
		functions[fname].append((ea, DecodeInstruction(ea).size, GetReg(ea, 'm') == 1, GetReg(ea, 'x') == 1, GetDisasm(ea)))

for f in functions:
	print 'void %s()\n{' % f
	for (ea, sz, m, x, disasm) in functions[f]:
		if (Name(ea)):
			print '%s:' % Name(ea)
		mnem, params, comment = re.match(r"(\w+)\s+(.*?)\s*(;.*)", disasm).groups()[:3]
		opcode  = parse(ea, sz, mnem, m, x, params)
		comment = comment[comment.rfind(';')+1:]
		print '    {0: <30}//{1}'.format(opcode, comment)
	print '}\n'
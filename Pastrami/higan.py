#!/usr/bin/env python

from sys import argv
import re

mod = ['inc', 'dec', 'asl', 'lsr', 'rol', 'ror', 'trb', 'tsb']
ignore = []

f = open(argv[1], 'r').read()

f = re.sub(r"0x([0-9a-f]*)", lambda m: '0x' + m.group(1).upper(), f)
f = re.sub(r"if\(", 'if (', f)
f = re.sub(r"regs.(\w)", lambda m: m.group(1).upper(), f)
f = re.sub(r"rd\.(l|w)", 'v', f)
f = re.sub(r"L ", '  ', f)
f = re.sub(r"\s+op_io(.*?)\(\);", '', f)
f = re.sub(r"op_readstackn?\(\)", 'rd_b(++S.w)', f)
f = re.sub(r"op_writestackn?\((.*)\)", r"wr_b(S.w--, \1)", f)

funs = [x.strip() for x in re.split('auto R65816::op_', f) if x != '']
for i in range(len(funs)):
    m = re.match('([^_( ]+)(.|\n)*', funs[i]).group(1)
    if (len(m) > 3 or m in ignore):
        funs[i] = ''
    else:
        try:
            mnem, sz, body = re.match(r"(\w+?)(_(\w))?\(\) \{\n((.|\n)*)\}", funs[i]).group(1, 3, 4)
        except:
            continue

        if sz == 'e':
            funs[i] = ''
            continue
        elif sz == 'n':
            sz = ''

        if sz:
            sz_n = (8 if sz == 'b' else 16)
            if re.search(r"( v [^=]{0,2}= )|(v\+\+)|(v\-\-)|(\+\+v)|(\-\-v)", body):
                funs[i]  = 'inline void %s_%s(uint%d_t& v)\n{\n%s}\n' % (mnem.upper(), sz, sz_n, body)
                funs[i] += 'inline void {0}_{1}(uint32_t i) {{ {0}_{1}(mem_{1}(i)); }}'.format(mnem.upper(), sz)
            else:
                funs[i]  = 'inline void %s_imm_%s(uint%d_t v)\n{\n%s}\n' % (mnem.upper(), sz, sz_n, body)
                funs[i] += 'inline void {0}_{1}(uint32_t i) {{ {0}_imm_{1}(mem_{1}(i)); }}'.format(mnem.upper(), sz)
        else:
            funs[i] = 'inline void %s()\n{\n%s}' % (mnem.upper(), body)
funs = [x for x in funs if x != '']

print('\n\n'.join(funs))

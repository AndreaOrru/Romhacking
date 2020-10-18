#!/usr/bin/env python3

import sys


def substitute_accents(s):
    s = s.replace("È", "[94]")
    s = s.replace("À", "[95]")
    s = s.replace("à", "[128]")
    s = s.replace("à", "[128]")
    s = s.replace("é", "[129]")
    s = s.replace("è", "[130]")
    s = s.replace("ì", "[131]")
    s = s.replace("ò", "[132]")
    s = s.replace("ù", "[133]")
    return s


def substitute_spaces(s):
    while True:
        pos = s.find("     ")
        if pos == -1:
            return s
        n = 5

        while s[pos + n] == " ":
            n += 1

        b = "[11][{}]".format(n)
        s = s[:pos] + b + s[pos + n :]
    return s


script_file = sys.argv[1]
dict_file = sys.argv[2]
new_script_file = sys.argv[3]

script = open(script_file).read().split("\n")
mtes = [x for x in open(dict_file).read().split("\n") if x]
mtes = sorted(enumerate(mtes), key=lambda x: len(x[1]), reverse=True)

new_script = []

for line in script:
    prefix, dialogue = line[:7], line[7:]
    for n, mte in mtes:
        bin_mte = "[14][{}]".format(n)
        dialogue = substitute_accents(dialogue)
        dialogue = substitute_spaces(dialogue)
        dialogue = dialogue.replace(mte, bin_mte)
    new_script.append(prefix + dialogue)


with open(new_script_file, "w") as f:
    for line in new_script:
        f.write(line + "\n")

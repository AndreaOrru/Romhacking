from typing import List

from bidict import bidict

from romhacking.formatting import ascii_dictionary

dictionary = bidict({
    **ascii_dictionary,
    **{
        chr(0x11): 'à',
        chr(0x13): 'è',
        chr(0x15): 'é',
        chr(0x17): 'ì',
        chr(0x19): 'ò',
        chr(0x1B): 'ù',
        chr(0x1D): 'È',
        '_': '…',
        '<': '«',
        '>': '»',
    }
})


def format(text: str) -> str:
    output = ''
    i = 0

    while i < len(text):
        c = text[i]

        try:
            output += dictionary[c]
        except KeyError:
            if ord(c) == 0x00:
                output += '<End>'
            elif ord(c) == 0x80:
                output += '<S'
                j = 0
                while True:
                    j += 1
                    c = text[i + j]
                    output += ' ${:02X}'.format(ord(c))
                    if j > 1 and ord(c) in (0x80, 0x81, 0x82):
                        break
                i += j
                output += '>'
            elif ord(c) == 0x81:
                output += '<Boy>'
            elif ord(c) == 0x82:
                output += '<Dog>'
            elif ord(c) == 0x83:
                output += '<Player3>'
            elif ord(c) == 0x84:
                output += '<Player4>'
            elif ord(c) == 0x8B:
                output += '<Choice>'
            elif ord(c) == 0xA1:
                output += '<Count>'
            elif ord(c) == 0xA2:
                output += '<Item>'
            else:
                output += '<${:02X}>'.format(ord(c))

        i += 1

    return output


def unformat(text: str) -> List[int]:
    output = []
    i = 0
    while i < len(text):
        c = text[i]
        if c != '<':
            output.append(ord(dictionary.inverse[c]))
            i += 1
            continue

        j = text.find('>', i + 1)
        tag = text[i + 1:j]

        if tag.startswith('S '):
            output.append(0x80)
            for byte in tag[2:].split():
                output.append(int(byte[1:], 16))
        elif tag == 'End':
            output.append(0x00)
        elif tag == 'Boy':
            output.append(0x81)
        elif tag == 'Dog':
            output.append(0x82)
        elif tag == 'Player3':
            output.append(0x83)
        elif tag == 'Player4':
            output.append(0x84)
        elif tag == 'Choice':
            output.append(0x8B)
        elif tag == 'Count':
            output.append(0xA1)
        elif tag == 'Item':
            output.append(0xA2)
        else:
            output.append(int(tag[1:], 16))
        i = j + 1

    return output

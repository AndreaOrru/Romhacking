from typing import List

from bidict import bidict

from starocean.rom_version import ROMVersion

dictionary_dejap = bidict(
    {
        **{x: chr(x + 0x2A) for x in range(0x06, 0x10)},  # 0-9
        **{x: chr(x + 0x31) for x in range(0x10, 0x2A)},  # A-Z
        **{x: chr(x + 0x37) for x in range(0x2A, 0x44)},  # a-z
        **{
            0x00: "\n",
            0x44: " ",
            0x45: "~",
            0x46: "!",
            0x47: "?",
            0x48: "#",
            0x49: "%",
            0x4A: "&",
            0x4B: "(",
            0x4C: ")",
            0x4D: "<",
            0x4E: ">",
            0x4F: "\\",
            0x50: "/",
            0x51: "*",
            0x52: "+",
            0x53: "-",
            0x55: ",",
            0x56: ".",
            0x57: ":",
            0x59: "”",
            0x5A: "'",
            0x5B: "“",
        },
    }
)

dictionary_magno = bidict(
    {
        **{x: chr(x + 0x2A) for x in range(0x06, 0x10)},  # 0-9
        **{x: chr(x + 0x31) for x in range(0x10, 0x2A)},  # A-Z
        **{x: chr(x + 0x30) for x in range(0x31, 0x4B)},  # a-z
        **{
            0x00: "\n",
            0x52: " ",
            0x53: "'",
            0x54: "!",
            0x55: "~",
            0x56: "?",
            0x57: ",",
            0x58: ".",
            0x59: ":",
            0x5A: ";",
            0x5B: "“",
            0x5C: "”",
            0x5D: "/",
            0x5E: "*",
            0x5F: "+",
            0x60: "-",
            0x61: "=",
            0x62: "#",
            0x63: "%",
            0x64: "&",
            0x65: "(",
            0x66: ")",
            0x67: "<",
            0x68: ">",
        },
    }
)

names = bidict(
    {
        0x00: "RATIX",
        0x01: "MILLY",
        0x02: "DORN",
        0x03: "RONIXIS",
        0x04: "IRIA",
        0x05: "CIUS",
        0x06: "JOSHUA",
        0x07: "TINEK",
        0x08: "MARVEL",
        0x09: "ASHLAY",
        0x0A: "PERISIE",
        0x0B: "FEAR",
    }
)


def dictionary(rom_version):
    if rom_version == ROMVersion.DEJAP:
        return dictionary_dejap
    else:
        return dictionary_magno


def format(data: List[int], rom_version: ROMVersion) -> str:
    d = dictionary(rom_version)
    text = ""
    i = 0
    while i < len(data):
        x = data[i]
        y = data[i + 1] if i + 1 < len(data) else None
        z = data[i + 2] if i + 2 < len(data) else None
        w = data[i + 3] if i + 3 < len(data) else None

        try:
            text += d[x]
        except KeyError:
            if x == 0x01:
                text += "<WAIT>"
                if y != 0xFF:
                    text += "\n"
            elif x == 0x02 and (y is not None) and y <= 11:
                text += f"<{names[y]}>"
                i += 1
            elif x == 0x03 and (y is not None):
                text += "<PAUSE><{:02X}>".format(y)
                i += 1
            elif x == 0x04 and (y is not None) and y <= 7:
                text += "<COLOR><{:02X}>".format(y)
                i += 1
            elif (x, y) == (0x89, 0x80):
                text += "<PAD><{:02X}><{:02X}>".format(z, w)
                i += 3
            elif (x, y) == (0xFF, 0xFF):
                text += "<CLOSE>"
                i += 1
            else:
                text += "<{:02X}>".format(x)
        i += 1
    return text


def unformat(text: str, rom_version: ROMVersion) -> List[int]:
    d = dictionary(rom_version)
    output = []
    i = 0
    while i < len(text):
        c = text[i]
        if c != "<":
            output.append(d.inverse[c])
            i += 1
            continue

        j = text.find(">", i + 1)
        tag = text[i + 1 : j]

        if tag in names.inverse:
            output.append(0x02)
            output.append(names.inverse[tag])
        elif tag == "WAIT":
            output.append(0x01)
            if text[j + 1] == "\n":
                j += 1
        elif tag == "PAUSE":
            output.append(0x03)
        elif tag == "COLOR":
            output.append(0x04)
        elif tag == "PAD":
            output += [0x89, 0x80]
        elif tag == "CLOSE":
            output += [0xFF, 0xFF]
        else:
            output.append(int(tag, 16))
        i = j + 1
    return output

from cached_property import cached_property

from evermore.formatting import format


class Sentence:
    def __init__(self, rom, start: int, is_ascii: bool):
        self.rom = rom
        self.start = start
        self.is_ascii = is_ascii

    @cached_property
    def data(self) -> str:
        if self.is_ascii:
            return self._decodeAscii()
        return self._decode()

    @cached_property
    def text(self) -> str:
        return format(self.data)

    def _decodeAscii(self) -> str:
        output = ""
        i = self.start

        while True:
            c = self.rom.readByte(i)
            output += chr(c)
            i += 1

            if c == 0x00:
                break

        return output

    def _decode(self) -> str:
        output = ""
        i = self.start

        while True:
            c = self.rom.readByte(i)
            i += 1

            if c < 0x40:
                output += self.rom.scatter_table[c]

            elif 0x40 <= c < 0x80:
                size = c & 0x3F
                for _ in range(size):
                    output += chr(self.rom.readByte(i))
                    i += 1

            elif 0x80 <= c < 0xC0:
                output += self._expandDTE(c)

            elif c == 0xC0:
                c = self.rom.readByte(i)
                output += self._expandMTE_16(c)
                i += 1

            elif c > 0xC0:
                output += self._expandMTE_8(c)

            if output[-1] == "\x00":
                break

        return output

    def _expandDTE(self, c: int) -> str:
        return self.rom.dtes[c - 0x80]

    def _expandMTE_8(self, c: int) -> str:
        return self.rom.mtes[c - 0xC1]

    def _expandMTE_16(self, c: int) -> str:
        return self.rom.mtes[c + 63]

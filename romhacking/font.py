from array import array
from typing import List

from functools import cached_property


class Font:
    CHAR_SIZE = 0x20

    def __init__(self, data: array):
        self.data = data

    @cached_property
    def characters(self) -> List[List[int]]:
        assert len(self.data) % self.CHAR_SIZE == 0

        characters = []
        for i in range(0, len(self.data), self.CHAR_SIZE):
            char_data = self.data[i : i + self.CHAR_SIZE]
            characters.append(self._convertTo16x16(char_data))
        return characters

    @cached_property
    def widths(self) -> List[int]:
        widths = []
        for char in self.characters:
            max_width = 0
            for line in char:
                string = "{:016b}".format(line)
                width = string.rfind("1")
                if width > max_width:
                    max_width = width
            widths.append(max_width + 1 if max_width else 3)
        return widths

    @staticmethod
    def displayChar(char: List[int]) -> str:
        strings = []
        for b in char:
            string = "{:016b}".format(b).replace("0", " ").replace("1", "#")
            strings.append(string)
        return "\n".join(strings)

    def generateVWF(self, string: List[int]) -> List[List[int]]:
        shift = 0
        tiles = []

        for c in string:
            if shift % 16 == 0:
                tiles.append([0] * 16)
                shift = 0

            char = self.characters[c]
            shifted_char = self._shiftChar(char, shift)
            for i, line in enumerate(shifted_char):
                tiles[-1][i] |= line

            width = self.widths[c]
            shift += width
            if shift > 16:
                shift = shift % 16
                rest_char = self._shiftChar(char, shift - width)
                tiles.append(rest_char)

        return tiles

    @staticmethod
    def _convertTo16x16(char_data: array) -> List[int]:
        data: List[int] = []
        for i in range(Font.CHAR_SIZE // 2):
            tile1 = char_data[i]
            tile2 = char_data[i + (Font.CHAR_SIZE // 2)]
            line = (tile1 << 8) | tile2
            data.append(line)
        return data

    @staticmethod
    def _shiftChar(char: List[int], shift: int) -> List[int]:
        shifted = char[:]

        for i, line in enumerate(shifted):
            if shift > 0:
                shifted[i] = line >> shift
            elif shift < 0:
                shifted[i] = line << -shift
            shifted[i] &= 0xFFFF

        return shifted

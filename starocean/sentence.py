from typing import List

from cached_property import cached_property

from romhacking.utils import invalidate_property
from starocean.formatting import format, unformat
from starocean.layout import TYPE_INDEX1, TYPE_INDEX2, TYPE_INDEX3, TYPE_INDEX4


class Sentence:
    def __init__(self, rom, data: List[int]):
        self._rom = rom
        self._original_compressed_data = data
        self._new_data: List[int] = []

    @cached_property
    def data(self) -> List[int]:
        if self._new_data:
            return self._new_data
        return self._decompress(self._original_compressed_data)

    @cached_property
    def text(self) -> str:
        return format(self.data)

    def setText(self, text: str) -> None:
        invalidate_property(self, "data")
        invalidate_property(self, "text")
        self._new_data = unformat(text)

    @staticmethod
    def _pushByte(data: List[int], value: int) -> None:
        assert value <= 0xFF
        data.append(value)

    @staticmethod
    def _pushWord(data: List[int], value: int) -> None:
        assert value <= 0xFFFF
        Sentence._pushByte(data, value & 0xFF)
        Sentence._pushByte(data, value >> 8)

    @staticmethod
    def _specialPush(data: List[int], value: int) -> None:
        Sentence._pushByte(data, value)
        if value >= 0x80:
            Sentence._pushByte(data, 0x01)

    def _decompress(self, data: List[int]) -> List[int]:
        output: List[int] = []

        i = 0
        while True:
            byte_type = self._rom.readByte(TYPE_INDEX1 + data[i])

            if byte_type in (1, 2, 3, 4, 7):
                self._specialPush(output, data[i])
                i += 1

            elif byte_type == 5:
                self._pushWord(output, 0xFFFF)
                return output

            elif byte_type == 6:
                for _ in range(4):
                    self._pushByte(output, data[i])
                    i += 1
            elif byte_type == 9:
                for _ in range(3):
                    self._pushByte(output, data[i])
                    i += 1
            elif byte_type == 8:
                for _ in range(2):
                    self._pushByte(output, data[i])
                    i += 1

            elif 0x10 <= byte_type <= 0x4F:
                byte = byte_type - 0x10

                index = data[i + 1]
                index += (byte_type & 0x0F) << 8
                index += 0x6E
                index *= 2
                index = self._rom.readWord(TYPE_INDEX2 + index)
                i += 2

                fragment_start = len(output)
                done = False
                while True:
                    index2 = self._rom.readByte(TYPE_INDEX3 + index) * 2
                    index += 1
                    if index2 == 0:
                        if byte & 0x20:
                            output[fragment_start] -= 0x1A
                            assert output[fragment_start + 1] != 0
                        if byte & 0x10:
                            self._specialPush(output, 0x44)
                        done = True
                        break
                    else:
                        word = self._rom.readWord(TYPE_INDEX4 + index2)
                        self._specialPush(output, word & 0xFF)

                        word >>= 8
                        if word == 5:
                            if byte & 0x20:
                                output[fragment_start] -= 0x1A
                                assert output[fragment_start + 1] != 0
                            if byte & 0x10:
                                self._specialPush(output, 0x44)
                            done = True
                            break
                        elif word != 1:
                            self._specialPush(output, word)
                if done:
                    continue

            elif 0x50 <= byte_type <= 0xFF:
                type_index = (byte_type - 0x50) * 2
                index = self._rom.readWord(TYPE_INDEX2 + type_index)

                while True:
                    index2 = self._rom.readByte(TYPE_INDEX3 + index) * 2
                    index += 1
                    if index2 == 0:
                        break

                    word = self._rom.readWord(TYPE_INDEX4 + index2)
                    self._specialPush(output, word & 0xFF)

                    word >>= 8
                    if word == 5:
                        break
                    elif word != 1:
                        self._specialPush(output, word)
                i += 1

            else:
                assert False

        return output

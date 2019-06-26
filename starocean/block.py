from enum import IntEnum
from typing import List

from cached_property import cached_property

from romhacking.compression import encode_12bit
from romhacking.utils import convert16to8, pairwise, readWord, writeWord
from starocean.layout import BLOCK_INDEX, HUFFMAN_TREE
from starocean.sentence import Sentence

HEADER_SIZE = 9


class BlockType(IntEnum):
    TEXT = 0x85
    EVENTS = 0x86
    OTHER = 0x80


class Block:
    def __init__(self, rom, index: int):
        self._rom = rom
        self.index = index
        self.start = rom.readAddress(self._block_index_address)

        self.header: List[int] = []
        self.indexes: List[int] = []
        self.content: List[int] = []
        self._parseBlock()

        self._new_data = False

    @property
    def type(self) -> BlockType:
        return BlockType(self.header[0])

    @property
    def size(self) -> int:
        return readWord(self.header, 1) + HEADER_SIZE

    @property
    def end(self) -> int:
        return self.start + self.size

    @property
    def data(self) -> List[int]:
        return self.header + convert16to8(self.indexes) + self.content

    @cached_property
    def sentences(self) -> List[Sentence]:
        assert not self._new_data
        last_index = (self._indexes_size + len(self.content) - 1) * 8

        sentences = []
        for i, j in pairwise(self.indexes + [last_index]):
            sentence = Sentence(self._rom, self._decompressRange(i, j))
            sentences.append(sentence)
        return sentences

    def setData(self, compressed_data: List[List[int]]) -> None:
        self.indexes, self.content = encode_12bit(compressed_data)

        size = len(self.content) + len(self.indexes) * 2
        writeWord(size, self.header, 1)

        self._new_data = True

    def reinsert(self) -> None:
        self._rom.writeAddress(self._block_index_address, self.start)
        self._rom.write(self.start, self.data)

    @property
    def _block_index_address(self):
        return BLOCK_INDEX + (self.index * 3)

    @property
    def _indexes_start(self) -> int:
        return self.start + HEADER_SIZE

    @property
    def _indexes_size(self) -> int:
        size = self._content_start - self._indexes_start
        return size

    @property
    def _content_start(self) -> int:
        offset = self._rom.readWord(self.start + 3) + 3
        return self.start + offset

    def _parseBlock(self) -> None:
        assert len(self.header) == len(self.indexes) == len(self.content) == 0

        for i in range(HEADER_SIZE):
            self.header.append(self._rom.readByte(self.start + i))
        for i in range(self._indexes_start, self._content_start, 2):
            self.indexes.append(self._rom.readWord(i))
        for i in range(self._content_start, self.end):
            self.content.append(self._rom.readByte(i))

    def _decompressRange(self, index: int, next_index: int) -> List[int]:
        assert not self._new_data
        output: List[int] = []

        next_byte_index = (next_index // 8) - self._indexes_size
        byte_index = (index // 8) - self._indexes_size
        bit_index = index % 8

        while byte_index <= next_byte_index:
            node = 0x00FE

            while not (node & 0x8000):
                try:
                    byte = self.content[byte_index]
                except IndexError:
                    return output

                huffman_index = node * 4
                if byte & (1 << bit_index):
                    huffman_index += 2

                node = self._rom.readWord(HUFFMAN_TREE + huffman_index)

                if bit_index == 7:
                    byte_index += 1
                    bit_index = 0
                else:
                    bit_index += 1
            output.append(node & 0xFF)

        return output

import re
from os.path import dirname, join
from typing import List, Tuple

from cached_property import cached_property

from romhacking.compression import bytepairCompress
from romhacking.font import Font
from romhacking.malloc import Heap
from romhacking.rom import ROM as GenericROM
from romhacking.rom import ROMType
from starocean.block import Block, BlockType
from starocean.layout import (DIALOGUE_FONT, UNUSED_AREA1, UNUSED_AREA2,
                              UNUSED_AREA3)

N_BLOCKS = 188
N_CHARS = 0x60
SHA1_ORIGINAL = '8574f0c49b0e823f21763331c2d66225b95c1653'


def asm_path(file_name: str) -> str:
    return join(dirname(__file__), 'asm', file_name)


class ROM(GenericROM):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        assert self.sha1() == SHA1_ORIGINAL

        self.type = ROMType.HiROM
        self.heap = self._setupFreeSpace()

    @cached_property
    def blocks(self) -> List[Block]:
        blocks = []
        for i in range(N_BLOCKS):
            blocks.append(Block(self, i))
        return blocks

    @cached_property
    def font(self) -> Font:
        return Font(self.read(DIALOGUE_FONT, N_CHARS * 0x20))

    def extract(self) -> str:
        dump = ''
        for block in self.blocks:
            for i, sentence in enumerate(block.sentences):
                dump += f'<BLOCK {block.index}, SENTENCE {i}>\n'
                dump += sentence.text + '\n\n\n'
        return dump

    def reinsert(self, dump: str) -> None:
        self._parseDump(dump)
        block_indexes, blocks_data = self._collectTextBlocks()

        dictionary, compressed_data = bytepairCompress(blocks_data)
        dictionary_start = self._insertDictionary(dictionary)

        for i, block_data in enumerate(compressed_data):
            block = self.blocks[block_indexes[i]]
            block.setData(block_data)
            block.start = self.heap.allocate(block.size)
            block.reinsert()

        self.save()
        self.assemble(asm_path('decompress.asm'),
                      {'DICTIONARY': '{:06X}'.format(dictionary_start)})

    def _parseDump(self, dump: str) -> None:
        matches = re.findall(
            r'<BLOCK (\d+), SENTENCE (\d+)>\n((?:.*?)<CLOSE>)', dump,
            re.DOTALL)

        for match in matches:
            block = int(match[0])
            sentence = int(match[1])
            text = match[2]
            self.blocks[block].sentences[sentence].setText(text)

    def _collectTextBlocks(self) -> Tuple[List[int], List[List[List[int]]]]:
        block_indexes = []
        blocks_data = []

        for block in self.blocks:
            if block.type == BlockType.TEXT:
                blocks_data.append([s.data for s in block.sentences])
                block_indexes.append(block.index)

        return block_indexes, blocks_data

    def _setupFreeSpace(self) -> Heap:
        heap = Heap(self)

        for block in self.blocks:
            if block.type == BlockType.TEXT:
                heap.addFreeArea(block.start, block.end)

        heap.addFreeArea(*UNUSED_AREA1)
        heap.addFreeArea(*UNUSED_AREA2)
        heap.addFreeArea(*UNUSED_AREA3)

        return heap

    def _insertDictionary(self, dictionary: List[Tuple[int, int]]) -> int:
        size = len(dictionary) * 4
        start = self.heap.allocate(size)
        for i, pair in enumerate(dictionary):
            self.writeWord(start + (i * 4), pair[0])
            self.writeWord(start + (i * 4) + 2, pair[1])
        return start

    def _translate(self, address: int) -> int:
        if address < 0x010000:
            return address - 0x8000
        return address & 0x3FFFFF

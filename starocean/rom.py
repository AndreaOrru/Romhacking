import re
from os.path import dirname, join
from textwrap import wrap
from typing import List, Tuple

from functools import cached_property

from romhacking.compression import bytepairCompress
from romhacking.font import Font
from romhacking.malloc import Heap
from romhacking.rom import ROM as GenericROM
from romhacking.rom import ROMType
from starocean.block import Block, BlockType
from starocean.layout import DIALOGUE_FONT, UNUSED_AREA1, UNUSED_AREA2, UNUSED_AREA3
from starocean.rom_version import ROMVersion

N_BLOCKS = 188
N_CHARS = 0x60
SHA1_DEJAP = "8574f0c49b0e823f21763331c2d66225b95c1653"
SHA1_MAGNO = "561495e007b1739fbcfb4cf26657db8744c17c5c"

# Blocks used in the intro.
BLOCK_BLACKLIST = [59]


def asm_path(file_name: str) -> str:
    return join(dirname(__file__), "asm", file_name)


class ROM(GenericROM):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        sha1 = self.sha1()
        if sha1 == SHA1_DEJAP:
            self.version = ROMVersion.DEJAP
        elif sha1 == SHA1_MAGNO:
            self.version = ROMVersion.MAGNO
        else:
            assert False

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

    @property
    def text_type(self):
        if self.version == ROMVersion.DEJAP:
            return BlockType.DEJAP_TEXT
        else:
            return BlockType.MAGNO_TEXT

    def extract(self) -> str:
        dump = ""
        for block in self.blocks:
            if block.index in BLOCK_BLACKLIST:
                continue
            if block.sentences:
                dump += f"<HEADER {block.index}, {block.header_str}>\n"
            for i, sentence in enumerate(block.sentences):
                dump += f"<BLOCK {block.index}, SENTENCE {i}>\n"
                dump += sentence.text + "\n\n\n"
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

        asm_file = "dejap.asm" if self.version == ROMVersion.DEJAP else "magno.asm"
        self.assemble(
            asm_path(asm_file),
            {"DICTIONARY": "{:06X}".format(dictionary_start)},
        )

    def _parseDump(self, dump: str) -> None:
        self._parseHeaders(dump)
        matches = re.findall(
            r"<BLOCK (\d+), SENTENCE (\d+)>\n((?:.*?)<CLOSE>)", dump, re.DOTALL
        )

        for match in matches:
            block = int(match[0])
            sentence = int(match[1])
            text = match[2]
            self.blocks[block].sentences[sentence].setText(text)

    def _parseHeaders(self, dump: str) -> None:
        matches = re.findall(r"<HEADER (\d+), ([0-9A-F]+)>", dump)
        for match in matches:
            block = int(match[0])
            header = [int(x, 16) for x in wrap(match[1], 2)]
            self.blocks[block].header = header
            self.blocks[block]._parseBlock()

    def _collectTextBlocks(self) -> Tuple[List[int], List[List[List[int]]]]:
        block_indexes = []
        blocks_data = []

        for block in self.blocks:
            if block.type == BlockType.DEJAP_TEXT:
                blocks_data.append([s.data for s in block.sentences])
                block_indexes.append(block.index)

        return block_indexes, blocks_data

    def _setupFreeSpace(self) -> Heap:
        heap = Heap(self)

        for block in self.blocks:
            if block.index in BLOCK_BLACKLIST:
                continue
            if block.type == self.text_type:
                heap.addFreeArea(block.start, block.end)

        if self.version == ROMVersion.DEJAP:
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

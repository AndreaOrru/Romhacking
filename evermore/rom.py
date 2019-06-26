import re
from os.path import dirname, join
from typing import List, Tuple

from cached_property import cached_property

from evermore.formatting import unformat
from evermore.layout import (DTES, MTES, MTES_END, POINTERS, POINTERS_END,
                             RELOCATED_TEXT, SCATTER_TABLE)
from evermore.sentence import Sentence
from romhacking.malloc import Heap
from romhacking.rom import ROM as GenericROM

SHA1_ORIGINAL = '79e7738630fff5699217ef58ecc421bc8fcbcd89'
N_SENTENCES = 3002


def asm_path(file_name: str) -> str:
    return join(dirname(__file__), 'asm', file_name)


class ROM(GenericROM):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        assert self.sha1() == SHA1_ORIGINAL

    @cached_property
    def pointers(self) -> List[Tuple[int, bool]]:
        pointers = []
        for i in range(POINTERS, POINTERS_END, 3):
            pointer = self.readAddress(i)
            is_ascii = not (pointer & 0x800000)
            bank = ((pointer >> 15) & 0x7F) + 0xC0
            pointer = (bank << 16) | (pointer & 0x7FFF)
            pointers.append((pointer, is_ascii))
        return pointers

    @cached_property
    def dtes(self) -> List[str]:
        dtes = []
        for i in range(64):
            dte = chr(self.readByte(DTES + (i * 2)))
            dte += chr(self.readByte(DTES + (i * 2) + 1))
            dtes.append(dte)
        return dtes

    @cached_property
    def mtes(self) -> List[str]:
        mte = ''
        mtes = []
        for i in range(MTES, MTES_END):
            c = self.readByte(i)
            if c != 0x00:
                mte += chr(c)
            else:
                mtes.append(mte)
                mte = ''
        return mtes

    @cached_property
    def scatter_table(self) -> List[str]:
        table = []
        for i in range(64):
            c = chr(self.readByte(SCATTER_TABLE + i))
            table.append(c)
        return table

    @cached_property
    def sentences(self) -> List[Sentence]:
        sentences = []
        for address, is_ascii in self.pointers:
            sentences.append(Sentence(self, address, is_ascii))
        return sentences

    def extract(self) -> str:
        dump = ''
        for i, sentence in enumerate(self.sentences):
            dump += sentence.text + '\n'
        return dump

    def reinsert(self, dump: str) -> None:
        sentences = self._parseDump(dump)

        # Expand to 4MB to free up 1MB.
        self.expand(0x400000)
        heap = Heap(self)
        heap.addFreeArea(RELOCATED_TEXT, RELOCATED_TEXT + 0x100000)

        # Reinsert dialogue and recompute pointers.
        for i, sentence in enumerate(sentences):
            pointer = heap.allocate(len(sentence))
            self.write(pointer, sentence)
            self.writeAddress(POINTERS + i * 3, pointer)

        self.save()
        self.assemble(asm_path('text.asm'))
        self.assemble(asm_path('font.asm'))
        self.assemble(asm_path('thousands_separator.asm'))

    def _parseDump(self, dump: str) -> List[List[int]]:
        matches = re.findall(r'((?:.*?)<End>)\n', dump, re.DOTALL)
        assert len(matches) == N_SENTENCES

        sentences = []
        for match in matches:
            sentence = unformat(match)
            sentences.append(sentence)
        return sentences

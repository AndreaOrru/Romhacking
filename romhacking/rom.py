from array import array
from enum import Enum, auto
from hashlib import sha1
from itertools import repeat
from math import ceil, log2
from os.path import split
from subprocess import call
from typing import Dict, List, Optional

PREFIX_SAVED_ROM = "patched_"


class ROMType(Enum):
    PC = auto()
    LoROM = auto()
    HiROM = auto()


class Header:
    TITLE = 0xFFC0
    TYPE = 0xFFD6
    SIZE = 0xFFD7
    NMI = 0xFFEA
    RESET = 0xFFFC


class ROM:
    def __init__(self, path: str):
        self.path = path
        with open(path, "rb") as f:
            self.data = array("B", f.read())
        self.type = self._discoverType()

    @property
    def size(self) -> int:
        return 0x400 << self.readByte(Header.SIZE)

    @size.setter
    def size(self, n_bytes: int):
        size = ceil(log2(n_bytes / 0x400))
        self.writeByte(Header.SIZE, size)

    @property
    def title(self) -> str:
        title = ""
        for i in range(21):
            c = self.readByte(Header.TITLE + i)
            if c == 0:
                break
            title += chr(c)
        return title

    @property
    def reset_vector(self) -> int:
        return self.readWord(Header.RESET)

    @property
    def nmi_vector(self) -> int:
        return self.readWord(Header.NMI)

    @property
    def save_path(self):
        folder, name = split(self.path)
        return f"{folder}/{PREFIX_SAVED_ROM}{name}"

    def readByte(self, address: int) -> int:
        return self.data[self._translate(address)]

    def readWord(self, address: int) -> int:
        lo = self.readByte(address)
        hi = self.readByte(address + 1)
        return (hi << 8) | lo

    def readAddress(self, address: int) -> int:
        lo = self.readWord(address)
        hi = self.readByte(address + 2)
        return (hi << 16) | lo

    def read(self, address: int, n_bytes: int) -> List[int]:
        data = []
        for i in range(n_bytes):
            data.append(self.readByte(address + i))
        return data

    def writeByte(self, address: int, value: int) -> None:
        assert value <= 0xFF
        self.data[self._translate(address)] = value

    def writeWord(self, address: int, value: int) -> None:
        assert value <= 0xFFFF
        self.writeByte(address, value & 0xFF)
        self.writeByte(address + 1, value >> 8)

    def writeAddress(self, address: int, value: int) -> None:
        assert value <= 0xFFFFFF
        self.writeWord(address, value & 0xFFFF)
        self.writeByte(address + 2, value >> 16)

    def write(self, address: int, data: List[int]) -> None:
        for i, c in enumerate(data):
            self.writeByte(address + i, c)

    def save(self):
        with open(self.save_path, "wb") as f:
            f.write(self.data.tobytes())

    def expand(self, new_size: int) -> None:
        actual_size = len(self.data)
        assert new_size >= self.size
        assert new_size >= actual_size

        self.data.extend(repeat(0, new_size - actual_size))
        self.size = new_size

    def assemble(self, asm_path: str, defines: Optional[Dict[str, str]] = None) -> None:
        if defines is None:
            defines = {}
        args = [f"-D{k}={v}" for k, v in defines.items()]
        call(["asar", *args, asm_path, self.save_path])

    def sha1(self):
        return sha1(self.data).hexdigest()

    def _translate(self, address: int) -> int:
        if self.type == ROMType.HiROM:
            return address & 0x3FFFFF
        elif self.type == ROMType.LoROM:
            return ((address & 0x7F0000) >> 1) | (address & 0x7FFF)
        else:
            return address

    def _discoverType(self) -> ROMType:
        if len(self.data) <= 0x8000:
            return ROMType.LoROM
        lorom_score = self._typeScore(ROMType.LoROM)
        hirom_score = self._typeScore(ROMType.HiROM)
        if hirom_score > lorom_score:
            return ROMType.HiROM
        else:
            return ROMType.LoROM

    def _typeScore(self, rom_type: ROMType) -> int:
        title = Header.TITLE
        if rom_type == ROMType.LoROM:
            title -= 0x8000

        score = 0
        for i in range(21):
            c = self.data[title + i]
            if c == 0x00:
                score += 1
            elif chr(c).isprintable():
                score += 2
            else:
                return 0
        return score

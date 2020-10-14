from abc import ABC
from copy import copy
from os.path import dirname, join, splitext
from subprocess import call
from unittest import TestCase
from unittest.mock import mock_open, patch

from romhacking.rom import ROM, ROMType


def rom_path(file_name: str) -> str:
    return join(dirname(__file__), "roms", file_name)


def assemble(file_name: str) -> str:
    asm = rom_path(file_name)
    rom = splitext(asm)[0] + ".sfc"
    call(["asar", asm, rom])
    return rom


class ROMTestCase(ABC):
    def test_size(self):
        self.assertEqual(self.rom.size, 2048)

    def test_title(self):
        self.assertEqual(self.rom.title, "TEST")

    def test_reset_vector(self):
        self.assertEqual(self.rom.reset_vector, 0x8000)

    def test_nmi_vector(self):
        self.assertEqual(self.rom.nmi_vector, 0x9000)

    def test_read(self):
        byte = ord("T")
        word = ord("T") | (ord("E") << 8)
        addr = ord("T") | (ord("E") << 8) | (ord("S") << 16)
        self.assertEqual(self.rom.readByte(0xFFC0), byte)
        self.assertEqual(self.rom.readWord(0xFFC0), word)
        self.assertEqual(self.rom.readAddress(0xFFC0), addr)
        self.assertListEqual(self.rom.read(0xFFC0, 4), [ord(x) for x in "TEST"])

    def test_write(self):
        rom = copy(self.rom)
        byte = ord("A")
        word = ord("A") | (ord("B") << 8)
        addr = ord("A") | (ord("B") << 8) | (ord("C") << 16)
        string = [ord(x) for x in "ABCD"]

        rom.writeByte(0xFFC0, byte)
        assert rom.readByte(0xFFC0) == byte

        rom.writeWord(0xFFC0, word)
        assert rom.readWord(0xFFC0) == word

        rom.writeAddress(0xFFC0, addr)
        assert rom.readAddress(0xFFC0) == addr

        rom.write(0xFFC0, string)
        assert rom.read(0xFFC0, 4) == string

    def test_save(self):
        m = mock_open()
        with patch("romhacking.rom.open", m):
            self.rom.save()
        m.assert_called_once()


class LoROMTest(ROMTestCase, TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = ROM(assemble("lorom.asm"))

    def test_type(self):
        self.assertEqual(self.rom.type, ROMType.LoROM)

    def test_translate(self):
        self.assertEqual(self.rom._translate(0x008000), 0x000000)
        self.assertEqual(self.rom._translate(0x108000), 0x080000)
        self.assertEqual(self.rom._translate(0x808000), 0x000000)


class HiROMTest(ROMTestCase, TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = ROM(assemble("hirom.asm"))

    def test_type(self):
        self.assertEqual(self.rom.type, ROMType.HiROM)

    def test_translate(self):
        self.assertEqual(self.rom._translate(0xC00000), 0x000000)
        self.assertEqual(self.rom._translate(0xC08000), 0x008000)
        self.assertEqual(self.rom._translate(0x400000), 0x000000)

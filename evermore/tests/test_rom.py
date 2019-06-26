from evermore.rom import N_SENTENCES
from romhacking.rom import ROMType

from .base import TestCase


class ROMTestCase(TestCase):
    def test_type(self):
        self.assertEqual(self.rom.type, ROMType.HiROM)

    def test_title(self):
        self.assertTrue(self.rom.title.startswith('SECRET OF EVERMORE'))

    def test_address_translation(self):
        self.assertEqual(self.rom.readAddress(0xD1D0C0), 0x800378)

    def test_pointers(self):
        self.assertEqual(len(self.rom.pointers), N_SENTENCES)

        address, is_ascii = self.rom.pointers[421]
        self.assertEqual(address, 0xC02667)
        self.assertFalse(is_ascii)

    def test_dtes(self):
        self.assertEqual(len(self.rom.dtes), 64)
        self.assertEqual(self.rom.dtes[0], 'e ')
        self.assertEqual(self.rom.dtes[-1], 'me')

    def test_mtes(self):
        self.assertEqual(len(self.rom.mtes), 319)
        self.assertEqual(self.rom.mtes[0], 'alchemy')
        self.assertEqual(self.rom.mtes[-1], "You've")

    def test_sentences(self):
        self.assertEqual(len(self.rom.sentences), N_SENTENCES)
        self.assertFalse(self.rom.sentences[421].is_ascii)
        self.assertTrue(self.rom.sentences[422].is_ascii)

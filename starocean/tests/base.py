import unittest
from abc import ABC
from os.path import dirname, join

from starocean.rom import ROM


def rom_path(file_name: str) -> str:
    return join(dirname(__file__), 'roms', file_name)


class TestCase(ABC, unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rom = ROM(rom_path('so.sfc'))

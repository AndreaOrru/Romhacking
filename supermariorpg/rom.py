from romhacking.rom import ROM as GenericROM
from romhacking.rom import ROMType

SHA1_ORIGINAL = "a4f7539054c359fe3f360b0e6b72e394439fe9df"


class ROM(GenericROM):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        assert self.sha1() == SHA1_ORIGINAL

        self.type = ROMType.PC

    def extract(self) -> str:
        dump = ""
        # for block in self.blocks
        return dump

    def reinsert(self, dump: str) -> None:
        pass

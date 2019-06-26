from typing import Tuple

from sortedcontainers import SortedList

from romhacking.rom import ROM, ROMType


class Heap:
    def __init__(self, rom: ROM):
        self._rom = rom
        self.areas = SortedList()

    def addFreeArea(self, start: int, end: int) -> None:
        assert (start <= 0xFFFFFF) and (end <= 0xFFFFFF + 1)

        if self._rom.type == ROMType.LoROM:
            raise NotImplementedError
        assert not self._intersectsAreas(start, end)

        i = start
        while i < end:
            j = min(end, (i + 0x10000) & ~0xFFFF)

            area_index = self.areas.bisect_left((i, j))
            if area_index > 0:
                prev_area_index = area_index - 1
                prev_area = self.areas[prev_area_index]
                if prev_area[1] == i and (prev_area[1] & 0xFFFF):
                    del self.areas[prev_area_index]
                    i = prev_area[0]

            self.areas.add((i, j))
            i = j

    def allocate(self, size: int) -> int:
        for i, area in enumerate(self.areas):
            area_size = area[1] - area[0]
            if area_size >= size:
                return self._splitArea(i, size)[0]
        assert False

    def _intersectsAreas(self, start: int, end: int) -> bool:
        i = self.areas.bisect_right((start, end))
        if i > 0:
            if self.areas[i - 1][1] > start:
                return True
        if i < len(self.areas):
            if self.areas[i][0] < end:
                return True
        return False

    def _splitArea(self, index: int, size: int) -> Tuple[int, int]:
        area = self.areas[index]
        assert area[1] - area[0] >= size

        split = area[0] + size
        busy_part = (area[0], split)
        free_part = (split, area[1])

        del self.areas[index]
        self.areas.add(free_part)

        return busy_part

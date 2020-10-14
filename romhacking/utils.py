from itertools import tee
from typing import List


def invalidate_property(instance, attribute):
    instance.__dict__.pop(attribute, None)


def pairwise(iterable):
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)


def readWord(data: List[int], index: int) -> int:
    lo = data[index]
    hi = data[index + 1]
    assert (lo <= 0xFF) and (hi <= 0xFF)
    return (hi << 8) | lo


def readAddress(data: List[int], index: int) -> int:
    lo = readWord(data, index)
    hi = data[index + 2]
    assert (lo <= 0xFFFF) and (hi <= 0xFF)
    return (hi << 16) | lo


def writeWord(value: int, data: List[int], index: int) -> None:
    assert value <= 0xFFFF
    data[index] = value & 0xFF
    data[index + 1] = value >> 8


def writeAddress(value: int, data: List[int], index: int) -> None:
    assert value <= 0xFFFFFF
    writeWord(value & 0xFFFF, data, index)
    data[index + 2] = value >> 16


def convert16to8(data: List[int]) -> List[int]:
    output = []
    for word in data:
        assert word <= 0xFFFF
        output.append(word & 0xFF)
        output.append(word >> 8)
    return output

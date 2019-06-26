from typing import List, Tuple

from romhacking.bytepair import bytepairCompress  # noqa


def encode_12bit(strings: List[List[int]]) -> Tuple[List[int], List[int]]:
    indexes: List[int] = []
    output: List[int] = []

    index = len(strings) * 2 * 8
    shifted = False

    for string in strings:
        assert index <= 0xFFFF
        indexes.append(index)

        for c in string:
            assert c <= 0xFFF
            if shifted:
                output[-1] |= (c >> 8)
                output.append((c & 0xFF))
            else:
                output.append((c >> 4))
                output.append((c & 0xF) << 4)
            shifted = not shifted
            index += 12

    return indexes, output

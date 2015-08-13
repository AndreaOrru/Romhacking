#pragma once
#include <list>
#include <utility>
#include <vector>
#include "block.hpp"
#include "types.hpp"


namespace BytePair
{

const std::vector<u8>& get(u16 w);
const std::vector<u32>& get_dict();
std::vector<std::list<u16>> compress(const std::vector<Block>& blocks);

}

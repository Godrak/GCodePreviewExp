#ifndef BITSET_H_
#define BITSET_H_

#include <vector>

namespace bitset {

template <typename T = unsigned long long>
class BitSet {
public:
    BitSet(unsigned int size) : size(size), blocks((size + sizeof(T) - 1) / sizeof(T)) { clear(); }

    BitSet() : size(0), blocks(0) {}

    BitSet &operator=(const BitSet &other)
    {
        if (this == &other) {
            return *this;
        }

        size   = other.size;
        blocks = other.blocks;

        return *this;
    }

    void clear() { blocks.assign(blocks.size(), 0); }

    void setAll() { blocks.assign(blocks.size(), ~T(0)); }

    void set(unsigned int index) {
        const auto [block_idx, bit_idx] = get_coords(index);
        blocks[block_idx] |= (static_cast<T>(1) << bit_idx);
    }

    void reset(unsigned int index) {
        const auto [block_idx, bit_idx] = get_coords(index);
        blocks[block_idx] &= ~(static_cast<T>(1) << bit_idx);
    }

    bool operator[](unsigned int index) const {
        const auto [block_idx, bit_idx] = get_coords(index);
        return ((blocks[block_idx] >> bit_idx) & 1) != 0;
    }

    unsigned int size;
    std::vector<T> blocks;

    std::pair<unsigned int, unsigned int> get_coords(unsigned int index) const {
        unsigned int block_idx = index / (sizeof(T) * 8);
        unsigned int bit_idx = index % (sizeof(T) * 8);
        return std::make_pair(block_idx, bit_idx);
    }
};

}

#endif //BITSET_H_
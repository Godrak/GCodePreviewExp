#ifndef BITSET_H_
#define BITSET_H_

#include <atomic>
#include <vector>

namespace bitset {

// By default, types are not atomic,
template<typename T> auto constexpr is_atomic = false;

// but std::atomic<T> types are,
template<typename T> auto constexpr is_atomic<std::atomic<T>> = true;

template<typename T = unsigned long long> 
class BitSet
{
public:
    BitSet(unsigned int size) : size(size), blocks((size + sizeof(T) - 1) / sizeof(T)) { clear(); }

    BitSet() : size(0), blocks(0) {}

    void clear()
    {
        for (unsigned int i = 0; i < blocks.size(); ++i) {
            blocks[i] &= T(0);
        }
    }

    void setAll()
    {
        for (unsigned int i = 0; i < blocks.size(); ++i) {
            blocks[i] |= ~T(0);
        }
    }

    void set(unsigned int index)
    {
        const auto [block_idx, bit_idx] = get_coords(index);
        blocks[block_idx] |= (T(1) << bit_idx);
    }

    void reset(unsigned int index)
    {
        const auto [block_idx, bit_idx] = get_coords(index);
        blocks[block_idx] &= ~(T(1) << bit_idx);
    }

    bool operator[](unsigned int index) const
    {
        const auto [block_idx, bit_idx] = get_coords(index);
        return ((blocks[block_idx] >> bit_idx) & 1) != 0;
    }

    template<typename U> void getEnabledIndices(std::vector<U> &dest) const
    {
        for (unsigned int block_idx = 0; block_idx < blocks.size(); ++block_idx) {
            T block = blocks[block_idx].load();
            while (block != 0) {
                unsigned int bit_idx = __builtin_ctzll(block); // Find the index of the least significant bit
                unsigned int index   = block_idx * (sizeof(T) * 8) + bit_idx;
                dest.push_back(index);
                block &= (block - 1); // Clear the least significant bit
            }
        }
    }

    BitSet operator&(const BitSet &other) const
    {
        BitSet result(size);
        for (unsigned int i = 0; i < blocks.size(); ++i) {
            result.blocks[i] = blocks[i] & other.blocks[i];
        }
        return result;
    }

    BitSet operator|(const BitSet &other) const
    {
        BitSet result(size);
        for (unsigned int i = 0; i < blocks.size(); ++i) {
            result.blocks[i] = blocks[i] | other.blocks[i];
        }
        return result;
    }

    // Atomic set operation (enabled only for atomic types), return true if bit changed
    template<typename U = T> inline typename std::enable_if<is_atomic<U>, bool>::type set_atomic(unsigned int index)
    {
        const auto [block_idx, bit_idx] = get_coords(index);
        T mask                          = static_cast<T>(1) << bit_idx;
        T oldval                        = blocks[block_idx].fetch_or(mask, std::memory_order_relaxed);
        return oldval xor (oldval or mask);
    }

    // Atomic reset operation (enabled only for atomic types), returns old value
    template<typename U = T> inline typename std::enable_if<is_atomic<U>, bool>::type reset_atomic(unsigned int index)
    {
        const auto [block_idx, bit_idx] = get_coords(index);
        T mask                          = ~(static_cast<T>(1) << bit_idx);
        T oldval                        = blocks[block_idx].fetch_and(mask, std::memory_order_relaxed);
        return oldval xor (oldval and mask);
    }

    std::pair<unsigned int, unsigned int> get_coords(unsigned int index) const
    {
        unsigned int block_idx = index / (sizeof(T) * 8);
        unsigned int bit_idx   = index % (sizeof(T) * 8);
        return std::make_pair(block_idx, bit_idx);
    }

    unsigned int   size;
    std::vector<T> blocks;
};

} // namespace bitset

#endif //BITSET_H_

#ifndef LIBMEM_HPP
#define LIBMEM_HPP

#include "memblock.hpp"
#include "utils/utils.hpp"

#include <cassert>
#include <concepts>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace LibMem {

// This class should ideally
// not do more work than
// it *has* to, so that the
// interface and the code
// remains clean
template <size_t MAXSIZE> class Allocator {

public:
	Allocator() {
		this->m_space = std::malloc(MAXSIZE);

		if (this->m_space == NULL) {
			throw std::runtime_error("malloc has failed");
		}

		this->m_total_available = MAXSIZE;
		this->m_total_padding = 0;
	}

	~Allocator() { std::free(this->m_space); }

	/**
	 * @brief Allocates memory and gives back the MemBlock object
	 * @param amount The amount of elements to be allocated of `typesize` each
	 * @param typesize The size of each element to be allocated
	 * @returns MemBlock structure populated with details about the memory
	 */
	template <size_t AMT, typename T> auto allocate() -> MemBlock<T> {
		constexpr size_t memsize = AMT * sizeof(T);
		constexpr size_t memsize_padded = Utils::round_pow_two(memsize);
		constexpr size_t padding = memsize_padded - memsize;

		static_assert(!(memsize_padded > MAXSIZE),
					  "Invalid Allocation: The requested size exceeds the "
					  "maximum allowed size.");

		// if the memory is available: allocate
		// else: try and defragment
		//       if defrag fails: throw OOM and die
		if (memsize_padded > this->m_total_available) {
			try {
				this->m_defragment<T>(memsize_padded);
			} catch (...) {
				throw;
			}
		}

		const size_t total_used = MAXSIZE - this->m_total_available;
		BlockInfo reused_block = this->reuse_block(memsize_padded);

		if (reused_block.addr != nullptr) {
			// add it to the allocated blocks
			this->m_blocks.push_back(std::move(reused_block));
		} else {
			auto block_begin =
				static_cast<T*>(this->m_space) + (total_used / sizeof(T));

			// add it to the allocated blocks
			this->m_blocks.push_back(std::move(
				BlockInfo{block_begin, padding, memsize_padded, false}));
		}

		// increment the class variables
		this->m_total_padding += padding;
		this->m_total_available -= memsize_padded;

		return MemBlock<T>(&this->m_blocks.back(), AMT);
	}

	template <typename T> auto free(MemBlock<T>& block) -> void {

		// get the block at that index and mark it as free
		for (auto& current_block : m_blocks) {
			if (current_block.addr == block.getptr() &&
				current_block.size == block.getsize()) {

				this->m_total_available += current_block.size;
				this->m_total_padding -= current_block.padding;

				auto idx = &current_block - &m_blocks[0];
				m_blocks.erase(m_blocks.begin() + idx);
			}
		}
	};

	/**
	 * @brief Re-use a previously freed memory block
	 * @returns A pair containing the pointer to the start of the block and it's
	 * index
	 */
	auto reuse_block(const size_t size) -> BlockInfo {
		BlockInfo retval{nullptr, 0, 0, false};

		BlockInfo* free_block = nullptr;
		for (auto& current_block : this->m_blocks) {
			if (current_block.free && current_block.size <= size) {
				free_block = &current_block;
			}
		}
		if (free_block == nullptr) {
			return retval;
		}

		retval.addr = free_block->addr;

		// remove `size` from the free_block at `index` in the array
		// by advancing the pointer by `size` units
		if (free_block->size != size) {

			// Advance the pointer, decrease the size
			free_block->addr = static_cast<size_t*>(free_block->addr) + size;
			free_block->padding = 0;
			free_block->size = free_block->size - size;
			free_block->free = true;

			this->m_total_available += free_block->size - size;
			this->m_total_padding -= free_block->padding;

			return retval;
		}

		this->m_total_available += free_block->size;
		this->m_total_padding -= free_block->padding;

		// if the block was fully used, remove it from the list
		for (auto& current_block : m_blocks) {
			if (current_block.addr == free_block->addr) {
				auto idx = &current_block - &m_blocks[0];
				m_blocks.erase(m_blocks.begin() + idx);
			}
		}

		return retval;
	}

	/**
	 * @brief Resets the allocator, voiding all previous allocations.
	 * @note Using any `MemBlock`s allocated before a reset is undefined
	 * behaviour after reset is called.
	 */
	auto reset() -> void {
		m_total_available = MAXSIZE;
		m_total_padding = 0;

		this->m_blocks.clear();
	}

	template <typename T> auto reallocate(MemBlock<T>& memblock) -> void {
		throw std::runtime_error(
			"Call to unimplemented function: reallocate()");
	};

private:
	void* m_space;

	size_t m_total_available;
	size_t m_total_padding;

	std::vector<BlockInfo> m_blocks;

	template <typename T> auto m_defragment(size_t memsize) -> void {
		throw std::runtime_error("Un-implemented: Defrag");
	}
};

} // namespace LibMem

#endif /* LIBMEM_HPP */
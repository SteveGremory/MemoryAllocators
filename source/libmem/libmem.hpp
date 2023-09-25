#ifndef LIBMEM_HPP
#define LIBMEM_HPP

#include "memblock.hpp"
#include "utils/utils.hpp"

#include <cassert>
#include <concepts>
#include <cstdlib>
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
private:
	void* m_space;

	size_t m_total_available;
	size_t m_total_padding;
	size_t m_current_index;

	struct BlockInfo {
		void* addr;

		size_t padding;
		size_t size;
		size_t index;

		bool free;

		BlockInfo(void* addr, size_t padding, size_t size, size_t index,
				  bool free)
			: addr(addr), padding(padding), size(size), index(index),
			  free(free) {}

		BlockInfo(void* addr, size_t padding, size_t size, size_t index)
			: addr(addr), padding(padding), size(size), index(index),
			  free(false) {}
	};

	std::vector<BlockInfo> m_blocks;

	auto m_defragment() -> void {
		// For each block of memory
		// remove the padding until there's
		// enough space to fit whatever
		// is being allocated
		throw std::runtime_error(
			"Call to unimplemented function: m_defragment()");
	}

public:
	Allocator() {
		this->m_space = std::malloc(MAXSIZE);
		if (this->m_space == NULL) {
			throw std::runtime_error("malloc has failed");
		}
		this->m_total_available = MAXSIZE;
		this->m_total_padding = 0;
		this->m_current_index = 0;
	}

	~Allocator() { std::free(this->m_space); }

	/**
	 * @brief Resets the allocator, voiding all previous allocations.
	 * @note Using any `MemBlock`s allocated before a reset is undefined
	 * behaviour after reset is called.
	 */
	auto reset() -> void {
		m_total_available = MAXSIZE;
		m_total_padding = 0;
		m_current_index = 0;

		this->m_blocks.clear();
	}

	/**
	 * @brief Allocates memory and gives back the MemBlock object
	 * @param amount The amount of elements to be allocated of `typesize` each
	 * @param typesize The size of each element to be allocated
	 * @returns MemBlock structure populated with details about the memory
	 */
	template <size_t AMT, typename T>
	[[nodiscard]] auto allocate() -> MemBlock<T> {
		constexpr size_t memsize = AMT * sizeof(T);
		constexpr size_t memsize_padded = Utils::round_pow_two(memsize);
		constexpr size_t padding = memsize_padded - memsize;

		// if the amount being asked for is
		// larger than what is possible, die
		static_assert(!(memsize_padded > MAXSIZE),
					  "Invalid Allocation: The requested size exceeds the "
					  "maximum allowed size.");

		// if the memory is available: allocate
		// else: try and defragment
		//       if defrag fails: throw OOM and die

		if (memsize_padded > this->m_total_available &&
			memsize_padded > this->m_total_padding) {

			throw std::runtime_error("Out of memory");
		} else if (memsize_padded > this->m_total_available &&
				   memsize_padded < this->m_total_padding) {
			// Try to defrag the memory and if
			// that fails, throw what it threw again.
			try {
				this->m_defragment();
			} catch (const std::exception& e) {
				throw std::runtime_error("Failed to defragment the memory: " +
										 std::string(e.what()));
			}
		}

		// If the block wasn't initialised earlier, do it now
		T* block_begin;
		std::pair<bool, size_t> available_block;
		size_t blockidx = this->m_current_index;

		const size_t total_used = MAXSIZE - this->m_total_available;
		available_block = this->available(memsize_padded);
		if (this->m_current_index > 0 && available_block.first) {
			auto blockinfo =
				this->reuse_block(available_block.second, memsize_padded);
			block_begin = static_cast<T*>(blockinfo.first);
			blockidx = blockinfo.second;
		} else {
			block_begin =
				static_cast<T*>(this->m_space) + (total_used / sizeof(T));
		}

		auto block = MemBlock<T>(block_begin, AMT, memsize, blockidx, padding);

		// in the end, increment the class
		// padding and the total
		// memory used
		this->m_total_padding += padding;
		this->m_total_available -= memsize_padded;

		// add it to the allocated blocks
		this->m_blocks.push_back(
			{block_begin, padding, memsize_padded, this->m_current_index});

		this->m_current_index++;

		assert(this->m_current_index == this->m_blocks.size());

		return block;
	}

	template <typename T> auto free(MemBlock<T>& block) -> void {

		// get the block at that index and mark it as free
		const auto blockidx = block.getindex();
		auto& blockref = this->m_blocks.at(blockidx);
		blockref.free = true;

		this->m_total_available += blockref.size;
		this->m_total_padding -= blockref.padding;

		blockref.padding = 0;
	};

	/**
	 * @brief Check if a block with the provided size is available
	 */
	auto available(const size_t size) -> std::pair<bool, size_t> {
		std::pair<bool, size_t> retval(false, 0);

		for (auto& current_block : this->m_blocks) {
			if (current_block.size <= size && current_block.free) {
				retval.first = true;
				retval.second = current_block.index;

				return retval;
			}
		}

		return retval;
	}

	/**
	 * @brief Re-use a previously freed memory block
	 * @returns A pair containing the pointer to the start of the block and it's
	 * index
	 */
	auto reuse_block(const size_t idx, const size_t size)
		-> std::pair<void*, size_t> {
		// get the region from the vector
		BlockInfo& free_block = this->m_blocks[idx];
		std::pair<void*, size_t> retval(nullptr, 0);

		retval.first = free_block.addr;
		retval.second = free_block.index;

		// remove `size` from the free_block at `index` in the array
		// by advancing the pointer by `size` units
		if (free_block.size != size) {
			retval.first = free_block.addr;

			// Advance the pointer, decreaase the size
			// make a new object, push it to the list
			const size_t leftover_size = free_block.size - size;
			BlockInfo leftover(static_cast<size_t*>(free_block.addr) + size, 0,
							   leftover_size, m_current_index, true);
			this->m_blocks.push_back(leftover);

			this->m_current_index++;

			this->m_total_available += leftover_size;
			this->m_total_padding -= free_block.padding;

			return retval;
		}

		// If the size is exactly equal to the free
		// space, then return the pointer and the index

		free_block.free = false;

		if (retval.first == nullptr) {
			throw std::runtime_error("Failed to re-use memory regions");
		}

		return retval;
	}

	template <typename T> auto reallocate(MemBlock<T>& memblock) -> void {
		throw std::runtime_error(
			"Call to unimplemented function: reallocate()");
	};
};
} // namespace LibMem

#endif /* LIBMEM_HPP */
#ifndef LIBMEM_HPP
#define LIBMEM_HPP

#include "memblock.hpp"
#include "utils/utils.hpp"

#include <concepts>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace LibMem {

constexpr size_t SPACESIZE = 32;

class FreedTable {
public:
	FreedTable();
	~FreedTable() = default;

	/**
	 * @brief Add a free region to the table
	 * @param start Address to the start of the region
	 * @param size Size of the region in bytes
	 * @returns void
	 */
	auto add_region(void* start, const size_t size) -> void;

	/**
	 * @brief Remove a free region from the table
	 * @param start Address to the start of the region
	 * @param size Size of the region in bytes
	 * @note If the size passed in is less than
	 * @returns nullptr upon failure and a pointer to the start of the region
	 * upon success
	 */
	[[nodiscard]] auto remove_region(const int index, const size_t size)
		-> void*;

	/**
	 * @brief Check if any available regions fit the current need
	 * @param size Size of the block that to be allocated
	 * @note Time complexity: O(n) where n is the number of elements
	 * currently in the vector
	 * @returns -1 if no regions are available or a the index of the
	 * available region if one is found
	 */
	[[nodiscard]] auto available(const size_t size) -> int;

	// Returns the internal vector
	auto get_available_regions() -> std::vector<std::pair<void*, size_t>>;

private:
	std::vector<std::pair<void*, size_t>> m_freed_regions;

	// Allocator is a friend class
	// so that it can access
	// FreedTable's private members
	friend class Allocator;
};

// This class should ideally
// not do more work than
// it *has* to, so that the
// interface and the code
// remains clean
class Allocator {
private:
	void* m_space;

	size_t m_total_available;
	size_t m_total_padding;
	size_t m_current_index;

	FreedTable m_freed_table;

	auto m_defragment() -> void;

public:
	Allocator();
	~Allocator();

	/**
	 * @brief Resets the allocator, voiding all previous allocations.
	 * @note Using any `MemBlock`s allocated before a reset is undefined
	 * behaviour after reset is called.
	 */
	auto reset() -> void;

	/**
	 * @brief Allocates memory and gives back the MemBlock object
	 * @param amount The amount of elements to be allocated of `typesize` each
	 * @param typesize The size of each element to be allocated
	 * @returns MemBlock structure populated with details about the memory
	 */
	template <size_t AMT, typename T>
	[[nodiscard]] auto allocate() -> MemBlock<T> {
		constexpr size_t memsize = AMT;
		constexpr size_t memsize_padded = Utils::round_pow_two(memsize);
		constexpr size_t padding = memsize_padded - memsize;

		// if the amount being asked for is
		// larger than what is possible, die
		static_assert(!(memsize_padded > SPACESIZE),
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
				throw std::runtime_error(
					"Failed to allocate memory after defragmentation: " +
					std::string(e.what()));
			}
		}

		// If the block wasn't initialised earlier, do it now
		T* block_begin;
		int fr_index = 0;

		const size_t total_used = SPACESIZE - this->m_total_available;
		if (memsize_padded > this->m_total_available) {
			if (this->m_current_index > 0 &&
				(fr_index = this->m_freed_table.available(memsize_padded)) !=
					-1) {
				block_begin = static_cast<T*>(
					m_freed_table.remove_region(fr_index, memsize_padded));
			} else {
				throw std::runtime_error(
					"Out of memory: couldn't allocate new memory");
			}
		} else {
			block_begin = static_cast<T*>(this->m_space) + total_used;
		}

		auto block =
			MemBlock<T>(block_begin, memsize, this->m_current_index++, padding);

		// in the end, increment the class
		// padding and the total
		// memory used
		this->m_total_padding += padding;
		this->m_total_available -= memsize_padded;

		return block;
	}

	template <typename T> auto free(const MemBlock<T>& block) -> void {
		// mark the region as freed
		const size_t total_size = block.m_size + block.m_padding;

		m_freed_table.add_region(block.m_ptr, total_size);
		this->m_total_available += total_size;
		this->m_total_padding -= block.m_padding;
	};

	template <typename T> auto reallocate(MemBlock<T>& memblock) -> void {
		throw std::runtime_error(
			"Call to unimplemented function: reallocate()");
	};

	inline auto getspace() -> void* { return this->m_space; }
};
} // namespace LibMem

#endif /* LIBMEM_HPP */
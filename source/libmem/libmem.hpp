#ifndef LIBMEM_HPP
#define LIBMEM_HPP

#include "utils/utils.hpp"

#include <cstdlib>
#include <stdexcept>

namespace LibMem {

constexpr size_t SPACESIZE = 4096 * 4;

// Each of these is 24 bytes
// if the cache line size is
// 64 bytes, then at least 2
// blocks should be pretty
// fast to access
struct MemBlock {
	MemBlock(void* ptr, size_t size, size_t index, size_t padding);
	~MemBlock();

	/**
	 * @brief Returns a pointer to the *start* of the data in memory. The
	 * pointer may change after a call to allocate/free.
	 * @returns Pointer to the start of the data in memory.
	 */
	auto getptr() -> void*;

	/**
	 * @brief Getter function to get the size of the MemBlock
	 * @returns Size of the MemBlock in bytes
	 */
	auto getsize() -> size_t;

	// Iterator functions
	auto begin() -> void*;
	auto end() -> void*;

private:
	void* m_ptr;

	size_t m_size;
	size_t m_index;
	size_t m_padding;

	// Allocator is a friend class
	// so that it can access
	// MemBlock's private members
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

	auto defragment() -> void;

public:
	Allocator();
	~Allocator();
	// Thought: are templated allocate/reallocate methods really that important?
	// Should I add them?

	/**
	 * @brief Allocates memory and gives back the MemBlock object
	 * @param amount The amount of elements to be allocated of `typesize` each
	 * @param typesize The size of each element to be allocated
	 * @returns MemBlock structure populated with details about the memory
	 */

	template <size_t AMT, size_t TSIZE>
	[[nodiscard]] auto allocate() -> MemBlock {
		constexpr size_t memsize = AMT * TSIZE;
		constexpr size_t memsize_padded = Utils::round_pow_two(memsize);
		constexpr size_t padding = memsize - memsize_padded;

		// if the amount being asked for is
		// larger than what is possible, die
		if constexpr (memsize > SPACESIZE) {
			throw std::runtime_error("Invalid Allocation: The requested "
									 "size exceeds the maximum allowed size.");
		}

		// if the memory is available: allocate
		// else: try and defragment
		//       if defrag fails: throw OOM and die
		if (memsize > this->m_total_available &&
			memsize > this->m_total_padding) {
			throw std::runtime_error("Out of memory");
		} else if (memsize > this->m_total_available &&
				   memsize < this->m_total_padding) {
			// Try to defrag the memory and if
			// that fails, throw what it threw again.
			try {
				this->defragment();
			} catch (const std::exception& e) {
				throw std::runtime_error(
					"Failed to allocate memory after defragmentation: " +
					std::string(e.what()));
			}
		}

		// If all checks pass, construct
		// a MemBlock with the requirements
		size_t total_used = SPACESIZE - this->m_total_available;
		size_t* block_begin =
			static_cast<size_t*>(this->m_space) + total_used + memsize_padded;

		auto block = MemBlock(static_cast<void*>(block_begin), memsize_padded,
							  this->m_current_index++, padding);

		// in the end, increment the class
		// padding and the total
		// memory used
		this->m_total_padding += padding;
		this->m_total_available -= memsize_padded;

		return block;
	}

	[[nodiscard]] auto reallocte(MemBlock& memblock) -> MemBlock;

	auto freemem(MemBlock block) -> void;
	auto setmem() -> void;
};
} // namespace LibMem

#endif /* LIBMEM_HPP */
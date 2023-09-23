#ifndef LIBMEM_HPP
#define LIBMEM_HPP

#include "utils/utils.hpp"

#include <concepts>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace LibMem {

constexpr size_t SPACESIZE = 4096 * 4;

// Each of these is 24 bytes
// if the cache line size is
// 64 bytes, then at least 2
// blocks should be pretty
// fast to access
template <typename T> struct MemBlock {

	MemBlock(T* ptr, size_t size, size_t index, size_t padding)
		: m_ptr(ptr), m_size(size), m_index(index), m_padding(padding) {}

	~MemBlock() {}

	/**
	 * @brief Returns a pointer to the *start* of the data in memory. The
	 * pointer may change after a call to allocate/free.
	 * @returns Pointer to the start of the data in memory.
	 */
	auto getptr() -> T* { return this->m_ptr; }

	/**
	 * @brief Getter function to get the size of the MemBlock
	 * @returns Size of the MemBlock in bytes
	 */
	auto getsize() -> size_t { return this->m_size; }

	// Iterator functions
	auto begin() -> T* { return this->m_ptr; }
	auto end() -> T* { return this->m_ptr + this->m_size; }

	/**
	 * @brief If the underlying type supports array indexing, it will be
	 * available via the MemBlock itself.
	 *
	 * @note Always prefer using integrated MemBlock functions and avoid using
	 * pointers direclty whenever possible.
	 */
	T& operator[](size_t index) {
		static_assert(
			requires { this->m_ptr[0]; },
			"This type does NOT support the index operator");

		if (index >= this->m_size) {
			throw std::out_of_range("Access violation: index out of range.");
		}

		return this->m_ptr[index];
	}

	// Overload the ++ operator (prefix)
	MemBlock<T>& operator++() {
		if (this->m_ptr < this->m_ptr + this->m_size - 1) {
			++this->m_ptr;
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
		return *this;
	}

	// Overload the ++ operator (postfix)
	MemBlock<T> operator++(auto) {
		MemBlock<T> temp(*this);
		if (this->m_ptr < this->m_ptr + this->m_size - 1) {
			++this->m_ptr;
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
		return temp;
	}

	// Overload the + operator
	MemBlock<T> operator+(size_t offset) const {
		if (this->m_ptr + offset < this->m_ptr + this->m_size) {
			return MemBlock<T>(this->m_ptr + offset, this->m_size - offset,
							   this->m_index, this->m_padding);
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
	}

	// Overload the dereference operator
	T& operator*() {
		if (this->m_ptr >= this->m_ptr + this->m_size) {
			throw std::out_of_range("Pointer out of bounds");
		}
		return *this->m_ptr;
	}

	// Overload the arrow operator
	T* operator->() {
		if (this->m_ptr >= this->m_ptr + this->m_size) {
			throw std::out_of_range("Pointer out of bounds");
		}
		return this->m_ptr;
	}

private:
	T* m_ptr;

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

	auto m_defragment() -> void;

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

	template <size_t AMT, typename T>
	[[nodiscard]] auto allocate() -> MemBlock<T> {
		constexpr size_t memsize = AMT;
		constexpr size_t memsize_padded = Utils::round_pow_two(memsize);
		constexpr size_t padding = memsize_padded - memsize;

		// if the amount being asked for is
		// larger than what is possible, die
		static_assert(!(memsize > SPACESIZE),
					  "Invalid Allocation: The requested size exceeds the "
					  "maximum allowed size.");

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
				this->m_defragment();
			} catch (const std::exception& e) {
				throw std::runtime_error(
					"Failed to allocate memory after defragmentation: " +
					std::string(e.what()));
			}
		}

		// If all checks pass, construct
		// a MemBlock with the requirements
		size_t total_used = SPACESIZE - this->m_total_available;
		T* block_begin =
			static_cast<T*>(this->m_space) + total_used + memsize_padded;

		auto block =
			MemBlock<T>(block_begin, memsize, this->m_current_index++, padding);

		// in the end, increment the class
		// padding and the total
		// memory used
		this->m_total_padding += padding;
		this->m_total_available -= memsize_padded;

		return block;
	}

	template <typename T>
	[[nodiscard]] auto reallocte(MemBlock<T>& memblock) -> MemBlock<T>;

	template <typename T> auto freemem(MemBlock<T> block) -> void;
	auto setmem() -> void;
};
} // namespace LibMem

#endif /* LIBMEM_HPP */
#ifndef MEMBLOCK_HPP
#define MEMBLOCK_HPP

#include <concepts>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace LibMem {

template <typename T> class MemBlock {
public:
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
} // namespace LibMem

#endif /* MEMBLOCK_HPP */
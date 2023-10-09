#ifndef MEMBLOCK_HPP
#define MEMBLOCK_HPP

#include <concepts>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "libmem.hpp"

namespace LibMem {
struct BlockInfo {
	void* addr{nullptr};

	size_t padding{0};
	size_t size{0};

	bool free{false};

	BlockInfo(void* addr, size_t padding, size_t size, bool free)
		: addr(addr), padding(padding), size(size), free(free) {}

	// Move Constructor
	BlockInfo(BlockInfo&& source) noexcept
		: addr(std::move(source.addr)), padding(std::move(source.padding)),
		  size(std::move(source.size)), free(std::move(source.free)) {
		source.addr = nullptr;
	}

	BlockInfo& operator=(BlockInfo&& source) {
		// self-asignment check
		if (this != &source) {
			padding = std::move(source.padding);
			size = std::move(source.size);
			free = std::move(source.free);
			addr = std::move(source.addr);

			source.addr = nullptr;
		}

		return *this;
	}

	BlockInfo(const BlockInfo& source) = delete;
	BlockInfo& operator=(const BlockInfo& source) = delete;
};

template <typename T> class MemBlock {
public:
	MemBlock(T* ptr, size_t items, size_t size, size_t padding)
		: m_ptr(ptr), m_items(items), m_size(size), m_padding(padding) {}

	MemBlock(BlockInfo* blockinfo, size_t items)
		: m_ptr(static_cast<T*>(blockinfo->addr)), m_items(items),
		  m_size(blockinfo->size), m_padding(blockinfo->padding) {}

	// Move Constructor
	MemBlock(MemBlock&& source) noexcept
		: m_ptr(std::move(source.m_ptr)),
		  m_padding(std::move(source.m_padding)),
		  m_size(std::move(source.m_size)) {
		source.m_ptr = nullptr;
	}

	MemBlock& operator=(MemBlock&& source) {
		// self-asignment check
		if (this != &source) {
			m_padding = std::move(source.m_padding);
			m_size = std::move(source.m_size);
			m_ptr = std::move(source.m_ptr);

			source.m_ptr = nullptr;
		}

		return *this;
	}

	MemBlock(const MemBlock& source) = delete;
	MemBlock& operator=(const MemBlock& source) = delete;

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

	/**
	 * @brief Getter function to get the number of items of type T stored in the
	 * MemBlock
	 * @returns Number of items of type T stored in the MemBlock
	 */
	auto getamt() -> size_t { return this->m_items; }

	// Iterator functions
	auto begin() -> T* { return this->m_ptr; }
	auto end() -> T* { return this->m_ptr + this->m_items; }

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

		if (index >= this->m_items) {
			throw std::out_of_range("Access violation: index out of range.");
		}

		return this->m_ptr[index];
	}

	// Overload the ++ operator (prefix)
	MemBlock<T>& operator++() {
		if (this->m_ptr < this->m_ptr + this->m_items - 1) {
			++this->m_ptr;
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
		return *this;
	}

	// Overload the ++ operator (postfix)
	MemBlock<T> operator++(auto) {
		MemBlock<T> temp(*this);
		if (this->m_ptr < this->m_ptr + this->m_items - 1) {
			++this->m_ptr;
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
		return temp;
	}

	// Overload the + operator
	MemBlock<T> operator+(size_t offset) const {
		if (this->m_ptr + offset < this->m_ptr + this->m_items) {
			return MemBlock<T>(this->m_ptr + offset, this->m_items - offset,
							   this->m_padding);
		} else {
			throw std::out_of_range("Pointer out of bounds");
		}
	}

	// Overload the dereference operator
	T& operator*() {
		if (this->m_ptr >= this->m_ptr + this->m_items) {
			throw std::out_of_range("Pointer out of bounds");
		}
		return *this->m_ptr;
	}

	// Overload the arrow operator
	T* operator->() {
		if (this->m_ptr >= this->m_ptr + this->m_items) {
			throw std::out_of_range("Pointer out of bounds");
		}
		return this->m_ptr;
	}

private:
	T* m_ptr;

	size_t m_items;
	size_t m_size;
	size_t m_padding;
};
} // namespace LibMem

#endif /* MEMBLOCK_HPP */
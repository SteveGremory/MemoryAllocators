#include "libmem.hpp"

namespace LibMem {

MemBlock::MemBlock(void* ptr, size_t size, size_t index, size_t padding)
	: m_ptr(ptr), m_size(size), m_index(index), m_padding(padding) {}

MemBlock::~MemBlock() {}

auto MemBlock::getptr() -> void* { return this->m_ptr; }
auto MemBlock::getsize() -> size_t { return this->m_size; }
auto MemBlock::begin() -> void* { return static_cast<size_t*>(this->m_ptr); }
auto MemBlock::end() -> void* {
	return static_cast<size_t*>(this->m_ptr) + this->m_size;
}

Allocator::Allocator() {
	m_space = nullptr;
	this->m_space = std::malloc(SPACESIZE);
	this->m_total_available = SPACESIZE;
}

Allocator::~Allocator() { std::free(this->m_space); }

auto Allocator::defragment() -> void {}

} // namespace LibMem

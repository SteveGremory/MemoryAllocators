#include "libmem.hpp"

namespace LibMem {

Allocator::Allocator() {
	m_space = nullptr;
	this->m_space = std::malloc(SPACESIZE);
	this->m_total_available = SPACESIZE;
}

Allocator::~Allocator() { std::free(this->m_space); }

auto Allocator::m_defragment() -> void {}

} // namespace LibMem

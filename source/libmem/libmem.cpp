#include "libmem.hpp"

namespace LibMem {

Allocator::Allocator() {
	m_space = nullptr;
	this->m_space = std::malloc(SPACESIZE);
	if (this->m_space == nullptr || this->m_space == NULL) {
		throw std::runtime_error("malloc has failed");
	}
	this->m_total_available = SPACESIZE;
	m_total_padding = 0;
	m_current_index = 0;
}

Allocator::~Allocator() { std::free(this->m_space); }

auto Allocator::m_defragment() -> void {}

auto Allocator::reset() -> void {
	m_total_available = SPACESIZE;
	m_total_padding = 0;
	m_current_index = 0;
}

} // namespace LibMem

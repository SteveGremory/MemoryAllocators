#include "libmem.hpp"

namespace LibMem {

Allocator::Allocator() {
	this->m_space = std::malloc(SPACESIZE);
	if (this->m_space == NULL) {
		throw std::runtime_error("malloc has failed");
	}
	this->m_total_available = SPACESIZE;
	this->m_total_padding = 0;
	this->m_current_index = 0;
}

Allocator::~Allocator() { std::free(this->m_space); }

auto Allocator::m_defragment() -> void {
	throw std::runtime_error("Call to unimplemented function: m_defragment()");
}

auto Allocator::reset() -> void {
	m_total_available = SPACESIZE;
	m_total_padding = 0;
	m_current_index = 0;

	this->m_freed_table.m_freed_regions.clear();
}

FreedTable::FreedTable() { this->m_freed_regions.reserve(32); }

auto FreedTable::add_region(void* start, const size_t size) -> void {
	m_freed_regions.emplace_back(start, size);
}

[[nodiscard]] auto FreedTable::remove_region(const int index, const size_t size)
	-> void* {
	// get the region from the vector
	auto& region = this->m_freed_regions[index];
	void* retval = nullptr;

	// remove `size` from the region at `index` in the array
	// by advancing the pointer by `size` units
	if (region.second != size) {
		retval = region.first;

		// Advance the pointer, decreaase the size
		region.first = static_cast<size_t*>(region.first) + size;
		region.second -= size;

		return retval;
	}

	// If the size is exactly equal to the free
	// space, then remove the element from the
	// vector and return the pointer
	retval = region.first;
	this->m_freed_regions.erase(this->m_freed_regions.begin() + index);

	if (retval == nullptr) {
		throw std::runtime_error("Failed to re-use memory regions");
	}

	return retval;
}

auto FreedTable::available(const size_t size) -> int {
	int index = 0;
	for (const auto& region : m_freed_regions) {
		if (region.second >= size) {
			return index;
		}
		index++;
	}
	return -1;
}

// Returns the internal vector
auto FreedTable::get_available_regions()
	-> std::vector<std::pair<void*, size_t>> {
	return m_freed_regions;
}

} // namespace LibMem

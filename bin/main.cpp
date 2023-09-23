#include <iostream>
#include <libmem/libmem.hpp>

using namespace LibMem;

int main() {
	auto allocator = Allocator();
	constexpr size_t size = 10;
	constexpr size_t sizeofint = sizeof(int);
	auto intblock = allocator.allocate<size, sizeofint>();

	int* heapintarr = static_cast<int*>(intblock.getptr());

	for (int i = 0; i < 10; i++) {
		heapintarr[i] = i + 1;
	}

	for (int i = 0; i < 10; i++) {
		std::cout << heapintarr[i] << std::endl;
	}

	return 0;
}

#include <iostream>
#include <libmem/libmem.hpp>

using namespace LibMem;

int main() {
	auto allocator = Allocator();

	auto intblock = allocator.allocate<10, int>();
	for (size_t i = 0; i < 10; i++) {
		intblock[i] = i + 1;
	}
	allocator.free(intblock);

	auto intblockreturns = allocator.allocate<10, int>();
	for (size_t i = 0; i < 10; i++) {
		intblockreturns[i] = i + 100;
	}

	for (size_t i = 0; i < 10; i++) {
		std::cout << intblockreturns[i] << std::endl;
	}
	return 0;
}

#include <iostream>
#include <libmem/libmem.hpp>

using namespace LibMem;

int main() {
	auto allocator = Allocator();

	auto intblock = allocator.allocate<10, int>();
	for (size_t i = 0; i < 10; i++) {
		intblock[i] = i + 1;
	}

	auto floatblock = allocator.allocate<10, float>();
	for (size_t i = 0; i < 10; i++) {
		floatblock[i] = i + 0.123;
	}

	for (size_t i = 0; i < 10; i++) {
		std::cout << "Int Array: " << intblock[i] << std::endl
				  << "Float Array: " << floatblock[i] << std::endl
				  << std::endl;
	}

	auto& x = *(intblock + 2);
	x = 123;

	std::cout << *(intblock + 2) << std::endl;

	return 0;
}

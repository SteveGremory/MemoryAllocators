#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <libmem/libmem.hpp>
#include <stdexcept>

// A benchmark for checking the defragmentation algorithm
static void DefragmentBenchmark(benchmark::State& state) {
	auto allocator = LibMem::Allocator<18 * sizeof(int)>();
	auto space = static_cast<int*>(allocator.getspace());

	for (auto _ : state) {
		auto intblock = allocator.allocate<10, int>();
		for (int i = 0; i < intblock.getamt(); i++) {
			intblock[i] = i;
		}

		// std::cout << "SPACE BEFORE:\t";
		// for (int i = 0; i < 18; i++) {
		//	std::cout << space[i] << '\t';
		// }
		// std::cout << std::endl;

		auto intblock2 = allocator.allocate<8, int>();
		for (int i = 0; i < intblock2.getamt(); i++) {
			intblock2[i] = i + 10;
		}

		// std::cout << "SPACE AFTER:\t";
		// for (int i = 0; i < 18; i++) {
		//	std::cout << space[i] << '\t';
		// }
		// std::cout << std::endl;
		// std::cout << std::endl;

		// Reset the allocator for the next run
		allocator.reset();
	}
}

BENCHMARK(DefragmentBenchmark)->Range(0, 1000);
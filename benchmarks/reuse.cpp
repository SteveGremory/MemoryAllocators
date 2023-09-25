#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <libmem/libmem.hpp>
#include <stdexcept>

// A benchmark for checking the defragmentation algorithm
static void ReuseBenchmark(benchmark::State& state) {
	auto allocator = LibMem::Allocator<100000 * sizeof(int)>();

	// 16 ints will be allocated
	auto intblock = allocator.allocate<10000, int>();
	allocator.free(intblock);

	for (auto _ : state) {

		auto intblock2 = allocator.allocate<10000, int>();

		for (int i = 0; i < intblock2.getamt(); i++) {
			intblock2[i] = i + 10;
		}

		// for (int i = 0; i < intblock2.getamt(); i++) {
		//	std::cout << intblock2[i] << '\t';
		// }
		// std::cout << std::endl;

		allocator.free(intblock);
	}

	// Reset the allocator for the next run
	allocator.reset();
}

// BENCHMARK(ReuseBenchmark)->Range(0, 1000);
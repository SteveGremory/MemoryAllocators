#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <libmem/libmem.hpp>
#include <stdexcept>

// A benchmark for checking the defragmentation algorithm
static void MemcpyBenchmark(benchmark::State& state) {
	auto allocator = LibMem::Allocator();

	for (auto _ : state) {

		// 16 bytes will be allocated
		auto intblock = allocator.allocate<10, int>();
		// allocate 16 more, then get the total padding and
		auto intblock2 = allocator.allocate<10, int>();

		// assign stuff to them
		for (int i = 0; i < 10; i++) {
			intblock[i] = i + 1;
			intblock2[i] = i + 1 + 10;
		}

		// move the 2nd part `padding` bytes behind in the memory, this
		// means that (6 + 6) = 12 bytes of space will be freed.
		memmove(intblock.getptr() + intblock.getamt(), intblock2.getptr(),
				intblock2.getsize());

		// Reset the allocator for the next run
		allocator.reset();
		benchmark::ClobberMemory();
	}
}

BENCHMARK(MemcpyBenchmark)->Range(0, 1000);
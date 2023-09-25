#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <libmem/libmem.hpp>
#include <stdexcept>

static void MallocBenchmark(benchmark::State& state) {

	for (auto _ : state) {
		int* intblock;

		benchmark::DoNotOptimize(intblock);

		intblock = static_cast<int*>(malloc(16 * sizeof(int)));

		for (int i = 0; i < 16; i++) {
			intblock[i] = i + 10;
		}

		free(intblock);

		benchmark::ClobberMemory();
	}
}

static void LibMemBenchmark(benchmark::State& state) {
	auto allocator = LibMem::Allocator<16 * sizeof(int)>();

	for (auto _ : state) {
		auto intblock = allocator.allocate<16, int>();

		for (int i = 0; i < intblock.getamt(); i++) {
			intblock[i] = i + 10;
		}

		allocator.free(intblock);

		benchmark::ClobberMemory();
	}
}

BENCHMARK(LibMemBenchmark)->Range(0, 1000);
// BENCHMARK(MallocBenchmark)->Range(0, 1000);
#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <libmem/libmem.hpp>
#include <stdexcept>

constexpr size_t ALLOCSIZE = 8192;

static void MallocBenchmark(benchmark::State& state) {

	for (auto _ : state) {
		int* intblock = static_cast<int*>(malloc(ALLOCSIZE * sizeof(int)));
		benchmark::DoNotOptimize(intblock);
		free(intblock);
		benchmark::ClobberMemory();
	}
}

static void LibMemBenchmark(benchmark::State& state) {
	for (auto _ : state) {
		auto allocator = LibMem::Allocator();
		auto intblock = allocator.allocate<10, int>();

		benchmark::ClobberMemory();
	}
}

BENCHMARK(LibMemBenchmark);
BENCHMARK(MallocBenchmark);
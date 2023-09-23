#include <benchmark/benchmark.h>
#include <chrono>
#include <cstdlib>
#include <libmem/libmem.hpp>
#include <stdexcept>

constexpr size_t ALLOCSIZE = 8192;

static void MallocBenchmark(benchmark::State& state) {

	for (auto _ : state) {
		int* intblock;
		int* intblock2;

		benchmark::DoNotOptimize(intblock);
		benchmark::DoNotOptimize(intblock2);

		intblock = static_cast<int*>(malloc(1024 * sizeof(int)));
		intblock2 = static_cast<int*>(malloc(1024 * sizeof(int)));

		free(intblock);
		free(intblock2);

		benchmark::ClobberMemory();
	}
}

static void LibMemBenchmark(benchmark::State& state) {
	auto allocator = LibMem::Allocator();

	for (auto _ : state) {
		auto intblock = allocator.allocate<1024, int>();
		auto intblock2 = allocator.allocate<1024, int>();

		allocator.free(intblock);
		allocator.free(intblock2);

		benchmark::ClobberMemory();
	}
}

BENCHMARK(LibMemBenchmark)->Range(0, 1000);
BENCHMARK(MallocBenchmark)->Range(0, 1000);
#include <benchmark/benchmark.h>

#include <iostream>

void BM_SomeFunction(benchmark::State& state) {
	// Perform setup here
	int x = 0;
	for (auto _ : state) {
		// This code gets timed
		x++;
	}

	std::cout << x << std::endl;
}

// Register the function as a benchmark
BENCHMARK(BM_SomeFunction);
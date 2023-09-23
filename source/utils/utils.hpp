#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>

namespace Utils {

/**
 * @brief Rounds off `num` to the nearest power of two
 * @param num The number to round off
 * @returns Closest power of 2 to num
 */
inline consteval size_t round_pow_two(size_t num) {
	if (num <= 1) {
		return 1;
	}

	num--;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	num |= num >> 8;
	num |= num >> 16;

	return num + 1;
};
} // namespace Utils

#endif /* UTILS_HPP */
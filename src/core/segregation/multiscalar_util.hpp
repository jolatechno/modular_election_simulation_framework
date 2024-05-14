#pragma once

#include "../../util/math.hpp"

#include "multiscalar.hpp"


namespace segregation::multiscalar::util {
	template<typename Type>
	std::vector<std::vector<Type>> get_accumulated_trajectory(const std::vector<std::vector<std::vector<Type>>> &vects) {
		std::vector<std::vector<Type>> accumulated_trajectory(vects[0].size(),  std::vector<Type>(vects[0][0].size()));

		for (size_t i = 0; i < vects[0].size(); ++i) {
			accumulated_trajectory[i][0] = 0;

			for (size_t j = 0; j < vects[0][0].size(); ++j) {
				if (j > 0) {
					accumulated_trajectory[i][j] = accumulated_trajectory[i][j-1];
				}

				for (size_t k = 0; k < vects.size(); ++k) {
					accumulated_trajectory[i][j] += vects[k][i][j];
				}
			}
		}

		return accumulated_trajectory;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_accumulated_trajectory_single(const std::vector<std::vector<Type>> &vect) {
		return get_accumulated_trajectory(std::vector<std::vector<std::vector<Type>>>({vect}));
	}
}
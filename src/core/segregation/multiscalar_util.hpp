#pragma once

#include "../../util/math.hpp"

#include "multiscalar.hpp"


namespace segregation::multiscalar::util {
	template<typename Type>
	std::vector<Type> get_total_distribution(const std::vector<std::vector<Type>> &vects) {
		std::vector<Type> total_distribution(vects.size());

		Type total = 0;
		#pragma omp parallel for
		for (size_t k = 0; k < vects.size(); ++k) {
			total_distribution[k] = std::accumulate(vects[k].begin(), vects[k].end(), 0);
			#pragma omp critical
			total                += total_distribution[k];
		}
		for (size_t k = 0; k < vects.size(); ++k) {
			total_distribution[k] /= total;
		}

		return total_distribution;
	}

	template<typename Type>
	std::vector<Type> get_total_population(const std::vector<std::vector<Type>> &vects) {
		std::vector<Type> total_population(vects[0].size(), 0);

		Type total = 0;
		#pragma omp parallel for
		for (size_t i = 0; i < vects[0].size(); ++i) {
			for (size_t k = 0; k < vects.size(); ++k) {
				total_population[i] += vects[k][i];
			}
		}

		return total_population;
	}
	
	template<typename Type>
	std::vector<std::vector<Type>> get_accumulated_trajectory(const std::vector<std::vector<Type>> &vects, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<Type>> accumulated_trajectory(indexes.size(),  std::vector<Type>(indexes[0].size()));

		#pragma omp parallel for
		for (size_t i = 0; i < indexes.size(); ++i) {
			accumulated_trajectory[i][0] = 0;

			for (size_t j = 0; j < indexes[0].size(); ++j) {
				if (j > 0) {
					accumulated_trajectory[i][j] = accumulated_trajectory[i][j-1];
				}

				for (size_t k = 0; k < vects.size(); ++k) {
					accumulated_trajectory[i][j] += vects[k][indexes[i][j]];
				}
			}
		}

		return accumulated_trajectory;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_accumulated_trajectory_single(const std::vector<Type> &vect) {
		return get_accumulated_trajectory(std::vector<std::vector<Type>>({vect}));
	}
}
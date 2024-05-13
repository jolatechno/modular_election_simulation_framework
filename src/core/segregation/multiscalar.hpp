#pragma once

#include "../../util/math.hpp"


namespace segregation::multiscalar {
	template<typename Type>
	std::vector<std::vector<Type>> get_distances(const std::vector<Type> &lat, const std::vector<Type> &lon) {
		std::vector<std::vector<Type>> distances(lat.size(), std::vector<Type>(lat.size()));

		/* TODO */

		return distances;
	}

	template<typename Type>
	std::vector<std::vector<size_t>> get_closest_neighbors(const std::vector<std::vector<Type>> &distances) {
		std::vector<std::vector<size_t>> indexes(distances.size(), std::vector<Type>(distances.size()));

		/* TODO */

		return indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_trajectory(const std::vector<Type> &vect, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<Type>> trajectory(indexes.size(), std::vector<Type>(indexes.size()));

		/* TODO */

		return trajectory;
	}

	template<typename Type>
	std::vector<std::vector<std::vector<Type>>> get_trajectories(const std::vector<std::vector<Type>> &vects, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<std::vector<Type>>> trajectories;

		for (const auto &vect : vects) {
			trajectories.push_back(get_trajectory(vect, indexes));
		}
		
		return trajectories;
	}
}
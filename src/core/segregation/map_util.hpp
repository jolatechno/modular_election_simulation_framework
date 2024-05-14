#pragma once

#include "../../util/math.hpp"
#include "../../util/util.hpp"


namespace segregation::map::util {
	template<typename Type>
	std::vector<Type> get_distances_single(const std::vector<Type> &lat, const std::vector<Type> &lon, size_t idx) {
		std::vector<Type> distances(lat.size());

		for (size_t j = 0; j < lat.size(); ++j) {
			Type delta_lat = lat[idx] - lat[j];
			Type delta_lon = lon[idx] - lon[j];

			distances[j] = delta_lat*delta_lat + delta_lon*delta_lon;
		}

		return distances;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_distances(const std::vector<Type> &lat, const std::vector<Type> &lon, const std::vector<size_t> &indexes) {
		std::vector<std::vector<Type>> distances(indexes.size(),  std::vector<Type>(lat.size()));

		for (size_t i = 0; i < indexes.size(); ++i) {
			distances[i] = get_distances_single(lat, lon, indexes[i]);
		}

		return distances;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_distances(const std::vector<Type> &lat, const std::vector<Type> &lon) {
		std::vector<std::vector<Type>> distances(lat.size(),  std::vector<Type>(lat.size()));

		for (size_t i = 0; i < lat.size(); ++i) {
			for (size_t j = 0; j < i; ++j) {
				Type delta_lat = lat[i] - lat[j];
				Type delta_lon = lon[i] - lon[j];

				distances[i][j] = delta_lat*delta_lat + delta_lon*delta_lon;
				distances[j][i] = distances[i][j];
			}
		}

		return distances;
	}
}
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
	std::vector<std::vector<Type>> get_trajectories(const std::vector<Type> &vect, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<Type>> trajectory(indexes.size(), std::vector<Type>(indexes.size()));

		for (size_t i = 0; indexes.size(); ++i) {
			for (size_t j = 0; j < indexes[i].size(); ++j) {
				trajectory[i][j] = vect[indexes[i][j]];
			}
		}

		return trajectory;
	}

	template<typename Type>
	std::vector<std::vector<std::vector<Type>>> get_trajectories(const std::vector<std::vector<Type>> &vects, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<std::vector<Type>>> trajectories;

		for (const auto &vect : vects) {
			trajectories.push_back(get_trajectories(vect, indexes));
		}
		
		return trajectories;
	}

	template<typename Type>
	std::vector<std::vector<double>> get_KLdiv_trajectories(const std::vector<std::vector<std::vector<Type>>> &trajectories) {
		std::vector<std::vector<double>> KLdiv_trajectories(trajectories[0].size(), std::vector<double>(trajectories[0].size()));

		/* TODO */
		
		return KLdiv_trajectories;
	}

	std::vector<std::vector<size_t>> get_focal_distance_indexes(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_coefficients) {
		std::vector<std::vector<size_t>> focal_distance_indexes(KLdiv_trajectories.size(), std::vector<size_t>(convergence_coefficients.size()));

		/* TODO */
		
		return focal_distance_indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_focal_distances(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_coefficients, const std::vector<Type> &Xvalues) {
		std::vector<std::vector<size_t>> focal_distance_indexes = get_focal_distance_indexes(KLdiv_trajectories, convergence_coefficients);

		std::vector<std::vector<Type>> focal_distances(KLdiv_trajectories.size(), std::vector<double>(convergence_coefficients.size()));
		for (size_t i = 0; KLdiv_trajectories.size(); ++i) {
			for (size_t j = 0; j < KLdiv_trajectories[i].size(); ++j) {
				focal_distances[i][j] = Xvalues[focal_distance_indexes[i][j]];
			}
		}
		
		return focal_distances;
	}

	template<typename Type>
	std::vector<Type> get_distortion_coefs(const std::vector<std::vector<Type>> &focal_distances, const std::vector<Type> &Xvalues, const Type &normalization_coef=1.0) {
		std::vector<Type> distortion_coefs = integrals(focal_distances, Xvalues);

		for (Type &distortion_coef : distortion_coefs) {
			distortion_coef *= normalization_coef;
		}
		
		return distortion_coefs;
	}
}
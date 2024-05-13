#pragma once

#include "../../util/math.hpp"


namespace segregation::multiscalar {
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

	template<typename Type>
	std::vector<std::vector<size_t>> get_closest_neighbors(const std::vector<std::vector<Type>> &distances) {
		std::vector<std::vector<size_t>> indexes(distances.size());

		for (size_t i = 0; i < indexes.size(); ++i) {
			indexes[i] = util::math::get_sorted_indexes(distances[i]);
		}

		return indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_trajectories_single(const std::vector<Type> &vect, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<Type>> trajectory(indexes.size(), std::vector<Type>(indexes.size()));

		for (size_t i = 0; i < indexes.size(); ++i) {
			Type total = 0.d;
			for (size_t j = 0; j < indexes[i].size(); ++j) {
				trajectory[i][j] = trajectory[i][j] + vect[indexes[i][j]];
			}
		}

		return trajectory;
	}

	template<typename Type>
	std::vector<std::vector<std::vector<Type>>> get_trajectories(const std::vector<std::vector<Type>> &vects, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<std::vector<Type>>> trajectories(vects.size(),
			std::vector<std::vector<Type>>(indexes.size(), std::vector<Type>(indexes[0].size())));

		for (size_t i = 0; i < indexes.size(); ++i) {
			Type total = 0.d;
			std::vector<Type> running_sum(vects.size(), 0);

			for (size_t j = 0; j < indexes[i].size(); ++j) {
				for (size_t k = 0; k < vects.size(); ++k) {
					running_sum[k] += vects[k][indexes[i][j]];
					total          += vects[k][indexes[i][j]];
				}
				for (size_t k = 0; k < vects.size(); ++k) {
					trajectories[k][i][j] = running_sum[k]/total;
				}
			}
		}
		
		return trajectories;
	}

	template<typename Type>
	std::vector<std::vector<double>> get_KLdiv_trajectories(const std::vector<std::vector<std::vector<Type>>> &trajectories) {
		std::vector<std::vector<double>> KLdiv_trajectories(trajectories[0].size(), std::vector<double>(trajectories[0].size()));

		std::vector<Type> total_distribution(trajectories.size());
		for (size_t k = 0; k < trajectories.size(); ++k) {
			total_distribution[k] = trajectories[k][0].back();
		}

		std::vector<Type> placeholder(trajectories.size());
		for (size_t i = 0; i < trajectories[0].size(); ++i) {
			for (size_t j = 0; j < trajectories[0].size(); ++j) {
				for (size_t k = 0; k < trajectories.size(); ++k) {
					placeholder[k] = trajectories[k][i][j];
				}
				KLdiv_trajectories[i][j] = util::math::get_KLdiv(placeholder, total_distribution);
			}
		}
		
		return KLdiv_trajectories;
	}

	template<typename Type>
	std::vector<std::vector<double>> get_KLdiv_trajectories_single(const std::vector<std::vector<Type>> &trajectory) {
		std::vector<std::vector<double>> KLdiv_trajectories(trajectory.size(), std::vector<double>(trajectory.size()));

		double total_distribution = trajectory[0].back();
		for (size_t i = 0; i < trajectory.size(); ++i) {
			for (size_t j = 0; j < trajectory.size(); ++j) {
				KLdiv_trajectories[i][j] = util::math::get_KLdiv_single(trajectory[i][j], total_distribution);
			}
		}
		
		return KLdiv_trajectories;
	}

	std::vector<std::vector<size_t>> get_focal_distance_indexes(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_coefficients) {
		std::vector<std::vector<size_t>> focal_distance_indexes(KLdiv_trajectories.size(), std::vector<size_t>(convergence_coefficients.size()));

		for (size_t i = 0; i < KLdiv_trajectories.size(); ++i) {
			size_t idx = KLdiv_trajectories.size()-1;
			for (size_t j = 0; j < convergence_coefficients.size(); ++j) {
				while (idx > 0 && KLdiv_trajectories[i][idx] < convergence_coefficients[j]) {
					--idx;
				}

				focal_distance_indexes[i][j] = idx;
			}
		}
		
		return focal_distance_indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_focal_distances(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_coefficients, const std::vector<Type> &Xvalues) {
		std::vector<std::vector<size_t>> focal_distance_indexes = get_focal_distance_indexes(KLdiv_trajectories, convergence_coefficients);

		std::vector<std::vector<Type>> focal_distances(KLdiv_trajectories.size(), std::vector<double>(convergence_coefficients.size()));
		for (size_t i = 0; i < KLdiv_trajectories.size(); ++i) {
			for (size_t j = 0; j < KLdiv_trajectories[i].size(); ++j) {
				focal_distances[i][j] = Xvalues[focal_distance_indexes[i][j]];
			}
		}
		
		return focal_distances;
	}

	template<typename Type1, typename Type2>
	std::vector<Type2> get_distortion_coefs(const std::vector<std::vector<Type1>> &focal_distances, const std::vector<Type2> &Xvalues, const Type1 &normalization_coef=1.0) {
		std::vector<Type2> distortion_coefs = util::math::integrals(focal_distances, Xvalues);

		for (Type2 &distortion_coef : distortion_coefs) {
			distortion_coef *= normalization_coef;
		}
		
		return distortion_coefs;
	}
}
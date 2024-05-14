#pragma once

#include "../../util/math.hpp"
#include "../../util/util.hpp"

#include <functional>


namespace segregation::multiscalar {
	template<typename Type>
	std::vector<std::vector<size_t>> get_closest_neighbors(const std::vector<std::vector<Type>> &distances) {
		std::vector<std::vector<size_t>> indexes(distances.size());

		for (size_t i = 0; i < distances.size(); ++i) {
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
		std::vector<std::vector<double>> KLdiv_trajectories(trajectories[0].size(), std::vector<double>(trajectories[0][0].size()));

		std::vector<Type> total_distribution(trajectories.size());
		for (size_t k = 0; k < trajectories.size(); ++k) {
			total_distribution[k] = trajectories[k][0].back();
		}

		std::vector<Type> placeholder(trajectories.size());
		for (size_t i = 0; i < trajectories[0].size(); ++i) {
			for (size_t j = 0; j < trajectories[0][0].size(); ++j) {
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
			for (size_t j = 0; j < trajectory[i].size(); ++j) {
				KLdiv_trajectories[i][j] = util::math::get_KLdiv_single(trajectory[i][j], total_distribution);
			}
		}
		
		return KLdiv_trajectories;
	}

	std::vector<std::vector<size_t>> get_focal_distance_indexes(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_thresholds) {
		std::vector<std::vector<size_t>> focal_distance_indexes(KLdiv_trajectories.size(), std::vector<size_t>(convergence_thresholds.size()));

		for (size_t i = 0; i < KLdiv_trajectories.size(); ++i) {
			size_t idx = KLdiv_trajectories[0].size()-1;
			for (size_t j = 0; j < convergence_thresholds.size(); ++j) {
				while (idx > 0 && KLdiv_trajectories[i][idx] < convergence_thresholds[j]) {
					--idx;
				}

				focal_distance_indexes[i][j] = idx;
			}
		}
		
		return focal_distance_indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_focal_distances(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_thresholds, const std::vector<std::vector<Type>> &Xvalues) {
		std::vector<std::vector<Type>> focal_distances(KLdiv_trajectories.size(), std::vector<Type>(convergence_thresholds.size()));

		for (size_t i = 0; i < KLdiv_trajectories.size(); ++i) {
			size_t idx = KLdiv_trajectories[0].size()-1;
			for (size_t j = 0; j < convergence_thresholds.size(); ++j) {
				while (idx > 0 && KLdiv_trajectories[i][idx] < convergence_thresholds[j]) {
					--idx;
				}

				focal_distances[i][j] = Xvalues[i][idx];
			}
		}
		
		return focal_distances;
	}

	template<typename Type1, typename Type2=double>
	std::vector<Type2> get_distortion_coefs(const std::vector<std::vector<Type1>> &focal_distances, const std::vector<double> &convergence_thresholds, const Type2 &normalization_coef=1.d) {
		std::vector<Type2> distortion_coefs = util::math::integrals(focal_distances, convergence_thresholds);

		for (Type2 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}

	template<typename Type, typename Type2=double>
	Type get_normalization_factor(const std::vector<std::vector<Type>> &vects, const std::vector<double> &convergence_thresholds, const std::vector<std::vector<Type2>> &Xvalues={}) {
		std::vector<Type> total_distribution(vects.size());

		Type total = 0;
		for (size_t k = 0; k < vects.size(); ++k) {
			total_distribution[k] = std::accumulate(vects[k].begin(), vects[k].end(), 0);
			total                += total_distribution[k];
		}
		for (size_t k = 0; k < vects.size(); ++k) {
			total_distribution[k] /= total;
		}
		std::sort(total_distribution.begin(), total_distribution.end());


		std::vector<std::vector<size_t>> indexes(1, std::vector<size_t>(vects[0].size()));
		std::iota(indexes[0].begin(), indexes[0].end(), 0);


		std::vector<std::vector<Type>> vects_(vects.size(), std::vector<Type>(vects[0].size(), 0));


		size_t limit_idx = 0;
		for (size_t i = 0; i < vects.size(); ++i) {
			size_t previous_limit_idx = limit_idx;
			limit_idx = std::min(vects[0].size(), (size_t)(limit_idx + total_distribution[i]*vects[0].size()));
			for (size_t j = previous_limit_idx; j < limit_idx; ++j) {
				vects_[i][j] = 1;
			}
		}

		auto trajectories_ = segregation::multiscalar::get_trajectories(vects_, indexes);
		auto KLdiv_trajectories = segregation::multiscalar::get_KLdiv_trajectories(trajectories_);

		if (Xvalues.empty()) {
			auto focal_distance_indexes = segregation::multiscalar::get_focal_distance_indexes(KLdiv_trajectories, convergence_thresholds);
			auto distortion_coefs = segregation::multiscalar::get_distortion_coefs(focal_distance_indexes, convergence_thresholds);
			return distortion_coefs[0];
		} else {
			auto focal_distances = segregation::multiscalar::get_focal_distances(KLdiv_trajectories, convergence_thresholds, Xvalues);

			Type max_normalization_factor = 0;
			for (int i = 0; i < Xvalues.size(); ++i) {
				std::vector<std::vector<Type2>> Xvalues_slice(Xvalues.begin()+i, Xvalues.begin()+i+1);

				auto focal_distances_slice = segregation::multiscalar::get_focal_distances(KLdiv_trajectories, convergence_thresholds, Xvalues_slice);
				auto distortion_coefs      = segregation::multiscalar::get_distortion_coefs(focal_distances_slice, convergence_thresholds);

				max_normalization_factor = std::max(max_normalization_factor, distortion_coefs[0]);
			}

			return max_normalization_factor;
		}
	}

	template<typename Type1, typename Type2=double>
	std::vector<Type1> get_normalized_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects, const std::vector<std::vector<size_t>> &indexes,
		const std::vector<double> &convergence_thresholds, const std::vector<std::vector<Type2>> &Xvalues={})
	{
		std::vector<Type1> distortion_coefs(indexes.size());

		for (size_t i = 0; i < indexes.size(); ++i) {
			std::vector<std::vector<size_t>> indexes_slice(indexes.begin()+i, indexes.begin()+i+1);

			auto this_trajectories_      = segregation::multiscalar::get_trajectories(vects, indexes_slice);
			auto this_KLdiv_trajectories = segregation::multiscalar::get_KLdiv_trajectories(this_trajectories_);

			if (Xvalues.size() < i || Xvalues[i].empty()) {
				auto this_focal_distance_indexes = segregation::multiscalar::get_focal_distance_indexes(this_KLdiv_trajectories, convergence_thresholds);
				auto this_distortion_coefs       = segregation::multiscalar::get_distortion_coefs(this_focal_distance_indexes, convergence_thresholds);
				distortion_coefs[i]              = this_distortion_coefs[0];
			} else {
				std::vector<std::vector<Type2>> Xvalues_slice(Xvalues.begin()+i, Xvalues.begin()+i+1);

				auto this_focal_distances  = segregation::multiscalar::get_focal_distances(this_KLdiv_trajectories, convergence_thresholds, Xvalues_slice);
				auto this_distortion_coefs = segregation::multiscalar::get_distortion_coefs(this_focal_distances, convergence_thresholds);
				distortion_coefs[i]        = this_distortion_coefs[0];
			}
		}

		double normalization_coef = get_normalization_factor(vects, convergence_thresholds, Xvalues);
		for (Type1 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}

	template<typename Type1, typename Type2=double>
	std::vector<Type1> get_normalized_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects, const std::vector<double> &convergence_thresholds,
		const std::function<std::pair<std::vector<size_t>, std::vector<Type2>>(size_t)> func)
	{
		std::vector<Type1> distortion_coefs(vects[0].size());
		std::vector<std::vector<Type2>>  Xvalues_slice(1);
		std::vector<std::vector<size_t>> indexes_slice(1);

		double normalization_coef = 0.d;
		for (size_t i = 0; i < vects[0].size(); ++i) {
			{
				auto [indexes, Xvalues] = func(i);
				indexes_slice[0] = indexes;
				Xvalues_slice[0] = Xvalues;
			}

			auto this_trajectories_      = segregation::multiscalar::get_trajectories(vects, indexes_slice);
			auto this_KLdiv_trajectories = segregation::multiscalar::get_KLdiv_trajectories(this_trajectories_);

			if (Xvalues_slice[0].empty()) {
				normalization_coef = std::max(normalization_coef, get_normalization_factor(vects, convergence_thresholds));

				auto this_focal_distance_indexes = segregation::multiscalar::get_focal_distance_indexes(this_KLdiv_trajectories, convergence_thresholds);
				auto this_distortion_coefs       = segregation::multiscalar::get_distortion_coefs(this_focal_distance_indexes, convergence_thresholds);
				distortion_coefs[i]              = this_distortion_coefs[0];
			} else {
				normalization_coef = std::max(normalization_coef, get_normalization_factor(vects, convergence_thresholds, Xvalues_slice));

				auto this_focal_distances  = segregation::multiscalar::get_focal_distances(this_KLdiv_trajectories, convergence_thresholds, Xvalues_slice);
				auto this_distortion_coefs = segregation::multiscalar::get_distortion_coefs(this_focal_distances, convergence_thresholds);
				distortion_coefs[i]        = this_distortion_coefs[0];
			}
		}

		for (Type1 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}
}
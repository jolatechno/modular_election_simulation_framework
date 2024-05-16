#pragma once

#include "../../util/math.hpp"
#include "../../util/util.hpp"

#include "multiscalar_util.hpp"

#include <functional>


namespace segregation::multiscalar {
	template<typename Type>
	std::vector<std::vector<size_t>> get_closest_neighbors(const std::vector<std::vector<Type>> &distances) {
		std::vector<std::vector<size_t>> indexes(distances.size());

		for (size_t i = 0; i < distances.size(); ++i) {
			indexes[i] = ::util::math::get_sorted_indexes(distances[i]);
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
				KLdiv_trajectories[i][j] = ::util::math::get_KLdiv(placeholder, total_distribution);
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
				KLdiv_trajectories[i][j] = ::util::math::get_KLdiv_single(trajectory[i][j], total_distribution);
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
		std::vector<Type2> distortion_coefs = ::util::math::integrals(focal_distances, convergence_thresholds);

		for (Type2 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}


	template<typename Type, typename Type2=double>
	Type get_normalization_factor(const std::vector<std::vector<Type>> &vects, const std::vector<std::vector<Type2>> &Xvalues={}) {
		return get_normalization_factor(util::get_total_distribution(vects, Xvalues, vects[0].size()));
	}

	template<typename Type, typename Type2=double>
	Type get_normalization_factor(const std::vector<Type> &total_distribution, const std::vector<std::vector<Type2>> &Xvalues={}, size_t N_steps=1000000) {
		if (!Xvalues.empty()) {
			N_steps = Xvalues[0].size();
		}

		std::vector<Type> total_distribution_copy = total_distribution;
		std::sort(total_distribution_copy.begin(), total_distribution_copy.end());

		auto distortion_factor = [&](const std::vector<Type2> &X_values_slice) {
			std::vector<Type> acc( total_distribution.size(), 0);
			std::vector<Type> traj(total_distribution.size());

			Type normalization_factor = 0, old_diveregnce = 0;

			size_t limit_idx = 0;
			for (size_t i = 0; i < total_distribution.size(); ++i) {
				size_t previous_limit_idx = limit_idx;
				limit_idx = std::min(N_steps-1, (size_t)(limit_idx + total_distribution_copy[i]*N_steps));

				for (size_t j = previous_limit_idx; j < limit_idx; ++j) {
					acc[i] += 1;

					for (int k = 0; k < total_distribution.size(); ++k) {
						traj[k] = acc[k]/(j+1);
					}

					Type KL_div = ::util::math::get_KLdiv(traj, total_distribution_copy);
					if (j == 0) {
						old_diveregnce = KL_div;
					}

					Type delta_X = 1;
					if (!X_values_slice.empty()) {
						delta_X = X_values_slice[i+1] - X_values_slice[i];
					}

					normalization_factor += delta_X*(old_diveregnce + delta_X*KL_div)/2;

					old_diveregnce = KL_div;
				}
			}

			return normalization_factor;
		};


		if (Xvalues.empty()) {
			return distortion_factor(std::vector<Type2>({}));
		} else {
			Type max_normalization_factor = 0;

			for (int i = 0; i < Xvalues.size(); ++i) {
				Type normalization_factor     = distortion_factor(Xvalues[i]);
				     max_normalization_factor = std::max(max_normalization_factor, normalization_factor);
			}

			return max_normalization_factor;
		}
	}

	template<typename Type1, typename Type2=double>
	std::vector<Type1> get_normalized_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects, const std::vector<std::vector<size_t>> &indexes,
		const std::vector<std::vector<Type2>> &Xvalues={})
	{
		std::vector<Type1> distortion_coefs(indexes.size(), 0);

		std::vector<Type1> traj(vects.size());
		std::vector<Type1> total_distribution = util::get_total_distribution(vects);

		std::vector<std::vector<size_t>> indexes_slice(1);


		for (size_t i = 0; i < indexes.size(); ++i) {
			indexes_slice[0] = indexes[i];

			auto this_trajectories_ = segregation::multiscalar::get_trajectories(vects, indexes_slice);

			Type1 old_max_KL_div = 0, max_KL_div = 0;
			for (size_t j = indexes[i].size()-1; j > 0; --j) {
				for (size_t k = 0; k < vects.size(); ++k) {
					traj[k] = this_trajectories_[k][0][j];
				}
				Type1 KL_div = ::util::math::get_KLdiv(traj, total_distribution);

				max_KL_div = std::max(max_KL_div, KL_div);
				if (j == indexes[i].size()-1) {
					old_max_KL_div = max_KL_div;
				}

				Type2 delta_X = 1;
				if (Xvalues.size() > i && !Xvalues[i].empty()) {
					delta_X = Xvalues[i][j] - Xvalues[i][j-1];
				}
				distortion_coefs[i] += delta_X*(old_max_KL_div + max_KL_div)/2;

				old_max_KL_div = max_KL_div;
			}
		}

		Type1 normalization_coef = get_normalization_factor(total_distribution, Xvalues, vects[0].size());
		for (Type1 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}

	template<typename Type1, typename Type2=double>
	std::vector<Type1> get_normalized_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects,
		const std::function<std::pair<std::vector<size_t>, std::vector<Type2>>(size_t)> func)
	{
		std::vector<Type1> distortion_coefs(vects[0].size(), 0);

		std::vector<Type1> traj(vects.size());
		std::vector<Type1> total_distribution = util::get_total_distribution(vects);

		std::vector<std::vector<size_t>> indexes_slice(1);
		std::vector<std::vector<Type2>>  Xvalues_slice(1);


		Type1 normalization_coef = 0.d;
		for (size_t i = 0; i < vects[0].size(); ++i) {
			auto [indexes, Xvalues] = func(i);
			indexes_slice[0]        = indexes;
			Xvalues_slice[0]        = Xvalues;

			auto this_trajectories_  = segregation::multiscalar::get_trajectories(vects, indexes_slice);

			Type1 old_max_KL_div = 0, max_KL_div = 0;
			for (size_t j = indexes.size()-1; j > 0; --j) {
				for (size_t k = 0; k < vects.size(); ++k) {
					traj[k] = this_trajectories_[k][0][j];
				}
				Type1 KL_div = ::util::math::get_KLdiv(traj, total_distribution);

				max_KL_div = std::max(max_KL_div, KL_div);
				if (j == indexes.size()-1) {
					old_max_KL_div = max_KL_div;
				}

				Type2 delta_X = 1;
				if (!Xvalues.empty()) {
					delta_X = Xvalues[j] - Xvalues[j-1];
				}
				distortion_coefs[i] += delta_X*(old_max_KL_div + max_KL_div)/2;

				old_max_KL_div = max_KL_div;
			}

			if (Xvalues.empty()) {
				if (i == 0) {
					normalization_coef = get_normalization_factor(total_distribution, std::vector<std::vector<Type2>>{}, vects[0].size());
				}
			} else {
				normalization_coef = std::max(normalization_coef, get_normalization_factor(total_distribution, Xvalues_slice));
			}
		}

		for (Type1 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}
}
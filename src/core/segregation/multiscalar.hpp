#pragma once

#include "../../util/math.hpp"
#include "../../util/util.hpp"

#include "multiscalar_util.hpp"

#include <functional>


namespace segregation::multiscalar {
	template<typename Type>
	std::vector<std::vector<size_t>> get_closest_neighbors(const std::vector<std::vector<Type>> &distances) {
		std::vector<std::vector<size_t>> indexes(distances.size());

		#pragma omp parallel for
		for (size_t i = 0; i < distances.size(); ++i) {
			indexes[i] = ::util::math::get_sorted_indexes(distances[i]);
		}

		return indexes;
	}

	template<typename Type>
	std::vector<std::vector<Type>> get_trajectories_single(const std::vector<Type> &vect, const std::vector<std::vector<size_t>> &indexes) {
		std::vector<std::vector<Type>> trajectory(indexes.size(), std::vector<Type>(indexes.size()));

		#pragma omp parallel for
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

		#pragma omp parallel for
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

		#pragma omp parallel for private(placeholder)
		for (size_t i = 0; i < trajectories[0].size(); ++i) {
			placeholder.resize(trajectories.size());

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

		#pragma omp parallel for
		for (size_t i = 0; i < trajectory.size(); ++i) {
			for (size_t j = 0; j < trajectory[i].size(); ++j) {
				KLdiv_trajectories[i][j] = ::util::math::get_KLdiv_single(trajectory[i][j], total_distribution);
			}
		}
		
		return KLdiv_trajectories;
	}

	std::vector<std::vector<size_t>> get_focal_distance_indexes(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<double> &convergence_thresholds) {
		std::vector<std::vector<size_t>> focal_distance_indexes(KLdiv_trajectories.size(), std::vector<size_t>(convergence_thresholds.size()));

		#pragma omp parallel for
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

		#pragma omp parallel for
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

	template<typename Type=double, typename Type2=double>
	std::vector<Type> get_distortion_coefs_from_KLdiv(const std::vector<std::vector<double>> &KLdiv_trajectories, const std::vector<std::vector<Type>> &Xvalues={}, const Type2 &normalization_coef=1.d) {
		std::vector<Type> distortion_coefs(KLdiv_trajectories.size(), 0);

		#pragma omp parallel for
		for (size_t i = 0; i < KLdiv_trajectories.size(); ++i) {
			double max_KL_div = 0, old_max_KL_div = KLdiv_trajectories[i].back();

			for (long long int j = KLdiv_trajectories[0].size()-2; j >= 0; --j) {
				max_KL_div = std::max(max_KL_div, KLdiv_trajectories[i][j]);

				Type delta_X = 1;
				if (!Xvalues.empty()) {
					delta_X = Xvalues[i][j+1] - Xvalues[i][j];
				}
				distortion_coefs[i] += delta_X*(old_max_KL_div + max_KL_div)/2;

				old_max_KL_div = max_KL_div;
			}

			distortion_coefs[i] /= normalization_coef;
		}

		return distortion_coefs;
	}

	template<typename Type1, typename Type2=double, typename Type3=double>
	std::vector<Type1> get_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects,
		const std::function<std::pair<std::vector<size_t>, std::vector<Type2>>(size_t)> func,
		const Type3 normalization_coef=1.d)
	{
		std::vector<Type1> distortion_coefs(vects[0].size(), 0);

		std::vector<Type1> traj(vects.size());
		std::vector<Type1> total_distribution = util::get_total_distribution(vects);

		std::vector<std::vector<size_t>> indexes_slice(1);
		std::vector<std::vector<Type2>>  Xvalues_slice(1);


		#pragma omp parallel for private(traj, indexes_slice, Xvalues_slice)
		for (size_t i = 0; i < vects[0].size(); ++i) {
			traj.resize(vects.size());
			indexes_slice.resize(1);
			Xvalues_slice.resize(1);

			auto [indexes, Xvalues] = func(i);
			indexes_slice[0]        = indexes;
			Xvalues_slice[0]        = Xvalues;

			auto this_trajectories_ = segregation::multiscalar::get_trajectories(vects, indexes_slice);

			for (size_t k = 0; k < vects.size(); ++k) {
				traj[k] = this_trajectories_[k][0].back();
			}
			double max_KL_div = 0, old_max_KL_div = ::util::math::get_KLdiv(traj, total_distribution);

			for (long long int j = indexes.size()-2; j >= 0; --j) {
				for (size_t k = 0; k < vects.size(); ++k) {
					traj[k] = this_trajectories_[k][0][j];
				}
				double KL_div = ::util::math::get_KLdiv(traj, total_distribution);
				max_KL_div   = std::max(max_KL_div, KL_div);

				Type2 delta_X = 1;
				if (!Xvalues.empty()) {
					delta_X = Xvalues[j+1] - Xvalues[j];
				}
				distortion_coefs[i] += delta_X*(old_max_KL_div + max_KL_div)/2;

				old_max_KL_div = max_KL_div;
			}
		}

		for (Type1 &distortion_coef : distortion_coefs) {
			distortion_coef /= normalization_coef;
		}
		
		return distortion_coefs;
	}

	template<typename Type1, typename Type2=double, typename Type3=double>
	std::vector<Type1> get_distortion_coefs_fast(
		const std::vector<std::vector<Type1>> &vects, const std::vector<std::vector<size_t>> &indexes,
		const Type3 normalization_coef=1.d,
		const std::vector<std::vector<Type2>> &Xvalues={})
	{
		return get_normalized_distortion_coefs_fast(vects,
			(std::function<std::pair<std::vector<size_t>, std::vector<Type2>>(size_t)>) [&indexes, &Xvalues](size_t i) {
				if (Xvalues.empty()) {
					return std::pair<std::vector<size_t>, std::vector<Type2>>(indexes[i], {});
				} else {
					return std::pair<std::vector<size_t>, std::vector<Type2>>(indexes[i], Xvalues[i]);
				}
			}, normalization_coef);
	}

	template<typename Type1, typename Type2=double>
	Type1 get_normalization_factor(const std::vector<std::vector<Type1>> &vects, const std::vector<std::vector<Type2>> &Xvalues={}) {
		/* Get population of each node,
		the "worst trajectory" would be if we encounter the smallest to largest population: */
		std::vector<Type1> populations = util::get_populations(vects);
		std::sort(populations.begin(), populations.end());
		const Type1        total_pop   = std::accumulate(populations.begin(), populations.end(), (Type1)0);

		/* Get the total distribution,
		sort it from smallest to biggest proportion,
		as the "worst trajectory" would be if we encounter the smallest minorities first */
		std::vector<Type1> total_distribution = util::get_total_distribution(vects);
		std::vector<Type1> total_distribution_copy = total_distribution;
		std::sort(total_distribution_copy.begin(), total_distribution_copy.end());

		/* Get the population of each minority: */
		std::vector<Type1> total_distribution_pop = total_distribution_copy;
		for (Type1 &total_distribution_pop_ : total_distribution_pop) {
			total_distribution_pop_ *= total_pop;
		}

		/* Get the X value trajectory,
		the "worst trajectory" is the one where we encounter the furthest 1st node,
		then the furhtest 2nd node, etc... */
		std::vector<Type2> worst_Xvalues = util::get_worst_Xvalues(Xvalues, vects[0].size());


		Type1 normalization_coef = 0;

		std::vector<Type1> placeholder(vects.size()), accumulated_pop(vects.size(), 0);
		Type1 accumulated_total_pop = populations[0];
		int current_idx = 0;


		while (total_distribution_pop[current_idx] < populations[0]) {
			accumulated_pop[current_idx] += total_distribution_pop[current_idx];
			populations[0]               -= total_distribution_pop[current_idx];
			total_distribution_pop[current_idx] = 0;
			++current_idx;
		}
		accumulated_pop[       current_idx] += populations[0];
		total_distribution_pop[current_idx] -= populations[0];

		for (size_t k = 0; k < vects.size(); ++k) {
			placeholder[k] = accumulated_pop[k]/accumulated_total_pop;
		}
		double KL_div = 0, old_KL_div = ::util::math::get_KLdiv(placeholder, total_distribution);


		for (size_t i = 1; i < vects[0].size(); ++i) {
			accumulated_total_pop += populations[i];

			while (total_distribution_pop[current_idx] < populations[i] && current_idx < vects.size()-1) {
				accumulated_pop[current_idx] += total_distribution_pop[current_idx];
				populations[i]               -= total_distribution_pop[current_idx];
				total_distribution_pop[current_idx] = 0;
				++current_idx;
			}
			accumulated_pop[       current_idx] += populations[i];
			total_distribution_pop[current_idx] -= populations[i];

			for (size_t k = 0; k < vects.size(); ++k) {
				placeholder[k] = accumulated_pop[k]/accumulated_total_pop;
			}
			KL_div = ::util::math::get_KLdiv(placeholder, total_distribution);

			Type2 delta_X = worst_Xvalues[i] - worst_Xvalues[i-1];

			normalization_coef += delta_X*(old_KL_div + KL_div)/2;

			old_KL_div = KL_div;
		}


		return normalization_coef;
	}
}
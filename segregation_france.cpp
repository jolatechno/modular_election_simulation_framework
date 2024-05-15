#include <iostream>

#include "src/core/segregation/multiscalar.hpp"
#include "src/core/segregation/multiscalar_util.hpp"
#include "src/core/segregation/map_util.hpp"

#include "src/util/hdf5_util.hpp"
#include "src/util/util.hpp"


const std::vector<std::string> candidates_from_left_to_right = {
	"ARTHAUD",
	"POUTOU",
	"MÉLENCHON",
	"ROUSSEL",
	"HIDALGO",
	"JADOT",
	"LASSALLE",
	"MACRON",
	"PÉCRESSE",
	"DUPONT_AIGNAN",
	"LE_PEN",
	"ZEMMOUR"
};
const int N_candidates = 12;

const int N_full_analyze = 800;

const char* input_file_name  = "output/preprocessed_partial.h5";
const char* output_file_name = "output/segregation_output.h5";


int main() {
	H5::H5File output_file(output_file_name, H5F_ACC_TRUNC);
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);


	std::vector<float> lat, lon;
	H5::Group geo_data = input_file.openGroup("geo_data");
	util::hdf5io::H5ReadVector(geo_data, lat, "lat");
	util::hdf5io::H5ReadVector(geo_data, lon, "lon");

	std::vector<double> populations;
	H5::Group demo_data = input_file.openGroup("demo_data");
	util::hdf5io::H5ReadVector(demo_data, populations,  "voter_population");

	std::vector<std::vector<double>> votes(N_candidates);
	H5::Group vote_data = input_file.openGroup("vote_data");
	for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
		std::string field_name = "Voix_" + candidates_from_left_to_right[icandidate];
		util::hdf5io::H5ReadVector(vote_data, votes[icandidate], field_name.c_str());
	}


	auto convergence_thresholds = util::math::logspace<double>(1e-7d, 9.d, 400);

	{
		std::vector<size_t> full_analyze_idxs(lat.size());
		std::iota(full_analyze_idxs.begin(), full_analyze_idxs.end(), 0);
		std::shuffle(full_analyze_idxs.begin(), full_analyze_idxs.end(), util::get_random_generator());
		full_analyze_idxs.resize(N_full_analyze);
		std::sort(full_analyze_idxs.begin(), full_analyze_idxs.end());

		std::cout << "Computing partial analysis...\n";


		auto distances_slice       = segregation::map::util::get_distances(lat, lon, full_analyze_idxs);
		auto traj_idxes_slice      = segregation::multiscalar::get_closest_neighbors(distances_slice);
		auto vote_trajectories     = segregation::multiscalar::get_trajectories(votes, traj_idxes_slice);
		auto KLdiv_trajectories    = segregation::multiscalar::get_KLdiv_trajectories(vote_trajectories);
		auto focal_distances_idxes = segregation::multiscalar::get_focal_distance_indexes(KLdiv_trajectories, convergence_thresholds);


		H5::Group partial_analysis = output_file.createGroup("partial_analysis");
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, distances_slice,   "distances");
		util::hdf5io::H5WriteVector(           partial_analysis, full_analyze_idxs, "full_analyze_idxs");
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, traj_idxes_slice,  "knn");

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			std::string field_name = "vote_trajectory_" + candidates_from_left_to_right[icandidate];
			util::hdf5io::H5WriteIrregular2DVector(partial_analysis, vote_trajectories[icandidate], field_name.c_str());
		}
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, KLdiv_trajectories,    "KLdiv_trajectories");
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, focal_distances_idxes, "focal_distances_idxes");
	}

	std::cout << "Computing full analysis...\n";

	auto normalized_distortion_coefs = segregation::multiscalar::get_normalized_distortion_coefs_fast(votes, convergence_thresholds,
		(std::function<std::pair<std::vector<size_t>, std::vector<double>>(size_t)>) [&lat, &lon](size_t i) {
			auto distances_slice  = segregation::map::util::get_distances(lat, lon, std::vector<size_t>{i});
			auto traj_idxes_slice = segregation::multiscalar::get_closest_neighbors(distances_slice);

			return std::pair<std::vector<size_t>, std::vector<double>>(traj_idxes_slice[0], {});
		});
	std::cout << "\nnormalized distortion coefs: " << normalized_distortion_coefs << "  <<  (" << *std::max_element(normalized_distortion_coefs.begin(), normalized_distortion_coefs.end()) << ")\n";

	auto normalized_distortion_coefs_pop = segregation::multiscalar::get_normalized_distortion_coefs_fast(votes, convergence_thresholds,
		(std::function<std::pair<std::vector<size_t>, std::vector<double>>(size_t)>) [&votes, &lat, &lon](size_t i) {
			auto distances_slice  = segregation::map::util::get_distances(lat, lon, std::vector<size_t>{i});
			auto traj_idxes_slice = segregation::multiscalar::get_closest_neighbors(distances_slice);

			auto accumulated_trajectory_pop = segregation::multiscalar::util::get_accumulated_trajectory(votes, traj_idxes_slice);

			return std::pair<std::vector<size_t>, std::vector<double>>(traj_idxes_slice[0], accumulated_trajectory_pop[0]);
		});
	std::cout << "\nnormalized distortion coefs (pop): " << normalized_distortion_coefs_pop << "  <<  (" << *std::max_element(normalized_distortion_coefs_pop.begin(), normalized_distortion_coefs_pop.end()) << ")\n";

	auto normalized_distortion_coefs_dist = segregation::multiscalar::get_normalized_distortion_coefs_fast(votes, convergence_thresholds,
		(std::function<std::pair<std::vector<size_t>, std::vector<float>>(size_t)>) [&votes, &lat, &lon](size_t i) {
			auto distances_slice  = segregation::map::util::get_distances(lat, lon, std::vector<size_t>{i});
			auto traj_idxes_slice = segregation::multiscalar::get_closest_neighbors(distances_slice);

			std::sort(distances_slice[0].begin(), distances_slice[0].end());

			return std::pair<std::vector<size_t>, std::vector<float>>(traj_idxes_slice[0], distances_slice[0]);
		});
	std::cout << "\nnormalized distortion coefs (dist): " << normalized_distortion_coefs_dist << "  <<  (" << *std::max_element(normalized_distortion_coefs_dist.begin(), normalized_distortion_coefs_dist.end()) << ")\n";

	H5::Group full_analysis = output_file.createGroup("full_analysis");
	util::hdf5io::H5WriteVector(full_analysis, normalized_distortion_coefs,      "normalized_distortion_coefs");
	util::hdf5io::H5WriteVector(full_analysis, normalized_distortion_coefs_pop,  "normalized_distortion_coefs_pop");
	util::hdf5io::H5WriteVector(full_analysis, normalized_distortion_coefs_dist, "normalized_distortion_coefs_dist");
}
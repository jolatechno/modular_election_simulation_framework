#include <iostream>

#include "src/core/segregation/multiscalar.hpp"
#include "src/core/segregation/multiscalar_util.hpp"

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

const char* input_file_name  = "output/preprocessed.h5";
const char* output_file_name = "output/segregation_output.h5";


int main() {
	H5::H5File output_file(output_file_name, H5F_ACC_TRUNC);
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);


	std::vector<float> lat, lon;
	H5::Group geo_data = input_file.openGroup("geo_data");
	util::hdf5io::H5ReadVector(geo_data, lat, "lat");
	util::hdf5io::H5ReadVector(geo_data, lon, "lon");


	std::cout << "Computing distances...\n";
	auto distances = segregation::multiscalar::get_distances(lat, lon);

	std::cout << "Computing KNN...\n";
	auto traj_idxes = segregation::multiscalar::get_closest_neighbors(distances);


	H5::Group out_geo_data = output_file.createGroup("geo_data");
	util::hdf5io::H5WriteIrregular2DVector(out_geo_data, distances, "distances");
	util::hdf5io::H5WriteIrregular2DVector(out_geo_data, distances, "knn");


	std::cout << "Reading voter data...\n";

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

		std::vector<std::vector<size_t>> traj_idxes_slice(N_full_analyze);
		for (size_t i = 0; i < N_full_analyze; ++i) {
			traj_idxes_slice[i] = traj_idxes[full_analyze_idxs[i]];
		}

		H5::Group partial_analysis = output_file.createGroup("partial_analysis");
		util::hdf5io::H5WriteVector(           partial_analysis, full_analyze_idxs, "full_analyze_idxs");
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, traj_idxes_slice,  "knn");


		std::cout << "Computing partial analysis...\n";


		auto vote_trajectories     = segregation::multiscalar::get_trajectories(votes, traj_idxes_slice);
		auto KLdiv_trajectories    = segregation::multiscalar::get_KLdiv_trajectories(vote_trajectories);
		auto focal_distances_idxes = segregation::multiscalar::get_focal_distance_indexes(KLdiv_trajectories, convergence_thresholds);

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			std::string field_name = "vote_trajectory_" + candidates_from_left_to_right[icandidate];
			util::hdf5io::H5WriteIrregular2DVector(partial_analysis, vote_trajectories[icandidate], field_name.c_str());
		}
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, KLdiv_trajectories,    "KLdiv_trajectories");
		util::hdf5io::H5WriteIrregular2DVector(partial_analysis, focal_distances_idxes, "focal_distances_idxes");
	}

	std::cout << "Computing full analysis...\n";

	auto normalized_distortion_coefs = segregation::multiscalar::get_normalized_distortion_coefs_fast(votes, convergence_thresholds,
		(std::function<std::pair<std::vector<size_t>, std::vector<double>>(int)>) [&traj_idxes](int i) {
			return std::pair<std::vector<size_t>, std::vector<double>>(traj_idxes[i], {});
		});
	std::cout << "\nnormalized distortion coefs: " << normalized_distortion_coefs << "  <<  (" << *std::max_element(normalized_distortion_coefs.begin(), normalized_distortion_coefs.end()) << ")\n";

	H5::Group full_analysis = output_file.createGroup("full_analysis");
	util::hdf5io::H5WriteVector(full_analysis, normalized_distortion_coefs, "normalized_distortion_coefs");
}
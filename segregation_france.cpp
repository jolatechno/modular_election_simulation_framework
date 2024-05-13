#include <iostream>

#include "src/core/segregation/multiscalar.hpp"

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

const char* input_file_name  = "output/preprocessed.h5";
const char* output_file_name = "output/segregation_output.h5";


int main() {
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);


	std::vector<float> lat, lon;
	H5::Group geo_data = input_file.openGroup("geo_data");
	util::hdf5io::H5ReadVector(geo_data, lat, "lat");
	util::hdf5io::H5ReadVector(geo_data, lon, "lon");


	auto distances = segregation::multiscalar::get_distances(lat, lon);
	std::cout << "distances: " << distances[0] << "\n";

	auto traj_idxes = segregation::multiscalar::get_closest_neighbors(distances);
	std::cout << "\nindexes: "   << traj_idxes[0] << "\n";

	std::vector<double> sorted_distances(50);
	for (int i = 0; i < sorted_distances.size(); ++i) {
		sorted_distances[i] = distances[0][traj_idxes[0][i]];
	}
	std::cout << "\nsorted_distances: "   << sorted_distances << "\n";


	std::vector<double> populations;
	H5::Group demo_data = input_file.openGroup("demo_data");
	util::hdf5io::H5ReadVector(demo_data, populations,  "voter_population");

	std::vector<std::vector<double>> votes(N_candidates);
	H5::Group vote_data = input_file.openGroup("vote_data");
	for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
		std::string field_name = "PROP_Voix_" + candidates_from_left_to_right[icandidate];
		util::hdf5io::H5ReadVector(vote_data, votes[icandidate], field_name.c_str());
	}


	auto convergence_thresholds = util::math::linspace<double>(0.d, 9.d, 400);

	auto vote_trajectories = segregation::multiscalar::get_trajectories(votes, traj_idxes);
	std::cout << "\nvote trajectory \"" << candidates_from_left_to_right[7] << "\": " << vote_trajectories[7][0] << "\n";

	auto KLdiv_trajectories = segregation::multiscalar::get_KLdiv_trajectories(vote_trajectories);
	std::cout << "\nKLdiv trajectory: " << KLdiv_trajectories[0] << "\n";

	auto focal_distance_indexes = segregation::multiscalar::get_focal_distance_indexes(KLdiv_trajectories, convergence_thresholds);
	std::cout << "\nfocal distance indexes: " << focal_distance_indexes[0] << "\n";


	auto normalization_factor = segregation::multiscalar::get_normalization_factor(vote_trajectories, convergence_thresholds);
	std::cout << "\nnormalization factor: " << normalization_factor << "\n";

	auto distortion_coefs = segregation::multiscalar::get_distortion_coefs(focal_distance_indexes, convergence_thresholds);
	std::cout << "distortion coefs: " << distortion_coefs << "\n";

	auto normalized_distortion_coefs = segregation::multiscalar::get_distortion_coefs(focal_distance_indexes, convergence_thresholds, normalization_factor);
	std::cout << "normalized distortion coefs: " << normalized_distortion_coefs << " < " << *std::max_element(normalized_distortion_coefs.begin(), normalized_distortion_coefs.end()) << "\n";
}
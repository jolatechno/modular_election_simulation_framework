#include <iostream>

#include "src/core/network.hpp"
#include "src/core/networks/network_file_io.hpp"
#include "src/core/networks/network_generator.hpp"
#include "src/core/networks/network_partition.hpp"
#include "src/core/networks/network_util.hpp"
#include "src/core/agent_population/agent_population.hpp"
#include "src/implementations/Nvoter_model.hpp"
#include "src/implementations/population_Nvoter_model.hpp"

#include "src/util/json_util.hpp"
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

const std::string root        = "output/";
const std::string config_file = "config.json";


template<class Type>
double get_median(std::vector<Type> vec) {
	std::sort(vec.begin(), vec.end()); 

	size_t median_element = vec.size()/2;
	return vec[median_element];
} 


int main(int argc, char *argv[]) {
	std::string config_name = util::get_first_cmd_arg(argc, argv);
	auto config             = util::json::read_config((root + config_file).c_str(), config_name);

	const std::string input_file_name  = root + std::string(config["preprocessed_file"    ].asString());
	const std::string output_file_name = root + std::string(config["output_file_convergence_time"].asString());

	const size_t N_select = config["convergence_time"]["N_select"].asInt();

	      size_t N_nodes;
	const int    N_try  = config["convergence_time"]["N_try" ].asInt();
	const int    N_it   = config["convergence_time"]["N_it"  ].asInt();
	const int    n_save = config["convergence_time"]["n_save"].asInt();

	H5::H5File output_file(output_file_name, H5F_ACC_TRUNC);
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);

	auto *interaction      = new BPsimulation::implem::population_Nvoter_interaction_function<N_candidates>(N_select);
	auto *renormalize      = new BPsimulation::core::agent::population::PopulationRenormalizeProportions<BPsimulation::implem::Nvoter<N_candidates>>();
	auto *agent_serializer = new BPsimulation::core::agent::population::AgentPopulationSerializer<BPsimulation::implem::Nvoter<N_candidates>>();


	auto *network = new BPsimulation::SocialNetwork<BPsimulation::core::agent::population::AgentPopulation<BPsimulation::implem::Nvoter<N_candidates>>>();
	BPsimulation::io::read_network_from_file(network, input_file);
	BPsimulation::io::write_network_to_file( network, output_file);
	N_nodes = network->num_nodes();


	std::vector<float> lat;
	H5::Group geo_data = input_file.openGroup("geo_data");
	util::hdf5io::H5ReadVector(geo_data, lat, "lat");

	std::vector<std::vector<size_t>> counties = {{}, {}};
	float median = get_median(lat);
	for (size_t node = 0; node < N_nodes; ++node) {
		int group = lat[node] < median;
		counties[group].push_back(node);
	}

	BPsimulation::io::write_counties_to_file(counties, output_file);


	std::vector<double> populations;
	H5::Group demo_data = input_file.openGroup("demo_data");
	util::hdf5io::H5ReadVector(demo_data, populations,  "voter_population");

	std::vector<std::vector<double>> votes(N_candidates);
	H5::Group vote_data = input_file.openGroup("vote_data");
	for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
		std::string field_name = "PROP_Voix_" + candidates_from_left_to_right[icandidate];
		util::hdf5io::H5ReadVector(vote_data, votes[icandidate], field_name.c_str());
	}

	for (size_t node = 0; node < N_nodes; ++node) {

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			(*network)[node].proportions[icandidate] = votes[icandidate][node];
		}

		(*network)[node].population = (size_t)populations[node];
	}
	network->update_agentwise(renormalize);

	BPsimulation::io::write_agent_states_to_file(network, agent_serializer, output_file, "/initial_state");


	BPsimulation::implem::Nvoter_majority_election_result<N_candidates>* general_election_results;
	std::vector<BPsimulation::core::election::ElectionResultTemplate*> counties_election_results, stuborness_results;
	for (int itry = 0; itry < N_try; ++itry) {
		if (itry > 0) {
			BPsimulation::io::read_agent_states_from_file(network, agent_serializer, output_file, "/initial_state");
		}

		std::cout << "try " << itry+1 << "/" << N_try << "\n";

		for (int it = 0; it < N_it; ++it) {
			if (it%n_save == 0 && it > 0) {
				std::string dir_name = "/states_" + std::to_string(itry) + "_" + std::to_string(it);
				BPsimulation::io::write_agent_states_to_file(network, agent_serializer, output_file, dir_name.c_str());
			}

			network->interact(interaction);
			network->update_agentwise(renormalize);
		}
	}
}
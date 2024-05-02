#include <iostream>

#include "src/core/network.hpp"
#include "src/core/networks/network_file_io.hpp"
#include "src/core/networks/network_generator.hpp"
#include "src/core/networks/network_partition.hpp"
#include "src/core/networks/network_util.hpp"
#include "src/core/agent_population/agent_population.hpp"
#include "src/implementations/voter_model.hpp"
#include "src/implementations/voter_model_stuborn.hpp"
#include "src/implementations/population_voter_model.hpp"
#include "src/implementations/population_voter_model_stuborn.hpp"
#include "src/util/util.hpp"


const size_t N_select = 15;

      size_t N_nodes;
const int    N_counties = 2;
const int    N_try      = 50;
const int    N_it       = 2001;
const int    n_save     = 10;

const char* input_file_name  = "output/preprocessed.h5";
const char* output_file_name = "output/output_3.h5";

template<class Type>
double get_median(std::vector<Type> vec) {
	std::sort(vec.begin(), vec.end()); 

	size_t median_element = vec.size()/2;
	return vec[median_element];
} 

int main() {
	H5::H5File output_file(output_file_name, H5F_ACC_TRUNC);
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);

	auto *interaction      = new population_voter_interaction_function(N_select);
	auto *renormalize      = new PopulationRenormalizeProportions<voter>();
	auto *agent_serializer = new AgentPopulationSerializer<voter>();


	auto *network = new SocialNetworkTemplate<AgentPopulation<voter>>();
	read_network_from_file(network, input_file);
	write_network_to_file( network, output_file);
	N_nodes = network->num_nodes();


	std::vector<float> lat;
	H5::Group geo_data = input_file.openGroup("geo_data");
	H5ReadVector(geo_data, lat, "lat");

	std::vector<std::vector<size_t>> counties = {{}, {}};
	float median = get_median(lat);
	for (size_t node = 0; node < N_nodes; ++node) {
		int group = lat[node] < median;
		counties[group].push_back(node);
	}

	write_counties_to_file(counties, output_file);


	std::vector<double> populations;
	H5::Group demo_data = input_file.openGroup("demo_data");
	H5ReadVector(demo_data, populations,  "voter_population");

	std::vector<double> vote_macron, vote_melenchon;
	H5::Group vote_data = input_file.openGroup("vote_data");
	H5ReadVector(vote_data, vote_macron,    "PROP_Voix_MACRON");
	H5ReadVector(vote_data, vote_melenchon, "PROP_Voix_MÉLENCHON");

	for (size_t node = 0; node < N_nodes; ++node) {
		double total_vote = vote_macron[node] + vote_melenchon[node];

		double prop_macron    = vote_macron[node]/total_vote;
		double prop_melenchon = vote_melenchon[node]/total_vote;

		(*network)[node].population = (size_t)populations[node];

		(*network)[node].proportions[0] = prop_macron;
		(*network)[node].proportions[1] = prop_melenchon;
	}

	write_agent_states_to_file(network, agent_serializer, output_file, "/initial_state");


	voter_majority_election_result* general_election_results;
	std::vector<ElectionResultTemplate*> counties_election_results, stuborness_results;
	for (int itry = 0; itry < N_try; ++itry) {
		if (itry > 0) {
			read_agent_states_from_file(network, agent_serializer, output_file, "/initial_state");
		}

		std::cout << "try " << itry+1 << "/" << N_try << "\n";

		for (int it = 0; it < N_it; ++it) {
			if (it%n_save == 0 && it > 0) {
				std::string dir_name = "/states_" + std::to_string(itry) + "_" + std::to_string(it);
				write_agent_states_to_file(network, agent_serializer, output_file, dir_name.c_str());
			}

			network->interact(interaction);
			network->update_agentwise(renormalize);
		}
	}
}
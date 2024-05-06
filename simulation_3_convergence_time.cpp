#include <iostream>

#include "src/core/network.hpp"
#include "src/core/networks/network_file_io.hpp"
#include "src/core/networks/network_generator.hpp"
#include "src/core/networks/network_partition.hpp"
#include "src/core/networks/network_util.hpp"
#include "src/core/agent_population/agent_population.hpp"
#include "src/implementations/Nvoter_model.hpp"
#include "src/implementations/population_Nvoter_model.hpp"
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

const size_t N_select = 40;

      size_t N_nodes;
const int    N_try      = 20;
const int    N_it       = 2001;
const int    n_save     = 20;

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

	auto *interaction      = new population_Nvoter_interaction_function<N_candidates>(N_select);
	auto *renormalize      = new PopulationRenormalizeProportions<Nvoter<N_candidates>>();
	auto *agent_serializer = new AgentPopulationSerializer<Nvoter<N_candidates>>();


	auto *network = new SocialNetworkTemplate<AgentPopulation<Nvoter<N_candidates>>>();
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

	std::vector<std::vector<double>> votes(N_candidates);
	H5::Group vote_data = input_file.openGroup("vote_data");
	for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
		std::string field_name = "PROP_Voix_" + candidates_from_left_to_right[icandidate];
		H5ReadVector(vote_data, votes[icandidate], field_name.c_str());
	}

	for (size_t node = 0; node < N_nodes; ++node) {

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			(*network)[node].proportions[icandidate] = votes[icandidate][node];
		}

		(*network)[node].population = (size_t)populations[node];
	}
	network->update_agentwise(renormalize);

	write_agent_states_to_file(network, agent_serializer, output_file, "/initial_state");


	Nvoter_majority_election_result<N_candidates>* general_election_results;
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
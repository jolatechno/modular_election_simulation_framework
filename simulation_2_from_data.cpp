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

#include "src/implementations/Nvoter_stuborn_model.hpp"
#include "src/implementations/population_Nvoter_stuborn_model.hpp"

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

const size_t N_select               = 15;
const double dt                     = 0.2;
const double overtoon_multiplier    = 0.07;
const double frustration_multiplier = 0.03;

const double initial_radicalization_multiplier = 0.1;

      size_t N_nodes;
const int    N_counties = 2;
const int    N_try      = 10;
const int    N_it       = 2001;
const int    n_election = 350;
const int    n_save     = 15;

const char* input_file_name  = "output/preprocessed.h5";
const char* output_file_name = "output/output_2.h5";

template<class Type>
double get_median(std::vector<Type> vec) {
	std::sort(vec.begin(), vec.end()); 

	size_t median_element = vec.size()/2;
	return vec[median_element];
} 

int main() {
	H5::H5File output_file(output_file_name, H5F_ACC_TRUNC);
	H5::H5File input_file( input_file_name,  H5F_ACC_RDWR);


	auto *election = new PopulationElection<Nvoter_stuborn<N_candidates>>(new Nvoter_majority_election<N_candidates, Nvoter_stuborn<N_candidates>>());

	auto *interaction = new population_Nvoter_stuborn_interaction_function<N_candidates>(N_select);
	auto *agentwise   = new Nvoter_stuborn_equilibirum_function<           N_candidates>(dt);
	auto *overton     = new Nvoter_stuborn_overtoon_effect<                N_candidates>(dt, overtoon_multiplier);
	auto *frustration = new Nvoter_stuborn_frustration_effect<             N_candidates>(dt, frustration_multiplier);

	auto *renormalize = new PopulationRenormalizeProportions<Nvoter_stuborn<N_candidates>>();

	auto *agent_full_serializer    = new AgentPopulationNVoterStubornSerializer<N_candidates>();
	auto *agent_partial_serializer = new AgentPopulationSerializer<Nvoter_stuborn<N_candidates>>();
	auto *election_serializer      = new NVoterMajorityElectionSerializer<N_candidates>();


	auto *network = new SocialNetworkTemplate<AgentPopulationNVoterStuborn<N_candidates>>();
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
			(*network)[node].proportions[icandidate]         =   votes[icandidate][node];
			(*network)[node].stuborn_equilibrium[icandidate] = 2*votes[icandidate][node]*initial_radicalization_multiplier;
		}

		(*network)[node].population = (size_t)populations[node];
	}
	network->update_agentwise(renormalize);

	write_agent_states_to_file(network, agent_full_serializer, output_file, "/initial_state");


	Nvoter_majority_election_result<N_candidates>* general_election_results;
	std::vector<ElectionResultTemplate*> counties_election_results, stuborness_results;
	for (int itry = 0; itry < N_try; ++itry) {
		if (itry > 0) {
			read_agent_states_from_file(network, agent_full_serializer, output_file, "/initial_state");
		}

		for (int it = 0; it < N_it; ++it) {
			if (it%n_save == 0 && it > 0) {
				std::string dir_name = "/states_" + std::to_string(itry) + "_" + std::to_string(it);
				write_agent_states_to_file(network, agent_partial_serializer, output_file, dir_name.c_str());
			}

			if (it%n_election == 0) {
				general_election_results  = (Nvoter_majority_election_result<N_candidates>*)network->get_election_results(election);
				counties_election_results = network->get_election_results(counties, election);

				std::string dir_name_general  = "/general_election_result_"  + std::to_string(itry) + "_" + std::to_string(it);
				std::string dir_name_counties = "/counties_election_result_" + std::to_string(itry) + "_" + std::to_string(it);

				write_election_result_to_file( general_election_results,  election_serializer, output_file, dir_name_general.c_str());
				write_election_results_to_file(counties_election_results, election_serializer, output_file, dir_name_counties.c_str());

				std::cout << "\n\ntry " << itry+1 << "/" << N_try << ", it " << it << "/" << N_it-1 << ":\n\n";
				std::cout << "network->get_election_results(...) = " << general_election_results->result << " (" << general_election_results->proportions << ")\n";
				std::cout << "network->get_election_results(counties, ...): \n";
				for (int couty = 0; couty < counties.size(); couty++) {
					Nvoter_majority_election_result<N_candidates> *county_result = (Nvoter_majority_election_result<N_candidates>*)counties_election_results[couty];
					std::cout << "\t" << county_result->result  << " (" << county_result->proportions << ") for county: " << counties[couty] << "\n";
				}
				std::cout << "\n";
			}

			network->interact(interaction);
			network->update_agentwise(agentwise);
			network->election_retroinfluence(general_election_results, overton);
			network->election_retroinfluence(general_election_results, frustration);
			network->election_retroinfluence(counties, counties_election_results, overton);
			network->election_retroinfluence(counties, counties_election_results, frustration);
			network->update_agentwise(renormalize);
		}
	}
}
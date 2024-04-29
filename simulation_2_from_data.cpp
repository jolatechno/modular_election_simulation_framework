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


const size_t N_select               = 10;
const double dt                     = 0.2;
const double overtoon_multiplier    = 0.02;
const double frustration_multiplier = 0.02;

const double initial_radicalization_multiplier = 0.1;

      size_t N_nodes;
const int    N_counties = 2;
const int    N_try      = 10;
const int    N_it       = 3001;
const int    n_election = 300;
const int    n_save     = 10;

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


	auto *election            = new PopulationElection<voter_stuborn>(new voter_majority_election<voter_stuborn>());
	auto *stuborness_election = new PopulationElection<voter_stuborn>(new voter_stuborness_election());

	auto *interaction = new population_voter_stuborn_interaction_function(N_select);
	auto *agentwise   = new voter_stuborn_equilibirum_function(dt);
	auto *overton     = new voter_stuborn_overtoon_effect(     dt, overtoon_multiplier);
	auto *frustration = new voter_stuborn_frustration_effect(  dt, frustration_multiplier);

	auto *renormalize = new PopulationRenormalizeProportions<voter_stuborn>();

	auto *agent_full_serializer    = new AgentPopulationVoterStubornSerializer();
	auto *agent_partial_serializer = new AgentPopulationSerializer<voter_stuborn>();
	auto *election_serializer      = new VoterMajorityElectionSerializer();


	auto *network = new SocialNetworkTemplate<AgentPopulationVoterStuborn>();
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

		double prop_macron            = vote_macron[node]/total_vote;
		double prop_macron_stuborn    = prop_macron*initial_radicalization_multiplier;
		double prop_macron_notStuborn = prop_macron - prop_macron_stuborn;

		double macron_radicalization_eq = 2*prop_macron_stuborn;

		double prop_melenchon            = vote_melenchon[node]/total_vote;
		double prop_melenchon_stuborn    = prop_melenchon*initial_radicalization_multiplier;
		double prop_melenchon_notStuborn = prop_melenchon - prop_melenchon_stuborn;

		double melenchon_radicalization_eq = 2*prop_melenchon_stuborn;

		(*network)[node].population = (size_t)populations[node];

		(*network)[node].stuborn_equilibrium[0] = macron_radicalization_eq;
		(*network)[node].stuborn_equilibrium[1] = melenchon_radicalization_eq;

		(*network)[node].proportions[0] = prop_macron_notStuborn;
		(*network)[node].proportions[1] = prop_melenchon_notStuborn;
		(*network)[node].proportions[2] = prop_macron_stuborn;
		(*network)[node].proportions[3] = prop_melenchon_stuborn;
	}

	write_agent_states_to_file(network, agent_full_serializer, output_file, "/initial_state");


	voter_majority_election_result* general_election_results;
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
				general_election_results  = (voter_majority_election_result*)network->get_election_results(election);
				counties_election_results = network->get_election_results(counties, election);
				stuborness_results        = network->get_election_results(counties, stuborness_election);

				std::string dir_name_general  = "/general_election_result_"  + std::to_string(itry) + "_" + std::to_string(it);
				std::string dir_name_counties = "/counties_election_result_" + std::to_string(itry) + "_" + std::to_string(it);
				write_election_result_to_file( general_election_results,  election_serializer, output_file, dir_name_general.c_str());
				write_election_results_to_file(counties_election_results, election_serializer, output_file, dir_name_counties.c_str());

				std::cout << "\n\ntry " << itry+1 << "/" << N_try << ", it " << it << "/" << N_it-1 << ":\n\n";
				std::cout << "network->get_election_results(...) = " << general_election_results->result << " (" << int(general_election_results->proportion*100) << "%)\n";
				std::cout << "network->get_election_results(counties, ...): \n";
				for (int couty = 0; couty < counties.size(); couty++) {
					voter_majority_election_result *county_result = (voter_majority_election_result*)counties_election_results[couty];
					std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[couty] << "\n";

					voter_stuborness_result *county_stuborness = (voter_stuborness_result*)stuborness_results[couty];
					std::cout << "\t\tStuborness distribution: " << county_stuborness->proportions[0] << ", " << county_stuborness->proportions[1] << ", " << county_stuborness->proportions[2] << ", " << county_stuborness->proportions[3] << "\n";
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
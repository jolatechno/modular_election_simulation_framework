#include <iostream>

#include "src/network.hpp"
#include "src/networks/network_file_io.hpp"
#include "src/networks/network_generator.hpp"
#include "src/networks/network_partition.hpp"
#include "src/networks/network_util.hpp"
#include "implementations/voter_model.hpp"
#include "implementations/voter_model_stuborn.hpp"
#include "implementations/population_voter_model.hpp"
#include "implementations/population_voter_model_stuborn.hpp"
#include "src/agent_population/agent_population.hpp"
#include "util/util.hpp"

const size_t N_select               = 10;
const double dt                     = 0.2;
const double overtoon_multiplier    = 0.015;
const double frustration_multiplier = 0.015;

const int N_try      = 10;
const int N_it       = 2001;
const int n_election = 400;
const int n_save     = 5;

const char* file_name =  "output/test.hdf5";

int main() {
	H5::H5File file(file_name, H5F_ACC_TRUNC);


	auto *election            = new PopulationElection<voter_stuborn>(new voter_majority_election<voter_stuborn>());
	auto *stuborness_election = new PopulationElection<voter_stuborn>(new voter_stuborness_election());

	auto *interaction = new population_voter_stuborn_interaction_function(N_select);
	auto *agentwise   = new voter_stuborn_equilibirum_function(dt);
	auto *overton     = new voter_stuborn_overtoon_effect(     dt, overtoon_multiplier);
	auto *frustration = new voter_stuborn_frustration_effect(  dt, frustration_multiplier);

	auto *renormalize = new PopulationRenormalizeProportions<voter_stuborn>();

	// TODO:
	auto *agent_serializer    = new AgentSerializerTemplate<AgentPopulationVoterStuborn>();
	auto *election_serializer = new ElectionResultSerializerTemplate();


	auto *network = new SocialNetworkTemplate<AgentPopulationVoterStuborn>(800);
	preferential_attachment(network, 3);
	write_network_to_file(network, file);

	std::vector<std::vector<size_t>> counties = random_graphAgnostic_partition_graph(network, 3);
	write_counties_to_file(counties, file);

	network_randomize_agent_states_county(network, counties[0], std::vector<double>({0.6, 0.4, 0.1, 0.2}), 100, 200, 0.2, 0.2);
	network_randomize_agent_states_county(network, counties[1], std::vector<double>({0.4, 0.6, 0.1, 0.2}), 50,  250, 0.2, 0.1);
	network_randomize_agent_states_county(network, counties[2], std::vector<double>({0.5, 0.5, 0.1, 0.1}), 100, 300, 0.1, 0.2);
	write_agent_states_to_file(network, agent_serializer, file, "/initial_state");

	voter_majority_election_result* general_election_results;
	std::vector<ElectionResultTemplate*> counties_election_results, stuborness_results;
	for (int itry = 0; itry < N_try; ++itry) {
		read_agent_states_from_file(network, agent_serializer, file);

		for (int it = 0; it < N_it; ++it) {
			if (it%n_save == 0) {
				std::string dir_name = "/states_" + std::to_string(itry) + "_" + std::to_string(it);
				write_agent_states_to_file(network, agent_serializer, file, dir_name.c_str());
			}

			if (it%n_election == 0) {
				general_election_results  = (voter_majority_election_result*)network->get_election_results(election);
				counties_election_results = network->get_election_results(counties, election);
				stuborness_results        = network->get_election_results(counties, stuborness_election);

				std::string dir_name_general  = "/general_election_result_"  + std::to_string(itry) + "_" + std::to_string(it);
				std::string dir_name_counties = "/counties_election_result_" + std::to_string(itry) + "_" + std::to_string(it);
				write_election_result_to_file( general_election_results,  election_serializer, file, dir_name_general.c_str());
				write_election_results_to_file(counties_election_results, election_serializer, file, dir_name_counties.c_str());

				std::cout << "\n\ntry " << itry+1 << "/" << N_try << ", it " << it+1 << "/" << N_it << ":\n\n";
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
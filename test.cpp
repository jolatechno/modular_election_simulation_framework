#include <iostream>

#include "src/core/network.hpp"
#include "src/core/networks/network_generator.hpp"
#include "src/core/networks/network_partition.hpp"
#include "src/core/networks/network_util.hpp"
#include "src/core/agent_population/agent_population.hpp"
#include "src/implementations/voter_model.hpp"
#include "src/implementations/voter_model_stuborn.hpp"
#include "src/implementations/population_voter_model.hpp"
#include "src/implementations/population_voter_model_stuborn.hpp"
#include "src/util/util.hpp"


int main() {
	std::cout << "VOTER MODEL:\n\n";

	{
		auto *test = new SocialNetworkTemplate<voter>(20);

		preferential_attachment(test, 2);
		network_randomize_agent_states(test, 0.5);

		test->add_connection(1, 2);
		test->add_connection(1, 0);

		(*test)[1].candidate = false;
		(*test)[2].candidate = true;

		std::cout << "network.num_nodes() = " << test->num_nodes() << "\n";
		std::cout << "network.degrees() = " << test->degrees() << "\n";
		std::cout << "network[1] = " << (*test)[1].candidate << "\n";
		std::cout << "network.are_neighbors(1, 2) = " << test->are_neighbors(1, 2) << "\n";
		std::cout << "network.are_neighbors(0, 2) = " << test->are_neighbors(0, 2) << "\n";
		std::cout << "network.neighbors(1) = " << test->neighbors(1) << "\n";

		voter_majority_election<voter> *election = new voter_majority_election<voter>();

		voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
		std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

		std::vector<std::vector<size_t>> counties = random_graphAgnostic_partition_graph(test, 3);
		auto results = test->get_election_results(counties, election);
		std::cout << "network->get_election_results(counties, ...): \n";
		for (size_t i = 0; i < counties.size(); i++) {
			voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
			std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";
		}
		std::cout << "\n";

		voter_interaction_function *interaction = new voter_interaction_function();
		for (int i = 0; i < 10; ++i) {
			if (i%1 == 0) {
				voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
				std::cout << "network->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

			}

			test->interact(interaction);
		}
	}

	std::cout << "\n\n\nSTUBORN VOTER MODEL:\n\n";

	{
		auto *test = new SocialNetworkTemplate<voter_stuborn>(20);

		preferential_attachment(test, 2);
		network_randomize_agent_states(test, 0.5, 0.05);

		test->add_connection(1, 2);
		test->add_connection(1, 0);

		(*test)[1].candidate = false;
		(*test)[2].candidate = true;
		(*test)[3].stuborn   = true;

		std::cout << "network.num_nodes() = " << test->num_nodes() << "\n";
		std::cout << "network.degrees() = " << test->degrees() << "\n";
		std::cout << "network[1] = " << (*test)[1].candidate << "\n";
		std::cout << "network[3].stuborn = " << (*test)[3].stuborn << "\n";
		std::cout << "network.are_neighbors(1, 2) = " << test->are_neighbors(1, 2) << "\n";
		std::cout << "network.are_neighbors(0, 2) = " << test->are_neighbors(0, 2) << "\n";
		std::cout << "network.neighbors(1) = " << test->neighbors(1) << "\n";

		voter_majority_election<voter_stuborn> *election = new voter_majority_election<voter_stuborn>();

		voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
		std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

		std::vector<std::vector<size_t>> counties = random_graphAgnostic_partition_graph(test, 3);
		auto results = test->get_election_results(counties, election);
		std::cout << "network->get_election_results(counties, ...): \n";
		for (size_t i = 0; i < counties.size(); i++) {
			voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
			std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";
		}
		std::cout << "\n";

		voter_stuborn_interaction_function *interaction = new voter_stuborn_interaction_function();
		for (int i = 0; i < 10; ++i) {
			if (i%1 == 0) {
				voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
				std::cout << "network->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

			}

			test->interact(interaction);
		}
	}

	std::cout << "\n\n\nPOPULATION DYNAMIC:\n\n";

	{
		auto *test = new SocialNetworkTemplate<AgentPopulation<voter>>(100);

		preferential_attachment(test, 3);
		std::vector<std::vector<size_t>> counties = random_graphAgnostic_partition_graph(test, 3);

		network_randomize_agent_states_county(test, counties[0], 150, 50,  std::vector<double>({0.6, 0.4}));
		network_randomize_agent_states_county(test, counties[1], 150, 100, std::vector<double>({0.4, 0.6}));
		network_randomize_agent_states_county(test, counties[2], 400, 100, std::vector<double>({0.5, 0.5}));


		std::cout << "network.num_nodes() = " << test->num_nodes() << "\n";
		std::cout << "network.degrees() = " << test->degrees() << "\n";
		std::cout << "network.neighbors(0) = " << test->neighbors(0) << "\n";
		std::cout << "network[1].population = " << (*test)[1].population << "\n";
		std::cout << "network[2].proportions = " << (*test)[1].proportions << "\n";

		PopulationElection<voter> *election = new PopulationElection<voter>(new voter_majority_election<voter>());

		voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
		std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

		auto results = test->get_election_results(counties, election);
		std::cout << "network->get_election_results(counties, ...): \n";
		for (int i = 0; i < counties.size(); i++) {
			voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
			std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";
		}
		std::cout << "\n";


		population_voter_interaction_function *interaction = new population_voter_interaction_function(20);

		PopulationRenormalizeProportions<voter> *renormalize = new PopulationRenormalizeProportions<voter>();

		for (int i = 0; i < 51; ++i) {
			if (i%10 == 0) {
				voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
				std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

				auto results = test->get_election_results(counties, election);
				std::cout << "network->get_election_results(counties, ...): \n";
				for (int i = 0; i < counties.size(); i++) {
					voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
					std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";
				}
				std::cout << "\n";
			}

			test->interact(interaction);
			test->update_agentwise(renormalize);
		}
	}

	std::cout << "\n\n\nPOPULATION DYNAMIC (+ stuborn voters):\n\n";

	{
		auto *test = new SocialNetworkTemplate<AgentPopulationVoterStuborn>(800);

		preferential_attachment(test, 3);
		std::vector<std::vector<size_t>> counties = random_graphAgnostic_partition_graph(test, 3);

		network_randomize_agent_states_county(test, counties[0], 0.2, 0.2, 150, 50,  std::vector<double>({0.6, 0.4, 0.1, 0.2}));
		network_randomize_agent_states_county(test, counties[1], 0.2, 0.1, 150, 100, std::vector<double>({0.4, 0.6, 0.1, 0.2}));
		network_randomize_agent_states_county(test, counties[2], 0.1, 0.2, 400, 100, std::vector<double>({0.5, 0.5, 0.1, 0.1}));


		std::cout << "network.num_nodes() = " << test->num_nodes() << "\n";
		std::cout << "network.degrees() = " << test->degrees() << "\n";
		std::cout << "network.neighbors(0) = " << test->neighbors(0) << "\n";
		std::cout << "network[1].population = " << (*test)[1].population << "\n";
		std::cout << "network[2].proportions = " << (*test)[1].proportions << "\n";
		std::cout << "network[3].stuborn_equilibrium[0] = " << (*test)[3].stuborn_equilibrium[0] << "\n";

		PopulationElection<voter_stuborn> *election            = new PopulationElection<voter_stuborn>(new voter_majority_election<voter_stuborn>());
		PopulationElection<voter_stuborn> *stuborness_election = new PopulationElection<voter_stuborn>(new voter_stuborness_election());

		voter_majority_election_result *result = (voter_majority_election_result*)test->get_election_results(election);
		std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

		auto results            = test->get_election_results(counties, election);
		auto stuborness_results = test->get_election_results(counties, stuborness_election);

		std::cout << "network->get_election_results(counties, ...): \n";
		for (int i = 0; i < counties.size(); i++) {
			voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
			std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";

			voter_stuborness_result *county_stuborness = (voter_stuborness_result*)stuborness_results[i];
			std::cout << "\t\tStuborness distribution: " << county_stuborness->proportions[0] << ", " << county_stuborness->proportions[1] << ", " << county_stuborness->proportions[2] << ", " << county_stuborness->proportions[3];
			std::cout << " (" << county_stuborness->proportions[0]+county_stuborness->proportions[1]+county_stuborness->proportions[2]+county_stuborness->proportions[3] << ")\n";
		}
		std::cout << "\n";


		double dt = 0.2;
		population_voter_stuborn_interaction_function *interaction = new population_voter_stuborn_interaction_function(10);
		voter_stuborn_equilibirum_function            *agentwise   = new voter_stuborn_equilibirum_function(dt);
		voter_stuborn_overtoon_effect                 *overton     = new voter_stuborn_overtoon_effect(     dt, 0.015);
		voter_stuborn_frustration_effect              *frustration = new voter_stuborn_frustration_effect(  dt, 0.02);

		PopulationRenormalizeProportions<voter_stuborn> *renormalize = new PopulationRenormalizeProportions<voter_stuborn>();

		for (int i = 0; i < 2001; ++i) {
			if (i%400 == 0) {
				result = (voter_majority_election_result*)test->get_election_results(election);
				std::cout << "\nnetwork->get_election_results(...) = " << result->result << " (" << int(result->proportion*100) << "%)\n";

				results = test->get_election_results(counties, election);
				auto stuborness_results = test->get_election_results(counties, stuborness_election);

				std::cout << "network->get_election_results(counties, ...): \n";
				for (int i = 0; i < counties.size(); i++) {
					voter_majority_election_result *county_result = (voter_majority_election_result*)results[i];
					std::cout << "\t" << county_result->result  << " (" << int(county_result->proportion*100) << "%) for county: " << counties[i] << "\n";

					voter_stuborness_result *county_stuborness = (voter_stuborness_result*)stuborness_results[i];
					std::cout << "\t\tStuborness distribution: " << county_stuborness->proportions[0] << ", " << county_stuborness->proportions[1] << ", " << county_stuborness->proportions[2] << ", " << county_stuborness->proportions[3];
					std::cout << " (" << county_stuborness->proportions[0]+county_stuborness->proportions[1]+county_stuborness->proportions[2]+county_stuborness->proportions[3] << ")\n";
				}
				std::cout << "\n";
			}

			test->interact(interaction);
			test->update_agentwise(agentwise);
			test->election_retroinfluence(result, overton);
			test->election_retroinfluence(result, frustration);
			test->election_retroinfluence(counties, results, overton);
			test->election_retroinfluence(counties, results, frustration);
			test->update_agentwise(renormalize);
		}
	}

	return 0;
}
#pragma once

#include "agent_population.hpp"

#include "../agent.hpp"

#include "../../util/util.hpp"


namespace BPsimulation::core::agent::population::util {
	template<class Agent, class Agent2=AgentPopulation<Agent>>
	std::vector<std::vector<double>> get_vote_proportions(const SocialNetwork<Agent2> *population) {
		const size_t n_agent_type = (*population)[0].agent_types.size();

		std::vector<std::vector<double>> votes(n_agent_type, std::vector<double>(population->num_nodes(), 0));

		#pragma omp parallel
		for (size_t i = 0; i < population->num_nodes(); ++i) {
			for (size_t j = 0; j < n_agent_type; ++j) {
				votes[j][i] = (*population)[i].proportions[j];
			}
		}

		return votes;
	}
}
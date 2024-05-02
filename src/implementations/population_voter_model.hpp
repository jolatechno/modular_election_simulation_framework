#pragma once

#include <random>

#include "../core/agent_population/agent_population.hpp"
#include "voter_model.hpp"

#include "../util/util.hpp"


class population_voter_interaction_function : public AgentPopulationInteractionFunctionTemplate<voter> {
public:
	size_t N_select;
	population_voter_interaction_function(size_t N_select_) : N_select(N_select_) {}

	void operator()(AgentPopulation<voter> &agent, std::vector<const AgentPopulation<voter>*> neighbors) const {
		if (agent.population > 0) {
			std::vector<long int> self_selected         = random_select(N_select, agent);
			std::vector<long int> neighborhood_selected = random_select(N_select, neighbors);

			long int N_candidate1_self = agent.population*agent.proportions[1] - self_selected[1] + neighborhood_selected[1];
			agent.proportions[1] = ((double)N_candidate1_self)/((double)agent.population);
			agent.proportions[0] = 1 - agent.proportions[1];
		}
	}
};
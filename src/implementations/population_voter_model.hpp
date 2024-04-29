#pragma once

#include <random>

#include "../core/agent_population/agent_population.hpp"
#include "voter_model.hpp"

#include "../util/util.hpp"


class population_voter_interaction_function : public AgentInteractionFunctionTemplate<AgentPopulation<voter>> {
public:
	size_t N_select;
	population_voter_interaction_function(size_t N_select_) : N_select(N_select_) {}

	void operator()(AgentPopulation<voter> &agent, std::vector<const AgentPopulation<voter>*> neighbors) const {
		double candidate1_proportion_self = agent.proportions[1];
		double N_population_self = agent.population;
		double N_candidate1_self = N_population_self*candidate1_proportion_self;

		double N_population_neighborhood = N_population_self;
		double N_candidate1_neighborhood = N_candidate1_self;
		for (const AgentPopulation<voter>* neighbor : neighbors) {
			N_population_neighborhood += neighbor->population;
			N_candidate1_neighborhood += neighbor->population*neighbor->proportions[1];
		}
		double candidate1_proportion_neighborhood = std::min(1.d, N_candidate1_neighborhood/N_population_neighborhood);

		std::binomial_distribution<int> distribution_self(        N_select, candidate1_proportion_self);
		std::binomial_distribution<int> distribution_neighborhood(N_select, candidate1_proportion_neighborhood);

		int N_candidate1_selected_self         = distribution_self(get_random_generator());
		int N_candidate1_selected_neighborhood = distribution_neighborhood(get_random_generator());

		N_candidate1_self = N_candidate1_self - N_candidate1_selected_self + N_candidate1_selected_neighborhood;
		agent.proportions[1] = N_candidate1_self/N_population_self;
		agent.proportions[0] = 1 - agent.proportions[1];
	}
};
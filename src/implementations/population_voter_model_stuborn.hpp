#pragma once

#include <random>

#include "../core/agent_population/agent_population.hpp"
#include "voter_model_stuborn.hpp"

#include "../util/util.hpp"

class AgentPopulationVoterStuborn : public AgentPopulation<voter_stuborn> {
public:
	double stuborn_equilibrium[2] = {0, 0};
	void randomize() {
		AgentPopulation<voter_stuborn>::randomize();
	}
	void randomize(size_t pop_min, size_t pop_max) {
		AgentPopulation<voter_stuborn>::randomize(pop_min, pop_max);
	}
	void randomize(size_t pop_min, size_t pop_max, double max_stuborn_equilibrium0, double max_stuborn_equilibrium1) {
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		stuborn_equilibrium[0] = distribution(get_random_generator())*max_stuborn_equilibrium0;
		stuborn_equilibrium[1] = distribution(get_random_generator())*max_stuborn_equilibrium1;

		randomize(pop_min, pop_max);
	}
	void randomize(std::vector<double> &mean_proportions, size_t pop_min, size_t pop_max) {
		AgentPopulation<voter_stuborn>::randomize(mean_proportions, pop_min, pop_max);
	}
	void randomize(std::vector<double> &mean_proportions, size_t pop_min, size_t pop_max, double max_stuborn_equilibrium0, double max_stuborn_equilibrium1) {
		std::uniform_real_distribution<double> distribution(0.0, 1.0);
		stuborn_equilibrium[0] = distribution(get_random_generator())*max_stuborn_equilibrium0;
		stuborn_equilibrium[1] = distribution(get_random_generator())*max_stuborn_equilibrium1;

		randomize(mean_proportions, pop_min, pop_max);
	}
};

class population_voter_stuborn_interaction_function : public AgentInteractionFunctionTemplate<AgentPopulationVoterStuborn> {
public:
	size_t N_select;
	population_voter_stuborn_interaction_function(size_t N_select_) : N_select(N_select_) {}

	void operator()(AgentPopulationVoterStuborn &agent, std::vector<const AgentPopulationVoterStuborn*> neighbors) const {
		const double N_population_self = agent.population*(agent.proportions[1] + agent.proportions[0]);
		double       N_candidate1_self = agent.population* agent.proportions[1];
		if (N_population_self <= N_select) {
			return;
		}
		double candidate1_proportion_self = std::min(1.d, N_candidate1_self/N_population_self);

		double N_population_neighborhood = agent.population;
		double N_candidate1_neighborhood = agent.population*(agent.proportions[1] + agent.proportions[3]);
		for (const AgentPopulationVoterStuborn* neighbor : neighbors) {
			N_population_neighborhood += neighbor->population;
			N_candidate1_neighborhood += neighbor->population*(neighbor->proportions[1] + neighbor->proportions[3]);
		}
		if (N_population_neighborhood <= N_select) {
			return;
		}
		double candidate1_proportion_neighborhood = std::min(1.d, N_candidate1_neighborhood/N_population_neighborhood);

		std::binomial_distribution<int> distribution_self(        N_select, candidate1_proportion_self);
		std::binomial_distribution<int> distribution_neighborhood(N_select, candidate1_proportion_neighborhood);

		int N_candidate1_selected_self         = distribution_self(get_random_generator());
		int N_candidate1_selected_neighborhood = distribution_neighborhood(get_random_generator());

		N_candidate1_self = std::min(N_population_self, std::max(0.d,
			N_candidate1_self + N_candidate1_selected_neighborhood - N_candidate1_selected_self));
		agent.proportions[1] =                      N_candidate1_self /agent.population;
		agent.proportions[0] = (N_population_self - N_candidate1_self)/agent.population;
	}
};

class voter_stuborn_equilibirum_function : public AgentWiseUpdateFunctionTemplate<AgentPopulationVoterStuborn> {
public:
	double dt;

	voter_stuborn_equilibirum_function(double dt_=0.1) : dt(dt_) {}
	void operator()(AgentPopulationVoterStuborn &agent) const {
		double radicalization_flux0 = std::max(agent.proportions[2]-1, std::min(agent.proportions[0],
			dt*(agent.proportions[0]*agent.stuborn_equilibrium[0] - agent.proportions[2])));
		double radicalization_flux1 = std::max(agent.proportions[3]-1, std::min(agent.proportions[1],
			dt*(agent.proportions[1]*agent.stuborn_equilibrium[1] - agent.proportions[3])));

		agent.proportions[0] -= radicalization_flux0;
		agent.proportions[2] += radicalization_flux0;
		agent.proportions[1] -= radicalization_flux1;
		agent.proportions[3] += radicalization_flux1;
	}
};

class voter_stuborn_overtoon_effect : public ElectionRetroinfluenceTemplate<AgentPopulationVoterStuborn> {
private:
	inline double overtoon_distance(double distance) const {
		double normalized_distance = 2*std::abs(distance) - 1;
		return std::pow(normalized_distance, exponent);
	}
public:
	double dt, multiplier, exponent; 

	voter_stuborn_overtoon_effect(double dt_=0.1, double multiplier_=1, double exponent_=3) :
		dt(dt_), multiplier(multiplier_), exponent(exponent_) {}
	void operator()(AgentPopulationVoterStuborn& agent, const ElectionResultTemplate* election_result_) const {
		voter_majority_election_result *election_result = (voter_majority_election_result*)election_result_;

		double proportion = election_result->proportion;
		if (proportion > 0.5) {
			double overtoon_multiplier = dt*multiplier*overtoon_distance(proportion);

			double deradicalization_flux0 = overtoon_multiplier*agent.proportions[2];
			double unstuborn_flip_flux    = overtoon_multiplier*agent.proportions[0];
			double radicalization_flux1   = overtoon_multiplier*agent.proportions[1];

			agent.proportions[0] += deradicalization_flux0 - unstuborn_flip_flux;
			agent.proportions[1] += unstuborn_flip_flux   - radicalization_flux1;
			agent.proportions[2] +=                       - deradicalization_flux0;
			agent.proportions[3] += radicalization_flux1;
		} else if (proportion < 0.5) {
			double overtoon_multiplier = dt*multiplier*overtoon_distance(1 - proportion);

			double deradicalization_flux1 = overtoon_multiplier*agent.proportions[3];
			double unstuborn_flip_flux    = overtoon_multiplier*agent.proportions[1];
			double radicalization_flux0   = overtoon_multiplier*agent.proportions[0];

			agent.proportions[0] += unstuborn_flip_flux   - radicalization_flux0;
			agent.proportions[1] +=                       - deradicalization_flux1;
			agent.proportions[2] += radicalization_flux0;
			agent.proportions[3] +=                       - deradicalization_flux1;
		}
	}
};

class voter_stuborn_frustration_effect : public ElectionRetroinfluenceTemplate<AgentPopulationVoterStuborn> {
public:
	double dt, multiplier; 
	
	voter_stuborn_frustration_effect(double dt_=0.1, double multiplier_=1) : dt(dt_), multiplier(multiplier_) {}
	void operator()(AgentPopulationVoterStuborn& agent, const ElectionResultTemplate* election_result_) const {
		voter_majority_election_result *election_result = (voter_majority_election_result*)election_result_;

		if (election_result->result) {
			double radicalization_flux0 = dt*multiplier*agent.proportions[0];
			
			agent.proportions[0] -= radicalization_flux0;
			agent.proportions[2] += radicalization_flux0;
		} else {
			double radicalization_flux1 = dt*multiplier*agent.proportions[1];

			agent.proportions[1] -= radicalization_flux1;
			agent.proportions[3] += radicalization_flux1;
		}
	}
};

class AgentPopulationVoterStubornSerializer : public AgentSerializerTemplate<AgentPopulationVoterStuborn> {
public:
	std::vector<std::pair<std::string, int>> list_of_fields() const {
		return {
			{"proportions_false_notStuborn", 6},
			{"proportions_true_notStuborn",  6},
			{"proportions_false_stuborn",    6},
			{"proportions_true_stuborn",     6},
			{"stuborn_equilibrium_false",    6},
			{"stuborn_equilibrium_true",     6},
			{"population",                   4}
		};
	}
	std::vector<variable_type> write(const AgentPopulationVoterStuborn &agent) const {
		std::vector<variable_type> values(7);

		values[0] = agent.proportions[0];
		values[1] = agent.proportions[1];
		values[2] = agent.proportions[2];
		values[3] = agent.proportions[3];

		values[4] = agent.stuborn_equilibrium[0];
		values[5] = agent.stuborn_equilibrium[1];

		values[6] = agent.population;

		return values;
	}
	void read(AgentPopulationVoterStuborn &agent, const std::vector<variable_type> &values) const {
		agent.proportions[0] = std::get<double>(values[0]);
		agent.proportions[1] = std::get<double>(values[1]);
		agent.proportions[2] = std::get<double>(values[2]);
		agent.proportions[3] = std::get<double>(values[3]);

		agent.stuborn_equilibrium[0] = std::get<double>(values[4]);
		agent.stuborn_equilibrium[1] = std::get<double>(values[5]);

		agent.population = std::get<size_t>(values[6]);
	}
};
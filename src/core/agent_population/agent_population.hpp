#pragma once

#include "../../util/util.hpp"


template<class Agent>
class AgentPopulation : public AgentTemplate {
public:
	const std::vector<const Agent*> agent_types = (new Agent)->list_of_possible_agents();
	size_t population = 1;
	std::vector<double> proportions = std::vector<double>(agent_types.size());

	void randomize() {
		std::normal_distribution<double> distribution(1.0, 0.5);

		double normalization_factor = 0.0;
		for (int i = 0; i < agent_types.size(); ++i) {
			proportions[i] = std::max(1e-9d, distribution(get_random_generator()));
			normalization_factor += proportions[i];
		}
		for (int i = 0; i < agent_types.size(); ++i) {
			proportions[i] /= normalization_factor;
		}
	}
	void randomize(std::vector<double> &mean_proportions) {
		std::normal_distribution<double> distribution(1.0, 0.5);

		double normalization_factor = 0.0;
		for (int i = 0; i < agent_types.size(); ++i) {
			proportions[i] = std::max(1e-9d, distribution(get_random_generator())*mean_proportions[i]);
			normalization_factor += proportions[i];
		}
		for (int i = 0; i < agent_types.size(); ++i) {
			proportions[i] /= normalization_factor;
		}
	}
	void randomize(size_t pop_min, size_t pop_max) {
		std::uniform_int_distribution<size_t> distribution(pop_min, pop_max);
		population = distribution(get_random_generator());

		randomize();
	}
	void randomize(std::vector<double> &mean_proportions, int pop_min, int pop_max) {
		std::uniform_int_distribution<size_t> distribution(pop_min, pop_max);
		population = distribution(get_random_generator());

		randomize(mean_proportions);
	}
};

template<class Agent>
class PopulationElection : public ElectionTemplate<AgentPopulation<Agent>> {
private:
	const ElectionTemplate<Agent> *base_election_func;
public:
	PopulationElection(const ElectionTemplate<Agent> *electionfunc) : base_election_func(electionfunc) {}

	ElectionResultTemplate* get_neutral_election_result() const { 
		return base_election_func->get_neutral_election_result();
	};
	ElectionResultTemplate* operator()(const AgentPopulation<Agent>& agent) const {
		ElectionResultTemplate *result = get_neutral_election_result();
		for (int i = 0; i < agent.agent_types.size(); ++i) {
			ElectionResultTemplate *type_result = (*base_election_func)(*agent.agent_types[i]);

			int agent_type_population = int(agent.population*agent.proportions[i]);
			(*type_result) *= agent_type_population;

			(*result) += type_result;
		}

		return result;
	};
};

template<class Agent>
class PopulationRenormalizeProportions : public AgentWiseUpdateFunctionTemplate<AgentPopulation<Agent>> {
public:
	PopulationRenormalizeProportions() {}
	void operator()(AgentPopulation<Agent> &agent) const {
		double normalization_factor = 0.0;
		for (double &proportion : agent.proportions) {
			proportion            = std::max(0.d, std::min(1.d, proportion));
			normalization_factor += proportion;
		}
		for (double &proportion : agent.proportions) {
			proportion /= normalization_factor;
		}
	}
};
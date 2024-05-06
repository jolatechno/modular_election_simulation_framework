#pragma once

#include <random>
#include <algorithm>

#include "../core/election.hpp"
#include "../core/agent.hpp"

#include "../util/util.hpp"

#include "Nvoter_model.hpp"


template<int N_candidates>
class Nvoter_stuborn : public Nvoter<N_candidates> {
private:
	Nvoter_stuborn(int candidate_, bool stuborn_) : stuborn(stuborn_) {
		Nvoter<N_candidates>::candidate = candidate_;
	}
public:
	Nvoter_stuborn() {}

	bool stuborn=false;

	template<typename ...Args>
	void randomize(float p_stuborn=0, Args... args) {
		std::uniform_real_distribution<float> distribution(0.0, 1.0);
		stuborn = distribution(get_random_generator()) < p_stuborn;

		Nvoter<N_candidates>::randomize(args...);
	}

	std::vector<const Nvoter_stuborn<N_candidates>*> list_of_possible_agents() {
		std::vector<const Nvoter_stuborn<N_candidates>*> possible_agents;

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			possible_agents.push_back(new Nvoter_stuborn<N_candidates>(icandidate, false));
		}
		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			possible_agents.push_back(new Nvoter_stuborn<N_candidates>(icandidate, true));
		}

		return possible_agents;
	}
};

template<int N_candidates>
class Nvoter_stuborn_interaction_function : public AgentInteractionFunctionTemplate<Nvoter_stuborn<N_candidates>> {
public:
	void operator()(Nvoter_stuborn<N_candidates> &agent, std::vector<const Nvoter_stuborn<N_candidates>*> neighbors) const {
		if (!agent.stuborn) {
			std::uniform_int_distribution<int> distribution(0, neighbors.size()-1);
			int neighbor_idx = distribution(get_random_generator());

			agent.candidate = neighbors[neighbor_idx]->candidate;
		}
	}
};

template<int N_candidates>
class NVoterStubornSerializer : public AgentSerializerTemplate<Nvoter_stuborn<N_candidates>> {
public:
	typedef std::variant<bool, int, unsigned int, long, size_t, float, double> variable_type;

	std::vector<std::pair<std::string, int>> list_of_fields() const {
		return {
			{"candidate", 1},
			{"stuborn",   0}
		};
	}
	std::vector<variable_type> write(const Nvoter_stuborn<N_candidates> &agent) const {
		std::vector<variable_type> values(1);
		
		values[0] = agent.candidate;
		values[1] = agent.stuborn;

		return values;
	}
	void read(Nvoter_stuborn<N_candidates> &agent, const std::vector<variable_type> &values) const {
		agent.candidate = std::get<int >(values[0]);
		agent.stuborn   = std::get<bool>(values[1]);
	}
};
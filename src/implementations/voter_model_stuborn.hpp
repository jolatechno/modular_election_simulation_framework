#pragma once

#include <random>

#include "voter_model.hpp"

#include "../util/util.hpp"


class voter_stuborn : public voter {
private:
	voter_stuborn(bool candidate_, bool stuborn_) : stuborn(stuborn_) {
		candidate = candidate_;
	}
public:
	voter_stuborn() {}

	bool stuborn=false;
	
	void randomize(float p=0.5, float p_stuborn=0) {
		std::uniform_real_distribution<float> distribution(0.0, 1.0);
		
		candidate = distribution(get_random_generator()) < p;
		stuborn   = distribution(get_random_generator()) < p_stuborn;
	}

	std::vector<const voter_stuborn*> list_of_possible_agents() {
		std::vector<const voter_stuborn*> possible_agents;
		possible_agents.push_back(new voter_stuborn(false, false));
		possible_agents.push_back(new voter_stuborn(true,  false));
		possible_agents.push_back(new voter_stuborn(false, true));
		possible_agents.push_back(new voter_stuborn(true,  true));
		return possible_agents;
	}
};

class voter_stuborn_interaction_function : public AgentInteractionFunctionTemplate<voter_stuborn> {
public:
	void operator()(voter_stuborn &agent, std::vector<const voter_stuborn*> neighbors) const {
		if (!agent.stuborn) {
			std::uniform_int_distribution<int> distribution(0, neighbors.size()-1);
			int neighbor_idx = distribution(get_random_generator());

			agent.candidate = neighbors[neighbor_idx]->candidate;
		}
	}
};

class voter_stuborness_result : public ElectionResultTemplate {
public:
	size_t candidate0_notStuborn=0, candidate1_notStuborn=0, candidate0_stuborn=0, candidate1_stuborn=0;
	float proportions[4] = {0, 0, 0, 0};

	voter_stuborness_result() {};
	ElectionResultTemplate& operator+=(const ElectionResultTemplate* other_) {
		voter_stuborness_result *other = (voter_stuborness_result*)other_;

		candidate0_notStuborn += other->candidate0_notStuborn;
		candidate1_notStuborn += other->candidate1_notStuborn;
		candidate0_stuborn    += other->candidate0_stuborn;
		candidate1_stuborn    += other->candidate1_stuborn;
		return *this;
	}
	ElectionResultTemplate& operator*=(size_t N) {
		candidate0_notStuborn *= N;
		candidate1_notStuborn *= N;
		candidate0_stuborn    *= N;
		candidate1_stuborn    *= N;
		return *this;
	};
	void post_process() {
		size_t total = candidate0_notStuborn + candidate1_notStuborn + candidate0_stuborn + candidate1_stuborn;
		proportions[0] = ((float)candidate0_notStuborn)/((float)total);
		proportions[1] = ((float)candidate1_notStuborn)/((float)total);
		proportions[2] = ((float)candidate0_stuborn   )/((float)total);
		proportions[3] = ((float)candidate1_stuborn   )/((float)total);
	}

};

class voter_stuborness_election : public ElectionTemplate<voter_stuborn> {
public:
	voter_stuborness_result* get_neutral_election_result() const {
		return new voter_stuborness_result();
	}
	voter_stuborness_result* operator()(const voter_stuborn& agent) const {
		voter_stuborness_result *result = new voter_stuborness_result();

		result->candidate0_notStuborn = (!agent.candidate) && (!agent.stuborn);
		result->candidate1_notStuborn =   agent.candidate  && (!agent.stuborn);
		result->candidate0_stuborn    = (!agent.candidate) &&   agent.stuborn;
		result->candidate1_stuborn    =   agent.candidate  &&   agent.stuborn;

		return result;
	}
};
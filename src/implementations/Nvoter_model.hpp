#pragma once

#include <random>
#include <algorithm>

#include "../core/election.hpp"
#include "../core/agent.hpp"

#include "../util/util.hpp"


template<int N_candidates>
class Nvoter : AgentTemplate {
private:
	Nvoter(int candidate_) : candidate(candidate_) {}
public:
	Nvoter() {}

	int candidate = 0;
	
	void randomize(const std::vector<double> &probas=std::vector<double>(N_candidates, 1.d)) {
		double normalization_factor = std::accumulate(probas.begin(), probas.end(), 0.d);

		std::uniform_real_distribution<double> distribution(0.d, normalization_factor);
		double rng_value = distribution(get_random_generator());

		candidate = -1;
		while (rng_value > 0 && candidate <= N_candidates) {
			++candidate;
			rng_value -= probas[candidate];
		}
	}

	std::vector<const Nvoter<N_candidates>*> list_of_possible_agents() {
		std::vector<const Nvoter<N_candidates>*> possible_agents;

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			possible_agents.push_back(new Nvoter<N_candidates>(icandidate));
		}

		return possible_agents;
	}
};

template<int N_candidates>
class Nvoter_majority_election_result : public ElectionResultTemplate {
public:
	std::vector<size_t> votes       = std::vector<size_t>(N_candidates, 0);
	std::vector<double> proportions = std::vector<double>(N_candidates, 0.d);
	int result;

	Nvoter_majority_election_result() {};
	ElectionResultTemplate& operator+=(const ElectionResultTemplate* other_) {
		Nvoter_majority_election_result<N_candidates> *other = (Nvoter_majority_election_result<N_candidates>*)other_;

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			votes[icandidate] += other->votes[icandidate];
		}

		return *this;
	}
	ElectionResultTemplate& operator*=(size_t N) {
		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			votes[icandidate] *= N;
		}

		return *this;
	};
	void post_process() {
		double normalization_factor = (double)std::accumulate(votes.begin(), votes.end(), (size_t)0);
		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			proportions[icandidate] = votes[icandidate]/normalization_factor;
		}

		result = std::distance(votes.begin(),
			std::max_element(votes.begin(), votes.end()));
	}

};

template<int N_candidates, class Agent>
class Nvoter_majority_election : public ElectionTemplate<Nvoter<N_candidates>> {
public:
	Nvoter_majority_election_result<N_candidates>* get_neutral_election_result() const {
		return new Nvoter_majority_election_result<N_candidates>();
	}
	Nvoter_majority_election_result<N_candidates>* operator()(const Nvoter<N_candidates>& agent) const {
		Nvoter_majority_election_result<N_candidates> *result = new Nvoter_majority_election_result<N_candidates>();

		result->votes[agent.candidate] = 1;

		return result;
	}
};

template<int N_candidates>
class Nvoter_interaction_function : public AgentInteractionFunctionTemplate<Nvoter<N_candidates>> {
public:
	void operator()(Nvoter<N_candidates> &agent, std::vector<const Nvoter<N_candidates>*> neighbors) const {
		std::uniform_int_distribution<int> distribution(0, neighbors.size()-1);
		int neighbor_idx = distribution(get_random_generator());

		agent.candidate = neighbors[neighbor_idx]->candidate;
	}
};

template<int N_candidates>
class NVoterMajorityElectionSerializer : public ElectionResultSerializerTemplate {
	std::vector<std::pair<std::string, int>> list_of_fields() const {
		std::vector<std::pair<std::string, int>> fields = {{"result", 1}};

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			std::string field_name = "vote_" + std::to_string(icandidate);
			fields.push_back({field_name, 4});
		}

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			std::string field_name = "proportion_" + std::to_string(icandidate);
			fields.push_back({field_name, 4});
		}

		return fields;
	}
	std::vector<variable_type> write(const ElectionResultTemplate &result_) const {
		Nvoter_majority_election_result<N_candidates> &result = (Nvoter_majority_election_result<N_candidates>&)result_;

		std::vector<variable_type> values;
		values.push_back(result.result);

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			values.push_back(result.votes[icandidate]);
		}

		for (int icandidate = 0; icandidate < N_candidates; ++icandidate) {
			values.push_back(result.proportions[icandidate]);
		}

		return values;
	}
};

template<int N_candidates>
class NVoterSerializer : public AgentSerializerTemplate<Nvoter<N_candidates>> {
public:
	typedef std::variant<bool, int, unsigned int, long, size_t, float, double> variable_type;

	std::vector<std::pair<std::string, int>> list_of_fields() const {
		return {
			{"candidate", 1}
		};
	}
	std::vector<variable_type> write(const Nvoter<N_candidates> &agent) const {
		std::vector<variable_type> values(1);
		
		values[0] = agent.candidate;

		return values;
	}
	void read(voter &agent, const std::vector<variable_type> &values) const {
		agent.candidate = std::get<int>(values[0]);
	}
};
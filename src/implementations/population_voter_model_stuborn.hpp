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

class population_voter_stuborn_interaction_function : public AgentInteractionFunctionTemplate<AgentPopulation<voter_stuborn>> {
public:
	size_t N_select;
	population_voter_stuborn_interaction_function(size_t N_select_) : N_select(N_select_) {}

	void operator()(AgentPopulation<voter_stuborn> &agent, std::vector<const AgentPopulation<voter_stuborn>*> neighbors) const {
		if (agent.population > 0) {
			std::vector<double> self_selected         = agent.random_select(N_select, {2, 3});
			std::vector<double> neighborhood_selected = agent.random_select(N_select, neighbors);

			agent.integrate_population_variation({
				neighborhood_selected[0] + neighborhood_selected[2] - self_selected[0],
				neighborhood_selected[1] + neighborhood_selected[3] - self_selected[1],
				0,
				0});
		}
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

		agent.integrate_proportion_variation({
			-radicalization_flux0,
			-radicalization_flux1,
			 radicalization_flux0,
			 radicalization_flux1});
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

			agent.integrate_proportion_variation({
				deradicalization_flux0 - unstuborn_flip_flux,
				unstuborn_flip_flux    - radicalization_flux1,
				                       - deradicalization_flux0,
				radicalization_flux1});
		} else if (proportion < 0.5) {
			double overtoon_multiplier = dt*multiplier*overtoon_distance(1 - proportion);

			double deradicalization_flux1 = overtoon_multiplier*agent.proportions[3];
			double unstuborn_flip_flux    = overtoon_multiplier*agent.proportions[1];
			double radicalization_flux0   = overtoon_multiplier*agent.proportions[0];

			agent.integrate_proportion_variation({
				unstuborn_flip_flux   - radicalization_flux0,
				                      - deradicalization_flux1,
				radicalization_flux0,
				                      - deradicalization_flux1});
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
			
			agent.integrate_proportion_variation({
				-radicalization_flux0,
				0,
				 radicalization_flux0,
				0});
		} else {
			double radicalization_flux1 = dt*multiplier*agent.proportions[1];

			agent.integrate_proportion_variation({
				0,
				-radicalization_flux1,
				0,
				 radicalization_flux1});
		}
	}
};

class AgentPopulationVoterStubornSerializer : public AgentSerializerTemplate<AgentPopulationVoterStuborn> {
private:
	AgentPopulationSerializer<voter_stuborn> partial_serializer;

public:
	typedef std::variant<bool, int, unsigned int, long, size_t, float, double> variable_type;
	std::vector<std::pair<std::string, int>> list_of_fields() const {
		std::vector<std::pair<std::string, int>> list_of_fields_ = partial_serializer.list_of_fields();

		list_of_fields_.push_back({"stuborn_equilibrium_false", 6});
		list_of_fields_.push_back({"stuborn_equilibrium_true",  6});

		return list_of_fields_;
	}
	std::vector<variable_type> write(const AgentPopulationVoterStuborn &agent) const {
		std::vector<variable_type> values = partial_serializer.write(agent);

		values.push_back(agent.stuborn_equilibrium[0]);
		values.push_back(agent.stuborn_equilibrium[1]);

		return values;
	}
	void read(AgentPopulationVoterStuborn &agent, const std::vector<variable_type> &values) const {
		partial_serializer.read(agent, values);

		agent.stuborn_equilibrium[0] = std::get<double>(values[5]);
		agent.stuborn_equilibrium[1] = std::get<double>(values[6]);
	}
};
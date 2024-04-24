#pragma once

#include <variant>


class AgentTemplate {
public:
	template<typename... Args>
	void randomize(Args... args) {};
};

template<class Agent>
class AgentInteractionFunctionTemplate {
public:
	virtual void operator()(Agent &agent, std::vector<const Agent*> neighbors) const {}
	virtual std::vector<const Agent*> list_of_possible_agents() const { return {NULL}; }
};

template<class Agent>
class AgentWiseUpdateFunctionTemplate {
public:
	virtual void operator()(Agent &agent) const {}
};

template<class Agent>
class AgentSerializerTemplate {
public:
	using variable_type = std::variant<bool, int, unsigned int, long, size_t, float, double>;
	
	virtual std::vector<std::pair<std::string, int>> list_of_fields() const { return {}; }
	virtual std::vector<variable_type> write(const Agent &agent) const { return {}; }
	virtual void read(Agent &agent, const std::vector<variable_type> &values) const {}
};
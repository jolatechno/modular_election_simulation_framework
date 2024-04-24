#pragma once

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
	virtual size_t size() const { return 0; }
	virtual std::string write(const Agent &agent) const { return ""; }
	virtual void read(Agent &agent, const std::string&) const {}
};
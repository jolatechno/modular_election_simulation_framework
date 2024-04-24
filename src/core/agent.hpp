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
protected:
	int type_id(bool          X) { return 0; }
	int type_id(int           X) { return 1; }
	int type_id(unsigned int  X) { return 2; }
	int type_id(long          X) { return 3; }
	int type_id(size_t        X) { return 4; }
	int type_id(float         X) { return 5; }
	int type_id(double        X) { return 6; }
	template<class Type>
	int type_id(Type          X) {
		static_assert(false, "Type not defined as serializable !");
		return -1;
	}

public:
	using variable_type = std::variant<bool, int, unsigned int, long, size_t, float, double>;

	virtual std::vector<std::pair<std::string, int>> list_of_fields() const { return {}; }
	virtual std::vector<variable_type> write(const Agent &agent) const { return {}; }
	virtual void read(Agent &agent, const std::vector<variable_type> &values) const {}
};
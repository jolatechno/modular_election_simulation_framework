#pragma once

#include "../../util/hdf5_util.hpp"

#include "../network.hpp"
#include "../agent.hpp"

#include "H5Cpp.h"


template<class Agent>
SocialNetworkTemplate<Agent> *read_network_from_file(H5::H5File &file, const char* group_name="/network") {
	SocialNetworkTemplate<Agent> *network = new SocialNetworkTemplate<Agent>(0);

	H5::Group group = file.openGroup(group_name);

	/* TODO */

	return network;
}
template<class Agent>
void write_network_to_file(const SocialNetworkTemplate<Agent> *network, H5::H5File &file, const char* group_name="/network") {
	H5::Group group = file.createGroup(group_name);

	/* TODO */
}


template<class Agent, class Agent2>
void read_agent_states_from_file(SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> *serializer, H5::H5File &file, const char* group_name="/states") {
	static_assert(std::is_convertible<Agent2, Agent>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	H5::Group group = file.openGroup(group_name);

	/* TODO */
}
template<class Agent, class Agent2>
void write_agent_states_to_file(const SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> *serializer, H5::H5File &file, const char* group_name="/states") {
	static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	H5::Group group = file.createGroup(group_name);

	/* TODO */
}


std::vector<std::vector<size_t>> read_counties_from_file(H5::H5File &file, const char* group_name="/counties") {
	std::vector<std::vector<size_t>> counties;

	H5::Group group = file.openGroup(group_name);

	/* TODO */

	return counties;
}
void write_counties_to_file(const std::vector<std::vector<size_t>> &counties, H5::H5File &file, const char* group_name="/counties") {
	H5::Group group = file.createGroup(group_name);

	/* TODO */
}


void write_election_result_to_file(const ElectionResultTemplate *result, const ElectionResultSerializerTemplate *serializer, H5::H5File &file, const char* group_name="/election_result") {
	H5::Group group = file.createGroup(group_name);

	/* TODO */
}
void write_election_results_to_file(const std::vector<ElectionResultTemplate*> &result, const ElectionResultSerializerTemplate *serializer, H5::H5File &file, const char* group_name="/election_results") {
	H5::Group group = file.createGroup(group_name);

	/* TODO */
}
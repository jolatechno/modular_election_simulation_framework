#pragma once

#include "../network.hpp"
#include "../agent.hpp"

#include "H5Cpp.h"


template<class Agent>
SocialNetworkTemplate<Agent> *read_network_from_file(H5::H5File &file, const char* directory="/network") {
	SocialNetworkTemplate<Agent> *network = new SocialNetworkTemplate<Agent>(0);
	/* TODO */
	return network;
}
template<class Agent>
SocialNetworkTemplate<Agent> *read_network_from_file(const char* file_name, const char* directory="/network") {
	H5::H5File file(file_name, H5F_ACC_RDONLY);
	return read_network_from_file<Agent>(file, directory);
}


template<class Agent>
void write_network_to_file(const SocialNetworkTemplate<Agent> *network, H5::H5File &file, const char* directory="/network") {
	/* TODO */
}
template<class Agent>
void write_network_to_file(const SocialNetworkTemplate<Agent> *network, const char* file_name, const char* directory="/network") {
	H5::H5File file(file_name, H5F_ACC_TRUNC);
	write_network_to_file(network, file, directory);
}


template<class Agent, class Agent2>
void read_agent_states_from_file(SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> &serializer, H5::H5File &file, const char* directory="/states") {
	static_assert(std::is_convertible<Agent2, Agent>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	/* TODO */
}
template<class Agent, class Agent2>
void read_agent_states_from_file(SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> &serializer, const char* file_name, const char* directory="/states") {
	H5::H5File file(file_name, H5F_ACC_RDONLY);
	read_agent_states_from_file(network, serializer, file, directory);
}


template<class Agent, class Agent2>
void write_agent_states_to_file(const SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> &serializer, H5::H5File &file, const char* directory="/states") {
	static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	/* TODO */
}
template<class Agent, class Agent2>
void write_agent_states_to_file(const SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> &serializer, const char* file_name, const char* directory="/states") {
	H5::H5File file(file_name, H5F_ACC_TRUNC);
	write_agent_states_to_file(network, serializer, file, directory);
}


std::vector<std::vector<size_t>> read_counties_from_file(H5::H5File &file, const char* directory="/counties") {
	std::vector<std::vector<size_t>> counties;
	/* TODO */
	return counties;
}
std::vector<std::vector<size_t>> read_counties_from_file(const char* file_name, const char* directory="/counties") {
	H5::H5File file(file_name, H5F_ACC_RDONLY);
	return read_counties_from_file(file, directory);
}


void write_counties_to_file(const std::vector<std::vector<size_t>> &counties, H5::H5File &file, const char* directory="/counties") {
	/* TODO */
}
void write_counties_to_file(const std::vector<std::vector<size_t>> &counties, const char* file_name, const char* directory="/counties") {
	H5::H5File file(file_name, H5F_ACC_TRUNC);
	write_counties_to_file(counties, file, directory);
}
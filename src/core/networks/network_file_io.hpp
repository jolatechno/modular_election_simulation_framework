#pragma once

#include "../../util/hdf5_util.hpp"

#include "../network.hpp"
#include "../agent.hpp"

#include "H5Cpp.h"


template<class Agent>
void write_network_to_file(const SocialNetworkTemplate<Agent> *network, H5::H5File &file, const char* group_name="/network") {
	H5::Group group = file.createGroup(group_name);

	std::vector<std::vector<size_t>> neighbors;
	for (size_t node = 0; node < network->num_nodes(); ++node) {
		neighbors.push_back(network->neighbors(node));
	}
	H5WriteIrregular2DVector(group, neighbors, "neighbors");
}
template<class Agent>
auto read_network_from_file(SocialNetworkTemplate<Agent> *network, H5::H5File &file, const char* group_name="/network") {
	H5::Group group = file.openGroup(group_name);

	network->clear_connections();

	std::vector<std::vector<size_t>> neighbors(0, std::vector<size_t>(0));
	H5ReadIrregular2DVector(group, neighbors, "neighbors");

	for (size_t node = 0; node < network->num_nodes(); ++node) {
		for (size_t neighbor : neighbors[node]) {
			network->add_connection_single_way(node, neighbor);
		}
	}
}


template<class Agent, class Agent2>
void write_agent_states_to_file(const SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> *serializer, H5::H5File &file, const char* group_name="/states") {
	static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	H5::Group group = file.createGroup(group_name);

	/* TODO */
}
template<class Agent, class Agent2>
void read_agent_states_from_file(SocialNetworkTemplate<Agent> *network, const AgentSerializerTemplate<Agent2> *serializer, H5::H5File &file, const char* group_name="/states") {
	static_assert(std::is_convertible<Agent2, Agent>::value, "Error: Agent class is not compatible with the one used by AgentSerializerTemplate in read_agent_states_from_file !");

	H5::Group group = file.openGroup(group_name);

	/* TODO */
}


void write_counties_to_file(const std::vector<std::vector<size_t>> &counties, H5::H5File &file, const char* group_name="/counties") {
	H5::Group group = file.createGroup(group_name);
	H5WriteIrregular2DVector(group, counties, "counties");
}
void read_counties_from_file(std::vector<std::vector<size_t>> &counties, H5::H5File &file, const char* group_name="/counties") {
	H5::Group group = file.openGroup(group_name);
	H5ReadIrregular2DVector(group, counties, "counties");
}


void write_election_result_to_file(const ElectionResultTemplate *result, const ElectionResultSerializerTemplate *serializer, H5::H5File &file, const char* group_name="/election_result") {
	H5::Group group = file.createGroup(group_name);

	/* TODO */
}
void write_election_results_to_file(const std::vector<ElectionResultTemplate*> &results, const ElectionResultSerializerTemplate *serializer, H5::H5File &file, const char* group_name="/election_results") {
	H5::Group group = file.createGroup(group_name);

	for (size_t i = 0; i < results.size(); ++i) {
		std::string this_election_group_name = std::string(group_name) + "/election_result_" + std::to_string(i);
		write_election_result_to_file(results[i], serializer, file, this_election_group_name.c_str());
	}
}
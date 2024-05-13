#pragma once

#include <random>
#include <algorithm>

#include "../network.hpp"

#include "../../util/util.hpp"


namespace BPsimulation::random {
	template<class Agent>
	void preferential_attachment(SocialNetwork<Agent> *network, int n_attachment) {
		std::vector<size_t> degrees = network->degrees();
		size_t total_degree = std::accumulate(degrees.begin(), degrees.end(), 0);

		auto nodes = network->nodes();
		std::shuffle(nodes.begin(), nodes.end(), util::get_random_generator());

		if (total_degree == 0) {
			size_t i = nodes[network->num_nodes()-2];
			size_t j = nodes[network->num_nodes()-1];

			network->add_connection(i, j);

			degrees[i] += 1;
			degrees[j] += 1;
			total_degree = 2;
		}

		for (int i = 0; i < n_attachment; ++i) {
			for (size_t node : nodes) {
				size_t node_degree_save = degrees[node];
				total_degree -= node_degree_save;
				degrees[node] = 0;

				std::uniform_int_distribution<size_t> distribution(0, total_degree-1);
				size_t rdm = distribution(util::get_random_generator());

				for (size_t j = 0;; ++j) {
					if (rdm <= degrees[nodes[j]]) {
						degrees[nodes[j]] += 1;
						network->add_connection(node, nodes[j]);

						break;
					}
					rdm -= degrees[nodes[j]];
				}
				degrees[node]  = node_degree_save + 1;
				total_degree  += node_degree_save + 2;
			}
		}
	}
}
#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <random>

#include "election.hpp"
#include "agent.hpp"

#include "../util/util.hpp"


namespace BPsimulation {
	template<class Agent>
	class SocialNetwork {
	private:
		std::vector<std::pair<std::vector<size_t>, Agent>> agent_map;
			std::vector<Agent> placeholder;;

		template<class Agent2>
		std::vector<const Agent2*> get_neighbors(size_t node) const {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one seeked in get_neighbors (private function of SocialNetwork) !");

			std::vector<const Agent2*> vec;

			std::vector<size_t> neighbor_list = neighbors(node);
			for (size_t neighbor : neighbor_list) {
				vec.push_back((const Agent2*)&(*this)[neighbor]);
			}

			return vec;
		}
	public:
		SocialNetwork(size_t num_nodes=0) {
			resize(num_nodes);
		}

		inline size_t num_nodes() const {
			return agent_map.size();
		}
		inline void resize(size_t num_nodes) {
			agent_map.resize(num_nodes);
		}
		inline std::vector<size_t> nodes() const {
			std::vector<size_t> nodes(num_nodes());
			std::iota(nodes.begin(), nodes.end(), 0);
			return nodes;
		}
		inline Agent& operator[](size_t node) {
			return agent_map[node].second;
		}
		inline const Agent& operator[](size_t node) const {
			return agent_map[node].second;
		}

		inline const std::vector<size_t>& neighbors(size_t node) const {
			return agent_map[node].first;
		}

		inline bool are_neighbors(size_t i, size_t j) const {
			if (i == j) {
				return true;
			}
			auto i_neighbors = neighbors(i);
			return std::find(i_neighbors.begin(), i_neighbors.end(), j) != i_neighbors.end();
		}
		inline void add_connection_single_way(size_t i, size_t j) {
			if (!are_neighbors(i, j)) {
				agent_map[i].first.push_back(j);
			}
		}
		inline void add_connection(size_t i, size_t j) {
			add_connection_single_way(i, j);
			add_connection_single_way(j, i);
		}

		inline void remove_connection_single_way(size_t i, size_t j) {
			std::remove(agent_map[i].first.begin(), agent_map[i].first.end(), j);
		}
		inline void remove_connection(size_t i, size_t j) {
			remove_connection_single_way(i, j);
			remove_connection_single_way(j, i);
		}
		inline void clear_connections(size_t i) {
			agent_map[i].first.clear();
		}
		inline void clear_connections() {
			for (size_t node = 0; node < num_nodes(); ++node) {
				clear_connections(node);
			}
		}

		inline size_t degree(size_t node) const {
			return neighbors(node).size();
		}
		inline std::vector<size_t> degrees() const {
			std::vector<size_t> degrees(num_nodes());
			for (size_t node = 0; node < num_nodes(); ++node) {
				degrees[node] = degree(node);
			}
			return degrees;
		}

		template<class Agent2>
		inline void interact_serial(const core::agent::AgentInteractionFunctionTemplate<Agent2> *interactionfunc) {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentInteractionFunctionTemplate in interact_serial !");

			std::vector<size_t> node_lists = nodes();
			std::shuffle(node_lists.begin(), node_lists.end(), util::get_random_generator());

			for (size_t node : node_lists) {
				(*interactionfunc)((Agent2&)(*this)[node], get_neighbors<Agent2>(node));
			}
		}

		template<class Agent2>
		inline void interact_parallel(const core::agent::AgentInteractionFunctionTemplate<Agent2> *interactionfunc) {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentInteractionFunctionTemplate in interact_parallel !");

			placeholder.resize(num_nodes());
			#pragma omp parallel for
			for (size_t node = 0; node < num_nodes(); ++node) {
				placeholder[node] = (*this)[node];
			}
			#pragma omp parallel for
			for (size_t node = 0; node < num_nodes(); ++node) {
				(*interactionfunc)((Agent2&)placeholder[node], get_neighbors<Agent2>(node));
			}
			#pragma omp parallel for
			for (size_t node = 0; node < num_nodes(); ++node) {
				(*this)[node] = placeholder[node];
			}
		}

		template<class Agent2>
		inline void interact(const core::agent::AgentInteractionFunctionTemplate<Agent2> *interactionfunc, bool parallel=false) {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentInteractionFunctionTemplate in interact !");
			if (parallel) {
				interact_parallel(interactionfunc);
			} else {
				interact_serial(  interactionfunc);
			}
		}

		template<class Agent2>
		core::election::ElectionResultTemplate* get_election_results(const std::vector<size_t> &county, const core::election::ElectionTemplate<Agent2> *electionfunc) const {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by ElectionTemplate in ElectionResultTemplate !");

			auto *result = electionfunc->get_neutral_election_result();
			for (auto it = county.begin(); it != county.end(); ++it) {
				(*result) += (*electionfunc)((Agent2&)(*this)[*it]);
			}

			result->post_process();
			return result;
		}
		template<class Agent2>
		inline core::election::ElectionResultTemplate* get_election_results(const core::election::ElectionTemplate<Agent2> *electionfunc) const {
			std::vector<size_t> node_lists = nodes();
			return get_election_results(node_lists, electionfunc);
		}
		template<class Agent2>
		inline std::vector<core::election::ElectionResultTemplate*> get_election_results(const std::vector<std::vector<size_t>> &counties, const core::election::ElectionTemplate<Agent2> *electionfunc) const {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by ElectionTemplate in get_election_results !");

			std::vector<core::election::ElectionResultTemplate*> results(counties.size());
			#pragma omp parallel for
			for (size_t i = 0; i < counties.size(); ++i) {
				results[i] = get_election_results(counties[i], electionfunc);
			}
			return results;
		}

		template<class Agent2>
		void inline update_agentwise(const core::agent::AgentWiseUpdateFunctionTemplate<Agent2> *updatefunc) {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by AgentWiseUpdateFunctionTemplate in update_agentwise !");

			#pragma omp parallel for
			for (size_t node = 0; node < num_nodes(); ++node) {
				(*updatefunc)((Agent2&)(*this)[node]);
			}
		}

		template<class Agent2>
		void inline election_retroinfluence(const std::vector<size_t> &county, const core::election::ElectionResultTemplate *election_results, const core::election::ElectionRetroinfluenceTemplate<Agent2> *influencefunc) {
			static_assert(std::is_convertible<Agent, Agent2>::value, "Error: Agent class is not compatible with the one used by ElectionRetroinfluenceTemplate in election_retroinfluence !");

			#pragma omp parallel for
			for (size_t node : county) {
				(*influencefunc)((Agent2&)(*this)[node], election_results);
			}
		}
		template<class Agent2>
		void inline election_retroinfluence(const core::election::ElectionResultTemplate *election_results, const core::election::ElectionRetroinfluenceTemplate<Agent2> *influencefunc) {
			std::vector<size_t> node_lists = nodes();
			return election_retroinfluence(node_lists, election_results, influencefunc);
		}
		template<class Agent2>
		void inline election_retroinfluence(const std::vector<std::vector<size_t>> &counties, const std::vector<core::election::ElectionResultTemplate*> &election_results, const core::election::ElectionRetroinfluenceTemplate<Agent2> *influencefunc) {
			static_assert(std::is_convertible<Agent,Agent2>::value, "Error: Agent class is not compatible with the one used by ElectionRetroinfluenceTemplate in election_retroinfluence !");

			size_t max_2nd_loop_length = counties[0].size();
			for (size_t i = 1; i < counties.size(); ++i) {
				max_2nd_loop_length = std::max(max_2nd_loop_length, counties[1].size());
			}

			#pragma omp parallel for collapse(2)
			for (size_t i = 0; i < counties.size(); ++i) {
				for (size_t j = 0; j < max_2nd_loop_length; ++j) {
					if (j < counties[i].size()) {
						size_t node = counties[i][j];
						(*influencefunc)((Agent2&)(*this)[node], election_results[i]);
					}
	#if !defined(_OPENMP)
					else {
						break;
					}
	#endif
				}
			}
		}
	};
}
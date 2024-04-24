#include <iostream>

#include "src/network.hpp"
#include "src/networks/network_file_io.hpp"
#include "src/networks/network_generator.hpp"
#include "src/networks/network_partition.hpp"
#include "src/networks/network_util.hpp"
#include "implementations/voter_model.hpp"
#include "implementations/voter_model_stuborn.hpp"
#include "implementations/population_voter_model.hpp"
#include "implementations/population_voter_model_stuborn.hpp"
#include "src/agent_population/agent_population.hpp"
#include "util/util.hpp"

int main() {
	auto *test = new SocialNetworkTemplate<voter>(20);
	preferential_attachment(test, 2);

	write_network_to_file(test, "output/test.hdf5");
}
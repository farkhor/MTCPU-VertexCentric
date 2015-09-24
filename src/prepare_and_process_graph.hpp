#ifndef PREPARE_AND_PROCESS_HPP_
#define PREPARE_AND_PROCESS_HPP_

#include <vector>
#include <fstream>

#include "utils/init_graph.hpp"

void prepare_and_process_graph(
		std::vector<initial_vertex>& initGraph,
		const uint nEdges,
		std::ofstream& outputFile,
		std::vector<unsigned int>& indicesRange,
		const int nThreads
		);

#endif /* PREPARE_AND_PROCESS_HPP_ */

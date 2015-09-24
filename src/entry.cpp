#include <string>
#include <stdexcept>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <cstring>
#include <thread>

#include "utils/init_graph.hpp"
#include "utils/read_graph.hpp"
#include "prepare_and_process_graph.hpp"


// Open files safely.
template <typename T_file>
void openFileToAccess( T_file& input_file, std::string file_name ) {
	input_file.open( file_name.c_str() );
	if( !input_file )
		throw std::runtime_error( "Failed to access specified file: " + file_name + "\n" );
}


int main( int argc, char** argv ) {

	const std::string usage =
	"\tRequired command line arguments:\n\
		-Input graph edge list: E.g., --edgelist in.txt\n\
	Additional arguments:\n\
		-Output file (default: out.txt). E.g., --output myout.txt\n\
		-Is the input graph directed (default:yes). To make it undirected: --undirected\n\
		-First column in the edge-list is the indices for source vertices (default: yes). To make the first column standing for the destination vertices --reversedColumns\n\
		-Number of threads (default: queried -- max default is 128). E.g., --nThreads 8.\n\
		-User's arbitrary parameter (default: 0). E.g., --arbparam 17.\n";

	// Required variables for initialization.
	std::ifstream inputEdgeList;
	std::ofstream outputFile;
	bool nonDirectedGraph = false;		// By default, the graph is directed.
	bool firstColumnSourceIndex = true;		// By default, the first column in the graph edge list stand for source vertices.
	long long arbparam = 0;
	unsigned int nThreads = 0;


	/********************************
	 * GETTING INPUT PARAMETERS.
	 ********************************/

	try{

		for( int iii = 1; iii < argc; ++iii )
			if( !strcmp( argv[iii], "--edgelist" ) && iii != argc-1 /*is not the last one*/)
				openFileToAccess< std::ifstream >( inputEdgeList, std::string( argv[iii+1] ) );
			else if( !strcmp( argv[iii], "--output" ) && iii != argc-1 /*is not the last one*/)
				openFileToAccess< std::ofstream >( outputFile, std::string( argv[iii+1] ) );
			else if( !strcmp(argv[iii], "--undirected"))
				nonDirectedGraph = true;
			else if( !strcmp(argv[iii], "--reversedColumns"))
				firstColumnSourceIndex = false;
			else if( !strcmp( argv[iii], "--arbparam" ) && iii != argc-1 /*is not the last one*/)
				arbparam = std::atoll( argv[iii+1] );
			else if( !strcmp( argv[iii], "--nThreads" ) && iii != argc-1 /*is not the last one*/)
				nThreads = std::atoll( argv[iii+1] );


		if( !inputEdgeList.is_open() )
			throw std::runtime_error( "Initialization Error: The input edge list has not been specified." );
		if( !outputFile.is_open() )
			openFileToAccess< std::ofstream >( outputFile, "out.txt" );

	}
	catch( const std::exception& strException ) {
		std::cerr << strException.what() << "\n" << "Usage: " << usage << "\nExiting." << std::endl;
		return( EXIT_FAILURE );
	}
	catch(...) {
		std::cerr << "An exception has occurred." << std::endl;
		return( EXIT_FAILURE );
	}


	try {

		/********************************
		 * Read the input graph.
		 ********************************/

		std::vector<initial_vertex> inMemGraph(0);
		uint nEdges = read_graph_from_file::read_graph(
				inputEdgeList,		// Input file. Expects the graph vertices start with index 0, and they are minimized.
				nonDirectedGraph,		// If the input graph is non-directed, this boolean is true; otherwise it's false.
				firstColumnSourceIndex,		// True if the first column is the source index; otherwise false.
				inMemGraph,		// The read graph.
				arbparam );		// Arbitrary user-provided parameter.
		std::cout << "Input graph collected with " << inMemGraph.size() << " vertices and " << nEdges << " edges." << std::endl;


		/********************************
		 * Determine the interval of vertices assigned to each thread.
		 ********************************/

		if( nThreads == 0 )	// Number of threads have not specified by the user.
			nThreads = std::thread::hardware_concurrency();
		if( nThreads > 128 || nThreads <= 0 )	// Number of threads allowed.
			nThreads = 1;
		std::cout << nThreads << " threads will be processing the graph." << std::endl;

		std::vector<unsigned int> indicesRange( nThreads + 1 );
		indicesRange.at(0) = 0;
		indicesRange.at( indicesRange.size() - 1 ) = inMemGraph.size();
		if( nThreads > 1 ){
			uint approxmiateNumEdgesPerThread = nEdges / nThreads;
			for( unsigned int dev = 1; dev < nThreads; ++dev ) {
				unsigned int accumulatedEdges = 0;
				uint movingVertexIndex = indicesRange.at( dev - 1 );
				while( accumulatedEdges < approxmiateNumEdgesPerThread ) {
					accumulatedEdges += inMemGraph.at( movingVertexIndex ).nbrs.size();
					++movingVertexIndex;
				}
				indicesRange.at( dev ) = movingVertexIndex;
			}
		}


		/********************************
		 * Prepare and process the graph.
		 ********************************/

		prepare_and_process_graph(
				inMemGraph,
				nEdges,
				outputFile,
				indicesRange,
				nThreads );

		std::cout << "Done." << std::endl;
		return( EXIT_SUCCESS );

	}
	catch( const std::exception& strException ) {
		std::cerr << strException.what() << "\n" << "Exiting." << std::endl;
	}
	catch(...) {
		std::cerr << "An exception has occurred." << std::endl;
		return( EXIT_FAILURE );
	}
}

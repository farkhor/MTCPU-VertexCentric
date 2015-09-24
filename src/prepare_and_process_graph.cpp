#include <iostream>
#include <atomic>
#include <chrono>
#include <future>

#include "prepare_and_process_graph.hpp"
#include "utils/buffer.hpp"
#include "user_specified_codes/user_specified_pre_and_post_processing_functions.hpp"
#include "user_specified_codes/user_specified_functions.hpp"

void prepare_and_process_graph(
		std::vector<initial_vertex>& initGraph,
		const uint nEdges,
		std::ofstream& outputFile,
		std::vector<unsigned int>& indicesRange,
		const int nThreads
		) {

	// Get the number of vertices.
	const auto nVertices = initGraph.size();

	/*********************************************************************************
	 * ALLOCATE BUFFERS AND PUT VERTICES INTO BUFFERS OF CSR REPRESENTATION.
	 ********************************************************************************/

	// Allocate buffers.
	buffer< std::atomic< Vertex > > vertexValue( nVertices );
	buffer<unsigned int> edgesIndices( nVertices + 1 );
	edgesIndices.at(0) = 0;
	buffer<unsigned int> vertexIndices( nEdges );
	buffer<Edge> EdgeValue;
	if( sizeof(Edge) > 1 ) EdgeValue.alloc( nEdges );
	buffer<Vertex_static> VertexValueStatic;
	if( sizeof(Vertex_static) > 1 ) VertexValueStatic.alloc( nVertices );

	// Put vertices into buffers of CSR form.
	for( unsigned int vIdx = 0; vIdx < nVertices; ++vIdx ) {
		auto& vvv = initGraph.at(vIdx);
		vertexValue[ vIdx ] = vvv.vertexValue;
		if( sizeof(Vertex_static) > 1 ) VertexValueStatic[ vIdx ] = vvv.VertexValueStatic;
		unsigned int nNbrs = vvv.nbrs.size();
		auto edgeIdxOffset = edgesIndices[ vIdx ];
		for( unsigned int nbrIdx = 0; nbrIdx < nNbrs; ++nbrIdx ) {
			auto& nbr = vvv.nbrs.at( nbrIdx );
			vertexIndices[ edgeIdxOffset + nbrIdx ] = nbr.srcIndex;
			if( sizeof(Edge) > 1 ) EdgeValue[ edgeIdxOffset + nbrIdx ] = nbr.edgeValue;
		}
		edgesIndices[ vIdx + 1 ] = edgeIdxOffset + nNbrs;
	}

	// Free-up some occupied memory.
	initGraph.resize( 0 );

	/*************************************************************************************
	 * PROCESS THE PREPARED GRAPH.
	 *************************************************************************************/

	// Define the function each thread will execute.
	auto threadsProcessingFunction = [&]( const int threadID ) {
		bool didSomeVerticesUpdate = false;
		const auto threadStartVertexIndex = indicesRange.at( threadID );
		const auto threadEndVertexIndex = indicesRange.at( threadID + 1 );
		for( auto vIdx = threadStartVertexIndex; vIdx < threadEndVertexIndex; ++vIdx ) {
			Vertex VertexInHand;
			const Vertex preV = vertexValue[ vIdx ].load( std::memory_order_relaxed );
			init_compute( VertexInHand, preV );
			for( auto eIdx = edgesIndices[ vIdx ]; eIdx < edgesIndices[ vIdx + 1 ]; ++eIdx ) {
				Vertex tmp;
				auto srcVertexIndex = vertexIndices[ eIdx ];
				compute_local(
						vertexValue[ srcVertexIndex ].load( std::memory_order_relaxed ),
						VertexValueStatic.get_ptr() + srcVertexIndex,
						EdgeValue.get_ptr() + eIdx,
						tmp
						);
				compute_reduce( VertexInHand, tmp );
			}
			if( update_condition( VertexInHand, preV ) ) {
				vertexValue[ vIdx ].store( VertexInHand, std::memory_order_relaxed );
				didSomeVerticesUpdate = true;
			}
		}
		return didSomeVerticesUpdate;
	};

	// Set the timer and start.
	using timer = std::chrono::high_resolution_clock;
	unsigned int iterationCounter = 0;
	bool anyUpdate;

	const timer::time_point t1 = timer::now();
	do{
		std::vector< std::future< bool > > jobs( 0 );
		for( auto threadIdx = 0; threadIdx < ( nThreads - 1 ); ++threadIdx )
			jobs.emplace_back( std::async( std::launch::async, threadsProcessingFunction, threadIdx ) );
		anyUpdate = threadsProcessingFunction( nThreads - 1 );	// Launcher thread's own work.
		for( auto& job: jobs )
			anyUpdate |= job.get();
		++iterationCounter;
	} while( anyUpdate == true );
	const timer::time_point t2 = timer::now();

	const auto processing_time = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
	std::cout << "Processing finished in " << static_cast<double>( processing_time ) / 1000.0 << " (ms).\n";
	std::cout << "Performed " << iterationCounter << " iterations in total.\n";

	/*************************************************************************************
	 * PERFORM USER-SPECIFIED OUTPUT FUNCTION.
	 *************************************************************************************/

	// Print the output vertex values to the file.
	for( unsigned int vvv = 0; vvv < nVertices; ++vvv )
		print_vertex_output( vvv, vertexValue[ vvv ], outputFile );

}

#include <string>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <iostream>

#include "read_graph.hpp"
#include "init_graph.hpp"
#include "../user_specified_codes/user_specified_pre_and_post_processing_functions.hpp"


unsigned int read_graph_from_file::read_graph(
		std::ifstream& inFile,
		const bool nondirectedGraph,
		const bool firstColumnSourceIndex,	// true if the first column is the source index.
		std::vector<initial_vertex>& initGraph,
		const long long arbparam ) {

	using indexT = unsigned int;

	std::string line;
	char delim[3] = " \t";	//In most benchmarks, the delimiter is usually the space character or the tab character.
	char* pch;
	unsigned int nEdges = 0;

	unsigned int Additionalargc=0;
	char* Additionalargv[ 61 ];

	// Read the input graph line-by-line.
	while( std::getline( inFile, line ) ) {
		if( line[0] < '0' || line[0] > '9' )	// Skipping any line blank or starting with a character rather than a number.
			continue;
		char cstrLine[256];
		std::strcpy( cstrLine, line.c_str() );
		indexT firstIndex, secondIndex;

		pch = strtok(cstrLine, delim);
		if( pch != NULL )
			firstIndex = static_cast<indexT>( atoll( pch ) );
		else
			continue;
		pch = strtok( NULL, delim );
		if( pch != NULL )
			secondIndex = static_cast<indexT>( atoll( pch ) );
		else
			continue;

		unsigned int theMax = std::max( firstIndex, secondIndex );
		indexT srcIndex = firstColumnSourceIndex ? firstIndex : secondIndex;
		indexT dstIndex = firstColumnSourceIndex ? secondIndex : firstIndex;
		if( initGraph.size() <= theMax )
			initGraph.resize(theMax+1);

		Additionalargc=0;
		Additionalargv[ Additionalargc ] = strtok( NULL, delim );
		while( Additionalargv[ Additionalargc ] != NULL ){
			Additionalargc++;
			Additionalargv[ Additionalargc ] = strtok( NULL, delim );
		}

		auto entryCompletionF = [&]( indexT srcIndex__, indexT dstIndex__ ) {
			neighbor nbrToAdd;
			nbrToAdd.srcIndex = srcIndex__;
			completeEntry(	Additionalargc,
							Additionalargv,
							srcIndex__,
							dstIndex__,
							&(nbrToAdd.edgeValue),
							initGraph.at(srcIndex__).vertexValue,
							&(initGraph.at(srcIndex__).VertexValueStatic),
							initGraph.at(dstIndex__).vertexValue,
							&(initGraph.at(dstIndex__).VertexValueStatic),
							arbparam );
			initGraph.at(dstIndex__).nbrs.push_back( nbrToAdd );
			nEdges++;
		};

		entryCompletionF( srcIndex, dstIndex );
		if( nondirectedGraph ) entryCompletionF( dstIndex, srcIndex );

	}

	return nEdges;

}

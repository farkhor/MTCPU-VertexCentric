#ifndef	USER_SPECIFIED_DEVICE_FUNCTIONS_CUH
#define	USER_SPECIFIED_DEVICE_FUNCTIONS_CUH

#include "user_specified_global_configurations.h"
#include "user_specified_structures.h"



/**************************************
 *  PROCESSING FUNCTIONS
 **************************************/

// This function is executed for each and every vertex at the beginning of an iteration.
inline void init_compute( Vertex& local_V, const Vertex V ) {

#ifdef BFS
	local_V.distance = V.distance;
#endif

#ifdef SSSP
	local_V.distance = V.distance;
#endif

#ifdef PR
	local_V.rank = 0;
#endif

}


// This function is executed for each and every edge.
inline void compute_local(
		Vertex SrcV,	// Source vertex.
		const Vertex_static* SrcV_static,	// Source Vertex_static address. Dereferencing this pointer if it's not defined causes error.
		const Edge* E,	// Edge address. Dereferencing this pointer if it's not defined cause error.
		Vertex& thread_V ) { // Thread's temporary calculation outcome.


#ifdef BFS
	thread_V.distance = SrcV.distance + 1;
#endif

#ifdef SSSP
	thread_V.distance = SrcV.distance + E->weight;
#endif

#ifdef PR
	unsigned int nbrsNum = SrcV_static->NbrsNum;
	thread_V.rank = ( nbrsNum != 0 ) ? ( SrcV.rank / nbrsNum ) : 0;
#endif

}

// Reduction function that is performed for every pair of neighbors of a vertex.
inline void compute_reduce (	Vertex& thread_V, Vertex& next_thread_V	) {

#ifdef BFS
	if ( thread_V.distance > next_thread_V.distance )
		thread_V.distance = next_thread_V.distance;
#endif

#ifdef SSSP
	if ( thread_V.distance > next_thread_V.distance)
		thread_V.distance = next_thread_V.distance;
#endif

#ifdef PR
	thread_V.rank += next_thread_V.rank;
#endif

}

// Below function signals the caller (and consequently the launcher thread) if the vertex content should be replaced with the newly calculated value.
inline bool update_condition (	Vertex& computed_V, const Vertex previous_V	) {

#ifdef BFS
	return ( computed_V.distance < previous_V.distance );
#endif

#ifdef SSSP
	return ( computed_V.distance < previous_V.distance );
#endif

#ifdef PR
	computed_V.rank = (1-PR_DAMPING_FACTOR) + computed_V.rank * PR_DAMPING_FACTOR;	// Or you can replace this expression by fused multiply-add.
	return ( fabs( computed_V.rank - previous_V.rank) > PR_TOLERANCE );
#endif

}


#endif	// USER_SPECIFIED_DEVICE_FUNCTIONS_CUH

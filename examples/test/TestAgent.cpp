/*
 * TestAgent.cpp
 *
 *  Created on: Oct 25, 2014
 *      Author: Jay
 */

#include "TestAgent.h"

extern "C" Agent* instantiate( void *arguments ) {
  return new TestAgent( arguments );
}

extern "C" void destroy( Agent *object ) {
  delete object;
}

void * TestAgent::init( void* argument ) {
	migratableDataSize = sizeof(double) + sizeof(int);
	migratableData = malloc(migratableDataSize);
	val = (double*)migratableData;
	//	iters = (int*)(migratableData+sizeof(double));
	iters = (int*)((double*)migratableData + 1);
	*iters = *(int*) argument;
	*val = 1.0;
	return NULL;
}

void * TestAgent::report( void *argument ) {
	ostringstream convert;
	convert << "Agent #(" << agentId << ") returning index of "
			<< index.at(0) << "," << index.at(1) << endl;
	MASS_base::log( convert.str( ) );
	int *retval = new int[2];
	retval[0]=index.at(0);
	retval[1]=index.at(1);
		//double * retval = new double(*val);
	return retval;
}

void * TestAgent::call_all( void *argument ) {
	for (int i = 0; i < *iters; i++) {
		*val *= 1.2;
	}
	return NULL;
}

void * TestAgent::call_all_return( void *argument ) {
	for (int i = 0; i < *iters; i++) {
		*val *= 1.2;
	}
	int *retval = new int(agentId);
	//double * retval = new double(*val);
	return retval;
}

//Best case migration...nothing moves!
void * TestAgent::best_migrate( void *argument ) {
  index[0] = index.at(0); //0;
  index[1] = index.at(1); //0;
  index[0] = 0;
  index[1] = 0;
	migrate(index);
	return NULL;
}

void * TestAgent::random_migrate( void *argument ) {
	int *size=(int*)argument;
	int rand_x = rand() % *size;
	int rand_y = rand() % *size;

	vector<int> new_coords;
	new_coords.push_back(rand_x);
	new_coords.push_back(rand_y);
	migrate(new_coords);
	return NULL;
}

void * TestAgent::worst_migrate( void *argument ) {
	int *args = (int*)argument;
	int size = args[0];
	int hosts = args[1];
	int offset = size / hosts;

	vector<int> new_coords;
	int new_x;
	if (index[0] > offset * (hosts - 1) - 1) {
		new_x = (index[0] + offset) % size;
	} else {
		new_x = (index[0] + offset);
	}
	new_coords.push_back(new_x 	);
	new_coords.push_back(index[1]);
	migrate(new_coords);

	return NULL;
}


/*
 * TestPlace.cpp
 *
 *  Created on: Jun 25, 2014
 *      Author: jay
 */

#include "TestPlace.h"

extern "C" Place* instantiate( void *arguments ) {
  return new TestPlace( arguments );
}

extern "C" void destroy( Place *object ) {
  delete object;
}

void * TestPlace::init( void* argument ) {
	inMessage_size = sizeof( double );
	outMessage_size = sizeof(double);
	iters = *(int*) argument;
	val = 1.0;
	outMessage = new double(val);
	x_neighbors[0] = NULL; x_neighbors[1] = NULL;
	y_neighbors[0] = NULL; y_neighbors[1] = NULL;
	return NULL;
}

void * TestPlace::call_all( void *argument ) {
	for (int i = 0; i < iters; i++) {
		val *= 1.2;
	}
	inMessages.clear();
	return NULL;
}

void * TestPlace::call_all_ex_b( void *argument ) {
	*(double*)outMessage = val;
	return NULL;
}

void * TestPlace::exchange_all( void *argument ) {
	double * retval = new double();
	*retval = val;
	return retval;
}
void * TestPlace::call_all_param( void* argument ) {
	double * retval = (double*)argument;
	*retval = *(int*)argument * 1.2;
	return retval;
}
void * TestPlace::call_all_param_return( void* argument ) {
	for (int i = 0; i < iters; i++) {
			val *= 1.2;
	}
	double * retval = new double(val);
	return retval;
}

void * TestPlace::ex_bound( void* argument ) {
	int directions[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
	for (int i = 0; i < 4; i++) {
		void * msg = getOutMessage(1, directions[i]);
		if (msg != NULL) {
			//	convert << *(double*)msg << " ";
			if (i / 2 == 0) {
				x_neighbors[i] = (double*)msg;
			} else {
				y_neighbors[i%2] = (double*)msg;
			}
		}
	}
	for (int i = 0; i < 2; i++) {
		if(y_neighbors[i] == NULL) {
			y_neighbors[i] = &val;
		}
		if (x_neighbors[i] == NULL) {
			x_neighbors[i] = &val;
		}
	}
	return NULL;
}


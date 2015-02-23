/*
 * TestPlace.h
 *
 *  Created on: Jun 25, 2014
 *      Author: jay
 */

#ifndef TESTPLACE_H_
#define TESTPLACE_H_

#include "Place.h"
#include <iostream>
#include <sstream>
#include "MASS_base.h"
#include <stdlib.h>

class TestPlace : public Place {
public:

	static const int call_all_ = 1;
	static const int exchange_all_ = 2;
	static const int call_all_param_ = 3;
	static const int call_all_param_return_ = 4;
	static const int call_all_ex_b_ = 5;
	static const int ex_bound_ = 6;
	static const int init_ = 7;

	TestPlace(void * args): Place( args ) {iters = 0; val = 1.0;};

	virtual void *callMethod( int functionId, void *argument ) {
	    switch( functionId ) {
	    case call_all_: return call_all( argument );
	    case call_all_ex_b_: return call_all_ex_b( argument );
	    case exchange_all_: return exchange_all( argument );
	    case call_all_param_: return call_all_param(argument);
	    case call_all_param_return_: return call_all_param_return(argument);
	    case ex_bound_: return ex_bound(argument);
	    case init_: return init(argument);
	    }
	    return NULL;
	  };
private:
	void *init(void *argument);
	void *call_all( void *argument );
	void *call_all_ex_b( void* argument); // uses exchange boundary improvements
	void *exchange_all( void *argument );
	void *call_all_param( void* argument );
	void *call_all_param_return( void* argument );
	void *ex_bound(void* argument);
	int iters;
	double val;
	double* x_neighbors[2];
	double* y_neighbors[2];
};

#endif /* TESTPLACE_H_ */

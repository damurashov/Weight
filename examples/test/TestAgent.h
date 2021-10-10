/*
 * TestAgent.h
 *
 *  Created on: Oct 25, 2014
 *      Author: Jay
 */

#ifndef TESTAGENT_H_
#define TESTAGENT_H_

#include "Agent.h"
#include <iostream>
#include <sstream>
#include "MASS_base.h"
#include <stdlib.h>

class TestAgent : public Agent{
public:

	static const int call_all_ = 1;
	static const int call_all_return_=2;
	static const int best_migrate_ = 3;
	static const int random_migrate_ = 4;
	static const int worst_migrate_ = 5;
	static const int init_ = 6;
	static const int report_ = 7;

	TestAgent(void * args): Agent( args ) {val = NULL; iters = NULL;};

	virtual void *callMethod( int functionId, void *argument ) {
	    switch( functionId ) {
	    case call_all_: return call_all( argument );
	    case call_all_return_: return call_all_return( argument );
	    case best_migrate_: return best_migrate ( argument );
	    case random_migrate_: return random_migrate( argument );
	    case worst_migrate_: return worst_migrate( argument );
	    case init_: return init(argument);
	    case report_: return report(argument);
	    }
	    return NULL;
	  };
private:
	double* val;
	int* iters;
	void *call_all( void *argument );
	void *call_all_return(void *argument );
	void *best_migrate(void *argument );
	void *random_migrate( void *argument );
	void *worst_migrate( void *argument );
	void *init(void *argument);
	void *report(void *argument);
};

#endif /* TESTAGENT_H_ */

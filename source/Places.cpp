/*
 MASS C++ Software License
 © 2014-2015 University of Washington
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 The following acknowledgment shall be used where appropriate in publications, presentations, etc.:
 © 2014-2015 University of Washington. MASS was developed by Computing and Software Systems at University of Washington Bothell.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include "Places.h"
#include "MASS.h"
#include "Message.h"
#include "Mthread.h"
#include <iostream>
#include <dlfcn.h> // dlopen, dlsym, and dlclose

//Used to toggle comments from Places.cpp
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

Places::Places(int handle, string className, void *argument,
	int argument_size, int dim, ...)
	: Places_base(handle, className, 0, argument, argument_size, dim, NULL) {

	size = new int[dim];
	// Extract each dimension's length
	va_list list;
	va_start(list, dim);

	for (int i = 0; i < dim; i++) {
		size[i] = va_arg(list, int);
	}
	va_end(list);

	init_all(argument, argument_size); // explicitly call Places_base.init_all
	init_master(argument, argument_size, 0);
}

Places::Places(int handle, string className, void *argument,
	int argument_size, int dim, int size[])
	: Places_base(handle, className, 0, argument, argument_size, dim, size) {

	// init_all called within Places_base
	init_master(argument, argument_size, 0);
}

Places::Places(int handle, string className, int boundary_width,
	void *argument, int argument_size, int dim, ...)
	: Places_base(handle, className, boundary_width, argument, argument_size,
		dim, NULL) {

	size = new int[dim];
	// Extract each dimension's length
	va_list list;
	va_start(list, dim);

	for (int i = 0; i < dim; i++) {
		size[i] = va_arg(list, int);
	}
	va_end(list);

	init_all(argument, argument_size); // explicitly call Places_base.init_all
	init_master(argument, argument_size, boundary_width);
}

Places::Places(int handle, string className, int boundary_width,
	void *argument, int argument_size, int dim, int size[])
	: Places_base(handle, className, boundary_width, argument, argument_size,
		dim, size) {

	// init_all called within Places_base
	init_master(argument, argument_size, boundary_width);
}

void Places::init_master(void *argument, int argument_size,
	int boundary_width) {

	// convert size[dimension] to vector<int>
	vector<int> *size_vector = new vector<int>(dimension);
	size_vector->assign(size, size + dimension);

	// create a list of all host names;  
	// the master IP name
	char localhost[100];
	bzero(localhost, 100);
	gethostname(localhost, 100);
	vector<string> hosts;
	hosts.push_back(*(new string(localhost)));

	// all the slave IP names
	for (int i = 0; i < int(MASS::mNodes.size()); i++) {
		hosts.push_back(MASS::mNodes[i]->getHostName());
	}

	// create a new list for message
	vector<string> *tmp_hosts = new vector<string>(hosts);


	Message *m = new Message(Message::PLACES_INITIALIZE, size_vector,
		handle, className,
		argument, argument_size, boundary_width,
		tmp_hosts);

	// send a PLACES_INITIALIZE message to each slave
	for (int i = 0; i < int(MASS::mNodes.size()); i++) {
		MASS::mNodes[i]->sendMessage(m);

		if (printOutput == true) {
			cerr << "PLACES_INITIALIZE sent to " << i << endl;
		}
	}
	delete m;

	// establish all inter-node connections within setHosts( )
	MASS_base::setHosts(hosts);

	// register this places in the places hash map
	MASS_base::placesMap.
		insert(map<int, Places_base*>::value_type(handle, this));

	// Synchronized with all slave processes
	MASS::barrier_all_slaves();
}

void Places::callAll(int functionId) {
	ca_setup(functionId, NULL, 0, 0, Message::PLACES_CALL_ALL_VOID_OBJECT);
}

void Places::callAll(int functionId, void *argument, int arg_size) {

	if (printOutput == true) {
		cerr << "callAll void object" << endl;
	}

	ca_setup(functionId, argument, arg_size, 0, // ret_size = 0
		Message::PLACES_CALL_ALL_VOID_OBJECT);
}

void *Places::callAll(int functionId, void *argument[], int arg_size,
	int ret_size) {

	if (printOutput == true) {
		cerr << "callAll return object" << endl;
	}

	return ca_setup(functionId, (void *)argument, arg_size, ret_size,
		Message::PLACES_CALL_ALL_RETURN_OBJECT);
}

void *Places::ca_setup(int functionId, void *argument,
	int arg_size, int ret_size,
	Message::ACTION_TYPE type) {
	// calculate the total argument size for return-objects
	int total = 1; // the total number of place elements
	for (int i = 0; i < dimension; i++)
		total *= size[i];
	int stripe = total / MASS_base::systemSize;

	// send a PLACES_CALLALL message to each slave
	Message *m = NULL;
	for (int i = 0; i < int(MASS::mNodes.size()); i++) {
		// create a message
		if (type == Message::PLACES_CALL_ALL_VOID_OBJECT)
			m = new Message(type, this->handle, functionId, argument, arg_size,
				ret_size);
		else { // PLACES_CALL_ALL_RETURN_OBJECT
			m = new Message(type, this->handle, functionId,
				// argument body
				(char *)argument + arg_size * stripe * (i + 1),
				// argument size
				(i == int(MASS::mNodes.size()) - 1) ?
				arg_size * (total - stripe * (i + 1)) : // + rmdr
				arg_size * stripe,
				ret_size); // no remainder   

			if (printOutput == true) {
				cerr << "Places.callAll: arg_size = " << arg_size
					<< " stripe = " << stripe << " i + 1 = " << (i + 1) << endl;
			}
			/*
			int *data = (int *)((char *)argument + arg_size * stripe * ( i + 1 ));
			for ( int i = 0; i < stripe; i++ )
		  cerr << *(data + i) << endl;
			*/
		}

		// send it
		MASS::mNodes[i]->sendMessage(m);

		if (printOutput == true) {
			cerr << "PLACES_CALL_ALL " << m->getAction() << " sent to " << i << endl;
		}

		// make sure to delete it
		delete m;
	}

	// retrieve the corresponding places
	MASS_base::currentPlaces = this;
	MASS_base::currentFunctionId = functionId;
	MASS_base::currentArgument = argument;
	MASS_base::currentArgSize = arg_size;
	MASS_base::currentMsgType = type;
	MASS_base::currentRetSize = ret_size;
	MASS_base::currentReturns =
		(type == Message::PLACES_CALL_ALL_VOID_OBJECT) ?
		NULL :
		new char[total * MASS_base::currentRetSize]; // prepare an entire return space

	  // resume threads
	Mthread::resumeThreads(Mthread::STATUS_CALLALL);

	// callall implementation
	if (type == Message::PLACES_CALL_ALL_VOID_OBJECT)
		Places_base::callAll(functionId, argument, 0); // 0 = the main thread id
	else
		Places_base::callAll(functionId, (void *)argument, arg_size, ret_size, 0);

	// confirm all threads are done with callAll.
	Mthread::barrierThreads(0);

	// Synchronized with all slave processes
	if (type == Message::PLACES_CALL_ALL_VOID_OBJECT)
		MASS::barrier_all_slaves(MASS_base::currentReturns, stripe,
			MASS_base::currentRetSize);
	else
		MASS::barrier_all_slaves(MASS_base::currentReturns, stripe,
			MASS_base::currentRetSize);

	return (void *)MASS_base::currentReturns;
}

void Places::exchangeAll(int dest_handle, int functionId,
	vector<int*> *destinations) {

	// send a PLACES_EXCHANGE_ALL message to each slave
	Message *m = new Message(Message::PLACES_EXCHANGE_ALL,
		this->handle, dest_handle, functionId,
		destinations, this->dimension);

	if (printOutput == true) {
		cerr << "dest_handle = " << dest_handle << endl;
	}

	for (int i = 0; i < int(MASS::mNodes.size()); i++) {
		MASS::mNodes[i]->sendMessage(m);
	}
	delete m;

	// retrieve the corresponding places
	MASS_base::currentPlaces = this;
	MASS_base::destinationPlaces = MASS_base::placesMap[dest_handle];
	MASS_base::currentFunctionId = functionId;
	MASS_base::currentDestinations = destinations;

	// reset requestCounter by the main thread
	MASS_base::requestCounter = 0;

	// for debug
	MASS_base::showHosts();

	// resume threads
	Mthread::resumeThreads(Mthread::STATUS_EXCHANGEALL);

	// exchangeall implementation
	Places_base::exchangeAll(MASS_base::destinationPlaces,
		functionId,
		MASS_base::currentDestinations, 0);

	// confirm all threads are done with exchangeAll.
	Mthread::barrierThreads(0);

	// Synchronized with all slave processes
	MASS::barrier_all_slaves();
}

void Places::exchangeBoundary() {

	// send a PLACES_EXCHANGE_BOUNDARY message to each slave
	Message *m = new Message(Message::PLACES_EXCHANGE_BOUNDARY, this->handle,
		0); // 0 is dummy

	for (int i = 0; i < int(MASS::mNodes.size()); i++) {
		MASS::mNodes[i]->sendMessage(m);
	}
	delete m;

	// retrieve the corresponding places
	MASS_base::currentPlaces = this;

	// for debug
	MASS_base::showHosts();

	// exchange boundary implementation
	Places_base::exchangeBoundary();

	// Synchronized with all slave processes
	MASS::barrier_all_slaves();
}

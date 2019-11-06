![logo.png](https://bitbucket.org/repo/AEx9kp/images/1314816353-logo.png)

# MASS C++

Multi-Agent Spatial Simulation Library

- [Introduction](#markdown-header-introduction)
- [Programming Model](#markdown-header-programming-model)
	- [Abstractions: Places and Agents](#markdown-header-abstractions-places-and-agents)
	- [Programming Framework](#markdown-header-programming-framework)
- [Documentation](#markdown-header-documentation)
	- [MASS](#markdown-header-mass)
	- [Places](#markdown-header-places)
		- [Places Class](#markdown-header-places-class)
		- [Place Class](#markdown-header-place-class)
		- [Example of a Place-Derived Class](#markdown-header-example-of-a-place-derived-class)
	- [Agents](#markdown-header-agents)
		- [Agents Class](#markdown-header-agents-class)
		- [Agent Class](#markdown-header-agent-class)
		- [Example of an Agent-Derived Class](#markdown-header-example-of-a-agent-derived-class)
	- [Setup and Use](#markdown-header-setup-and-use)
		- [Compilation](#markdown-header-compilation)
			- [Application Directory Setup](#markdown-header-application-directory-setup)
		- [Installation](#markdown-header-installation)
		- [Abnormal Termination Clean-up](#markdown-header-abnormal-termination-clean-up)
	- [Outputs and Logging](#markdown-header-outputs-and-logging)


## Introduction

MASS is an open source library for parallelizing [agent-based simulations](https://en.wikipedia.org/wiki/Agent-based_model). MASS is primarily intended to parallelize simulations, primarily focusing on multi-entity interaction in physical, biological, social, and strategic domains. Example use cases include problems in physics (including molecular dynamics, Schrodinger's wave equation, and Fourier's heat equation), neural networks and urban planning simulations.

## Programming Model

### Abstractions: Places and Agents

The concepts of [Places](#markdown-header-places) and [Agents](#markdown-header-agents) are key to the MASS library.

Places are a matrix of elements (each element being a Place) that are dynamically allocated over a cluster of computing nodes. Places represent an abstraction of the execution environment in which Agents can exist, capable of holding thier own state and exchanging information with other places.

Agents are a set of stateful execution instances that can reside on a Place, migrate to any other Place within the Places matrix, spawn new Agents, and interact with other Agents.

### Programming Framework

The following code shows a C++ program that uses the MASS library to simulate a simple multi-agent spatial simulation:

```cpp
#include "MASS.h"
#include "Territory.h"
#include "Troop.h"
#include <vector>
#define MSG "argument\0"

int main(int argc, char* argv[]) {
	char* arguments[4];
	arguments[0] = argv[1]; 					// username
	arguments[1] = argv[2]; 					// password
	arguments[2] = argv[3]; 					// machinefile
	arguments[3] = argv[4]; 					// port
	int nProcesses = atoi(argv[5]); 			// # of processes
	int nThreads = atoi(argv[6]); 				// # of threads per process

	// start a process at each computing node
	MASS::init(arguments, nProcesses, nThreads);

	// create some variable for clarity
	char* msg = MSG;							// a starting outbound message
	int territoriesHandle = 1; 					// a handle for the territory class
	int troopsHandle = 2; 						// a handle for the troop class
	int spatialDimensions = 2; 					// the number of dimensions
	int xSize = 100; 							// the size of the x dimension
	int ySize = 100; 							// the size of the y dimension
	int numAgents = 4000; 						// the starting number of agents
	
	// distribute places and agents over computing nodes
	Places* territories = new Places(territoriesHandle, "Territory", msg, sizeof(MSG), spatialDimensions, xSize, ySize);
	Agents* troops = new Agents(troopsHandle, "Troop", msg, sizeof(MSG), territories, numAgents);

	// set up the directions
	vector<int*> directions;
	int north[2] = {0, 1}; directions.push_back(north);
	int east[2] = {1, 0}; directions.push_back(east);
	int south[2] = {0, -1}; directions.push_back(south);
	int west[2] = {-1, 0}; directions.push_back(west);

	int totalTime = 2000; 						// set up the total time
	for (int currentTime = 0; currentTime < totalTime; currentTime++) {

		// call some custom function on the territories
		territories->callAll(Territory::someCustomFunction_, (void*) &currentTime, sizeof(currentTime));

		// call a custom data exchange function on the territories
		// directions could be used here to define neighboring territories 
		// to exchange with from main
		territories->exchangeAll(Territory::someCustomExchange_, &directions);

		// call some custom function on all the agents
		troops->callAll(Troop::someCustomFunction_, (void*) &currentTime, sizeof(currentTime));

		// have troops migrate based on state that was set during the custom function
		troops->manageAll();
	}

	MASS::finish();
}
```

## Documentation

### MASS

The MASS class is the main entry point to the library. All programs using the MASS library must initialize with MASS::init(...) and end with MASS::finish().

##### Methods #####
```cpp
static void init(char* args[], int nProcesses, int nThread);
```
- Initializes the MASS library remote processes. Expected args = [username, password, machinefile, port]. nProcess specifies the number of processes to launch (including remote and master). nProcess should be equal to the number of lines in machinefile. nThread specifies the number of threads to use on each process.

```cpp
static void finish();
```
- Called to clean-up remote processes at the end of computation.

### Places

#### Places Class ####

Places is a distributed matrix of Place objects. Each Place is addressed by a set of network-independent matrix indices.

##### Constructors #####
```cpp
Places(int handle, string className, void* argument, int argument_size, int dimensions, ...);
```
- Constructs a new instance of Places on the initialized MASS processes. Handle should be unique across all machines and between all Agents and Places instances. Class Name should be the name of a user defined class that extends Place. Argument is an argument that is passed to each instance of the initialized class in it's constructor. Agument_size defines the size of the Argument. Dimensions defines the dimensionality of the matrix constructed (1, 2, or 3) and each integer thereafter defines the size of the matrix in the corresponding dimension; for example a call ending in 2, 100, 50 would produce a matrix of size \[100][50].

```cpp
Places(int handle, string className, void* argument, int argument_size, int dimensions, int size[]);
```
- Constructs a new instance of Places in the same way as above, except that the size in each dimension is defined in an array. A call corresponding to the above one ending in 2, 100, 50 would end with 2, {100, 50}.

```cpp
Places(int handle, string className, int boundary_width, void* argument, int argument_size, int dimensions, ...);
```
- Constructs a new instance of Places with a custom boundary width. The boundry is the shadow space on each process representing the state of remote places since the last call to exchangeBoundry(). Normally MASS splits the matrix along the x direction between nodes and gives each node a boundary width of 1 on each side. For example, if you created a new two dimensional instance of Places with a size 50 by 25 on 2 nodes, each node would have a 25 by 25 matrix of Place objects that it is responsible for, but would keep a shadow matrix of 1 by 25 Place objects representing the state of the boundry Place objects on the other node. If the same situation occurred with a boundary width of 2, the shadow matrix would have a size of 2 by 25.

```cpp
Places(int handle, string className, int boundary_width, void* argument, int argument_size, int dimensions, int size[]);
```
- Constructs a new instance of Places with a custom boundary width and size based on an array.

##### Methods #####
```cpp
void callAll(int functionId);
```
- Calls the method specified with functionId on all Place elements in this Places instance. This is done in parallel among multi-processes/threads.

```cpp
void callAll(int functionId, void* argument, int arg_size);
```
- Same as above except the argument is passed to each element. Arg_size specifies the size of the argument being used.

```cpp
void* callAll(int functionId, void* argument[], int arg_size, int ret_size);
```
- Same as above except each Place recieves a unique argument specified by argument[i] where i is the index when the array is flattened into a single dimension. This also receives a return value from each Place element into (void*)[i] where the size of each element is return_size. 

```cpp
void exchangeAll(int dest_handle, int functionId, vector<int*>* destinations);
```
- Calls 


```cpp
void exchangeBoundry();
```
- Exchange the state of the boundary Place isntances with the left and right neighboring nodes.

#### Place Class ####

Place is the abstract class from which a user can derive his/her application-specific matrix, Places.

##### Constructors #####

```cpp
Place(void* args);
```
- The default constructor, 

```cpp
vector<int> size;
```
- Defines the size of the Places matrix in which this Place object resides. Size[0], size[1], and size[2] correspond to the size in the x, y, and z dimensions.

```cpp
vector<int> index;
```
- Defines this places coordinates in the Places matrix in which this Place object resides. Index[0], index[1], and index[2] correspond to the coordinates in the x, y and z dimensions.

```cpp
vector<MObject*> agents;
```
- A vector with references to all Agent objects residing on this Place.

```cpp
void* outMessage;
```
- Stores a set of arguments to be passed to remote-cell functions that will be invoked by Places.exchangeAll(). The argument size should be specified in the data field Place.outMessage_size.

```cpp
int outMessage_size;
```
- Defines the current size of outMessage.

```cpp
vector<void*> inMessages;
```
- Stores return values from calls made vi

```cpp
int inMessage_size;
```
- Defines the current size of inMessage.

```cpp
virtual void* callMethod(int functionId, void* arguments);
```
- A method to be defined by the user in a Place-derived class. This method is called by Places.callAll() and Places.exchangeAll().

#### Example of a Place-Derived Class ####

An application-specific Place-derived class enables users to define custom functions to be called in parallel by defining identifiers for functions along with a callMethod function that takes in a function identifier and calls the correct internal function. Additionally note that because the user defined program dynamically links to the MASS library, the user must define instantiate() and destroy() C functions to be used by dlopen() and dlsym(). 

The following is a simple example of a Place-Derived class:

```cpp
#ifndef DERIVED_PLACE_H
#define DERIVED_PLACE_H

#include <iostream>
#include "Place.h"

class DerivedPlace : public Place {
public:
	// set up function ids
	static const int customFunction_ = 0;
	static const int otherCustomFunction_ = 1;

	// constructor
	DerivedPlace(void* argument) : Place(argument) {
		// set some internal state
	}

	// define a switch statement used by callAll
	virtual void* callMethod(int functionId, void* argument) {
		switch(functionId) {
			case customFunction_: return customFunction(argument);
			case otherCustomFunction_ : return otherCustomFunction();
		}
		return NULL;
	};

private:
	// define custom functions
	void* customFunction(void* argument) {
		// do something
		return NULL;
	};

	void* otherCustomFunction() {
		// do something else
		return NULL;
	};
};

#endif

extern "C" Place* instantiate(void* argument) {
	return new DerivedPlace(argument);
}

extern "C" void destroy(Place* object) {
	delete object;
}
```

This example class could be used by the following example driver:
```cpp
#include "MASS.h"
#include "DerivedPlace.h"

int main(int argc, char* argv[]) {
	int nProc = 4, nThr = 4; 			// define the number of processes and threads
	MASS::init(argv, nProc, nThr);		// initialize the MASS library

	// initialize places with instances of our derived class
	Places* places = new Places(1, "DerivedPlace", "args", 4, 2, 100, 100);

	// Call the customFunction on each instance of DerivedPlace
	places->callAll(DerivedPlace::customFunction_, "message", 7);

	// Clean up MASS
	MASS::finish();
}
```


### Agents

#### Agents Class ####

```cpp
Agents(int handle, string className, void* argument, int argument_size, Places* places, int initialPopulation);
```
- Constructs a new instance of Agents on the initialized Places. Handle should be unique across all machines and between all Agents and Places instances. Class Name should be the name of a user defined class that extends Agent. Argument is an argument that is passed to each instance of the initialized user-defined class in it's constructor. Argument Size defines the size of the argument. Places is a pointer to the Places instance over which the Agent objects should be allocated. initPopulation defines the initial population of the agents.

```cpp
void callAll(int functionId);
```
- Calls the method specified with functionId on all Agent objects referenced by this Agents instance. This is done in parallel among multi-processes/threads.

```cpp
void callAll(int functionId, void* argument, int arg_size);
```
- Same as above except the argument is passed to each element. Arg size specifies the size of the argument.

```cpp
void* callAll(int functionId, void* argument, int arg_size, int ret_size);
```
<!--- TODO: Better Explain how to access return values -->
- Same as above except each Agent returns a value accessable via (void*)[i]. Ret Size defines the size of the return value.

```cpp
void manageAll();
```
- 

#### Agent Class ####

#### Example of an Agent-Derived Class ####

### Setup and Use

#### Compilation ####

##### Application Directory Setup #####

#### Installation ####

#### Abnormal Termination Clean-up ####

#### Outputs and Logging ####

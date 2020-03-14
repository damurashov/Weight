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

#ifndef DLLCLASS_H
#define DLLCLASS_H

#include "Place.h"
#include "Agent.h"
#include <string>  // string
#include <dlfcn.h> // dlopen dlsym dlclose

using namespace std;

class DllClass {
public:
	void *stub;
	instantiate_t *instantiate;
	destroy_t *destroy;
	Place **places;          // an array of places
	Place **left_shadow;     // the shadow of the left neigbhor's boundary
	Place **right_shadow;    // the shadown of the right neighbor's boundary
	vector<Agent*> *agents;   // a bag of agents
	vector<Agent*> *retBag;   // a temporary bag of agents
	int agentCounter; // count # of unevaluated agents in vector<Agent*> *agents. Used in migrationHelperCollisionFree

	DllClass(string className);
};

#endif

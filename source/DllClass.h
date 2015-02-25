#ifndef DLLCLASS_H
#define DLLCLASS_H

#include "Place.h"
#include "Agent.h"
#include <string>  // string
#include <dlfcn.h> // dlopen dlsym dlclose

using namespace std;

class DllClass {
 public:
  void *stub;                 // actual pointer to class method to link
  instantiate_t *instantiate; // pointer to class constructor
  destroy_t *destroy;         // pointer to class destructor
  Place **places;             // an array of places
  Place **left_shadow;        // the shadow of the left neigbhor's boundary
  Place **right_shadow;       // the shadow of the right neighbor's boundary
  vector<Agent*> *agents;     // a bag of agents
  vector<Agent*> *retBag;     // a temporary bag of agents

  DllClass( string className );
};

#endif

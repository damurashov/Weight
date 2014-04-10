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
  vector<Agent*> *agents;   // a bag of agents

  DllClass( string className );
};

#endif

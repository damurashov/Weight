#include <iostream>
#include <dlfcn.h>
#include "Place.h"

using namespace std;

int main( ) {
  while ( true ) {
    string libraryName;
    cout << "library name = ";
    cin >> libraryName;
    void* stub; // an opaque handle

      if ( ( stub = dlopen( libraryName.c_str( ), RTLD_LAZY ) ) == NULL ) {
	cerr << "library: " << libraryName << " not found" << endl;
	continue;
      }

      instantiate_t* instantiate_myclass 
	= (instantiate_t *)dlsym( stub, "instantiate" );
      destroy_t* destroy_object = (destroy_t *) dlsym( stub, "destroy" );

      Place* object = instantiate_myclass( NULL );

      object->callAll( 0, NULL );
      destroy_object( object );
      dlclose( stub );
  }

}

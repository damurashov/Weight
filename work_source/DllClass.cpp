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

#include "DllClass.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output for DllClass
const bool printOutput = false;
//const bool printOutput = true;

DllClass::DllClass( string className ) {
  // For debugging
  ostringstream convert;

  // Create "./className"
  int char_len = 2 + className.size( ) + 1;
  char dot_className[char_len];
  bzero( dot_className, char_len );
  strncpy( dot_className, "./", 2 );
  strncat( dot_className, className.c_str( ), className.size( ) );

  agents = new vector<Agent*>;

  // load a given class
  if ( ( stub = dlopen( dot_className, RTLD_LAZY ) ) == NULL ) {
    if(printOutput == true){
        convert.str( "" );
        convert << "class: " << dot_className << " not found" << endl;
        MASS_base::log( convert.str( ) );
    }
    exit( -1 );
  }

  // register the object instantiation/destroy functions
  instantiate = ( instantiate_t * )dlsym( stub, "instantiate" ); 
  destroy = ( destroy_t * )dlsym( stub, "destroy" );
}


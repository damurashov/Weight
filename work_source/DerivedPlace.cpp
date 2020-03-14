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

#ifndef DERIVEDPLACE_H
#define DERIVEDPLACE_H

#include <iostream>
#include "Place.h"
#define FUNCTION 0 // FUNCTION ID

class DerivedPlace : public Place {
public:
  // 1: CONSTRUCTOR DESIGN
  DerivedPlace( void *argument ) : Place( argument ) {
    // START OF USER IMPLEMENTATION 
    // END OF USER IMPLEMENTATION   
  }

  // 2: CALLALL DESIGN
  virtual void *callAll( int functionId, void *argument ) {
    switch( functionId ) {
      // START OF USER IMPLEMENTATION
    case FUNCTION: return function( argument );
      // END OF USER IMPLEMNTATION
    }
    return NULL;
  };

private:
  // 3: EACH FUNCTION DESIGN
  // START OF USER IMPLEMENTATION
  void *function( void *argument ) {
    return NULL;
  }
  // END OF USER IMPLEMENTATION
};

#endif

extern "C" Place* instantiate( void *argument ) {
  return new DerivedPlace( argument );
}

extern "C" void destroy( Place *object ) {
  delete object;
}


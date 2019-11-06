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

#ifndef NOMAD_H
#define NOMAD_H

#include "Agent.h"
#include "MASS_base.h"
#include <sstream> // ostringstream

//Used to toggle output in Nomad.h
const bool printOut = false;
//const bool printOut = true;

using namespace std;

class Nomad : public Agent {

 public:
  static const int agentInit_ = 0;
  static const int somethingFun_ = 1;
  static const int createChild_ = 2;
  static const int killMe_ = 3;
  static const int move_ = 4;
  static const int callalltest_ = 5;
  static const int addData_ = 6;
  static const int move2_ = 7;
  
  Nomad( void *argument ) : Agent( argument ) {
    if(printOut == true)
        MASS_base::log( "BORN!!" );
  };

  virtual void *callMethod( int functionId, void *argument ) {
    switch( functionId ) {
    case agentInit_: return agentInit( argument );
    case somethingFun_: return somethingFun( argument );
    case createChild_: return createChild( argument );
    case killMe_: return killMe( argument );
    case move_: return move( argument );
    case callalltest_: return callalltest( argument );
    case addData_: return addData( argument );
    case move2_: return move2( argument );
    }
    return NULL;
  };

 private:
  void *agentInit( void *argument );
  void *somethingFun( void *argument );
  void *createChild( void *argument );
  void *killMe( void *argument );
  void *move( void *argument );
  void *callalltest( void *argument );
  void *addData( void *argument );
  void *move2( void *argument );
};

#endif

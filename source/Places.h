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

#ifndef PLACES_H
#define PLACES_H

#include <string>
#include <stdarg.h>  // int size, ...
#include "Places_base.h"
#include "Message.h"

using namespace std;

class Places: public Places_base {
public:
  Places( int handle, string className, void *argument, int argument_size,
      int dim, ... );
  Places( int handle, string className, void *argument, int argument_size,
      int dim, int size[] );

  Places( int handle, string className, int boundary_width, void *argument,
      int argument_size, int dim, ... );
  Places( int handle, string className, int boundary_width, void *argument,
      int argument_size, int dim, int size[] );

  ~Places( );

  void callAll( int functionId );
  void callAll( int functionId, void *argument, int arg_size );
  void *callAll( int functionId, void *argument[], int arg_size, int ret_size );
  void callSome( int functionId, int dim, int index[] );
  void callSome( int functionId, void *arguments[], int arg_size, int dim,
      int index[] );
  void *callSome( int functionId, void *arguments[], int arg_size, int ret_size,
      int dim, int index[] );
  void exchangeAll( int dest_handle, int functionId,
      vector<int*> *destinations );
  void exchangeBoundary( );
  void init_master( void *argument, int argument_size, int boundary_width );

private:
  void *ca_setup( int functionId, void *argument, int arg_size, int ret_size,
      Message::ACTION_TYPE type );
  void *cs_setup( int functionId, void *arguments, int arg_size, int ret_size,
      int dim, int index[], Message::ACTION_TYPE type );
};

#endif

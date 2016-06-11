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

#ifndef MTHREAD_H
#define MTHREAD_H

#include "MASS_base.h"
#include <pthread.h>

class Mthread {
 public:
  enum STATUS_TYPE { STATUS_READY,          // 0
                     STATUS_TERMINATE,      // 1
                     STATUS_CALLALL,        // 2
		     STATUS_EXCHANGEALL,    // 3
		     STATUS_AGENTSCALLALL,  // 4
		     STATUS_MANAGEALL	    // 5
  };

  static void init( );
  static void *run( void *param );
  static void resumeThreads( STATUS_TYPE new_status );
  static void barrierThreads( int tid );

  static pthread_mutex_t lock;
  static pthread_cond_t barrier_ready;
  static pthread_cond_t barrier_finished;
  static int barrier_count;
  static STATUS_TYPE status;

  static int threadCreated;
  static int agentBagSize;
};

#endif

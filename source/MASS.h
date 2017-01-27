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

#ifndef MASS_H
#define MASS_H

#include <vector>             // vector
#include "MASS_base.h"
#include "Utilities.h"
#include "Ssh2Connection.h"
#include "MNode.h"
#include "Places.h"
#include "Agents.h"

#define LIBSSH2_PORT 22

class MASS : public MASS_base {
	friend class Places;
	friend class Agents;
public:
	static void init(char *args[], int nProc, int nThr);
	static void finish();
private:
	static Utilities util;
	static vector<MNode*> mNodes;
	static void barrier_all_slaves()
	{
		barrier_all_slaves(NULL, 0, 0, NULL);
	};
	static void barrier_all_slaves(int localAgents[])
	{
		barrier_all_slaves(NULL, 0, 0, localAgents);
	};
	static void barrier_all_slaves(char *return_values, int stripe,
		int arg_size)
	{
		barrier_all_slaves(return_values, stripe, arg_size, NULL);
	};
	static void barrier_all_slaves(char *return_values, int stripe,
		int arg_size, int localAgents[]);
};

#endif

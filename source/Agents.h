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

#ifndef AGENTS_H
#define AGENTS_H

#include <string>
#include "Agents_base.h"
#include "Places.h"
#include "Message.h"

using namespace std;

class Agents : public Agents_base {
public:
	Agents(int handle, string className, void *argument, int argument_size,
		Places *places, int initPopulation);
	~Agents();

	void callAll(int functionId);
	void callAll(int functionId, void *argument, int arg_size);
	void *callAll(int functionId, void *argument, int arg_size,
		int ret_size);
	void manageAll();
	void ma_setup();
	int nAgents();

	void init_master(void *argument, int argument_size);
private:
	int *localAgents; // localAgents[i] = # agents in rank[i]
	int total;
	void *ca_setup(int functionId, void *argument, int arg_size, int ret_size,
		Message::ACTION_TYPE type);
};

#endif

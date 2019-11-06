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
 The following acknowledgment shall be used where appropriate in publications,
 presentations, etc.: © 2014-2015 University of Washington. MASS was developed
 by Computing and Software Systems at University of Washington Bothell. THE
 SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef PLACES_BASE_H
#define PLACES_BASE_H

#include <string>
#include <vector>
#include "Place.h"

using namespace std;

class Places_base {
    friend class MProcess;
    friend class Agents_base;
    friend class Place;

   public:
    Places_base(int handle, string className, int boundary_width,
                void *argument, int argument_size, int dim, int size[]);
    ~Places_base();

    void callAll(int functionId, void *argument, int tid);
    void **callAll(int functionId, void *argument, int arg_size, int ret_size,
                   int tid);
    void exchangeAll(Places_base *dstPlaces, int functionId,
                     vector<int *> *destinations, int tid);
    // exchangeAll that relies on neighbor functionality
    void exchangeAll(Places_base *dstPlaces, int functionId, int tid);
    void exchangeBoundary();  // called from Places.exchangeBoundary( )

    int getHandle() { return handle; };
    int getPlacesSize() { return places_size; };

   protected:
    const int handle;  // handle
    const string className;
    const int dimension;

    int lower_boundary;
    int upper_boundary;
    int places_size;
    int *size;
    int shadow_size;
    int boundary_width;

    void init_all(void *argument, int argument_size);
    vector<int> getGlobalArrayIndex(int singleIndex);
    void getLocalRange(int range[], int tid);
    static void *processRemoteExchangeRequest(void *param);
    void getGlobalNeighborArrayIndex(vector<int> src_index, int offset[],
                                     int dst_size[], int dst_dimension,
                                     int *dest_index);
    int getGlobalLinearIndexFromGlobalArrayIndex(int dest_index[],
                                                 int dest_size[],
                                                 int dest_dimension);
    int getRankFromGlobalLinearIndex(int globalLinearIndex);
    static void *exchangeBoundary_helper(void *param);
    static void *sendMessageByChild(void *param);
    struct ExchangeSendMessage {
        int rank;
        void *message;
    };
};

#endif

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
 SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PLACES_H
#define PLACES_H

#include <stdarg.h>  // int size, ...
#include <string>
#include "Message.h"
#include "Places_base.h"
#include "FileParser.h"

using namespace std;

class Places : public Places_base {
   public:

    /*Elias --> this is added for graph feature of MASS lib
    ----------------------------------------------------------------------------------------------------------------
    ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    Places(int handle,string className,int boundary_width,int dimension, string filename,
                     FILE_TYPE_ENUMS type, void* argument, int arg_size);
    Places(int handle,string className,int boundary_width,int dimension,void* argument,int argSize,int nVertices);
   // Places(int handle,string className,int boundary_width, int dimension, int size); 

    /*-----------------------------------------------------------------------------------------------------------------
     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    
    Places(int handle, string className, void *argument, int argument_size,int dim, ...);

    Places(int handle, string className, void *argument, int argument_size, int dim, int size[]);

    Places(int handle, string className, int boundary_width, void *argument, int argument_size, int dim, ...);

    Places(int handle, string className, int boundary_width, void *argument, int argument_size, int dim, int size[]);

    virtual ~Places(){};

    void callAll(int functionId);
    void callAll(int functionId, void *argument, int arg_size);
    void *callAll(int functionId, void *argument[], int arg_size, int ret_size);
    void callSome(int functionId, int dim, int index[], int numPlaces);
    void callSome(int functionId, void *arguments[], int arg_size, int dim,
                  int index[], int numPlaces);
    void *callSome(int functionId, void *arguments[], int arg_size,
                   int ret_size, int dim, int index[], int numPlaces);
    void exchangeAll(int dest_handle, int functionId,
                     vector<int *> *destinations);
    void exchangeAll(int dest_handle, int functionId);
    void exchangeBoundary();
    void init_master(void *argument, int argument_size, int boundary_width);


    /*Elias-->added for graph feature +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     *-----------------------------------------------------------------------------------------------------*/
    void init_master_base(void* argument, int arg_size, int boundary_width);
    vector<string>* getHosts();

    /*-----------------------------------------------------------------------------------------------------
     +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    
   private:
    std::string filename;
    FILE_TYPE_ENUMS fileType = FILE_TYPE_ENUMS::HIPPIE;//DEFAULT FILE TYPE ENUM

    void *ca_setup(int functionId, void *argument, int arg_size, int ret_size,
                   Message::ACTION_TYPE type);
    void *cs_setup(int functionId, void *arguments, int arg_size, int ret_size,
                   int dim, int index[], int numPlaces,
                   Message::ACTION_TYPE type);
};

#endif

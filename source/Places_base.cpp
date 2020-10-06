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

#include "Places_base.h"

#include <iostream>
#include <set>
#include <sstream>  // ostringstream
#include "DllClass.h"
#include "MASS_base.h"
#include "Message.h"
#include "limits.h"

// Used to enable or disable output in places
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
==================Elias -> added for graph features ==============================================================
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/**
 * Constructor for Places_base Objects. These Objects encompass the basic
 * features that are found in all Places Objects, enabling MASS to be able to
 * operate across all Places in a unified manner, regardless of each individual
 * Place instantiation.
 *
 * @param handle          a unique identifer (int) that designates a group of
 *                        places. Must be unique over all machines.
 * @param className       name of the user-created Place classes to load
 * @param boundary_width  width of the boundary to place between stripes
 * @param argument        array of arguments to pass into each Place constructor
 * @param argument_size   size of each argument (e.g. - sizeof( int ) )
 * @param dimension       how many dimensions this simulation encompasses
 * @param filemane        the name of the file(absolute path)
 *@param types            the file type as enums(MATSim, HIPPIE, TXT etc...)
 
 */
Places_base::Places_base(int handle, std::string className,int boundary_width,int dimension, string filename,
                     FILE_TYPE_ENUMS type, void* argument, int arg_size)
                    :handle(handle),
                     className(className),
                     dimension(dimension), 
                     boundary_width(boundary_width),
                     fileType(type),
                     filename(filename),
                     numVertices(0){

    ostringstream convert;
    if (printOutput == true) {
        convert << "Places_base handle = " << handle
                << ", class = " << className
                << ", argument = " << (char*)argument
                << ", argument_size = " << arg_size
                << ", dimension = " << dimension << ", boundary_width = " 
                << boundary_width << endl;
        MASS_base::log(convert.str());
    }

    init_all_graph(filename,type, argument);

}

/**
 * Constructor for Places_base Objects. These Objects encompass the basic
 * features that are found in all Places Objects, enabling MASS to be able to
 * operate across all Places in a unified manner, regardless of each individual
 * Place instantiation.
 *
 * @param handle          a unique identifer (int) that designates a group of
 *                        places. Must be unique over all machines.
 * @param className       name of the user-created Place classes to load
 * @param boundary_width  width of the boundary to place between stripes
 * @param argument        array of arguments to pass into each Place constructor
 * @param argument_size   size of each argument (e.g. - sizeof( int ) )
 * @param dimension       how many dimensions this simulation encompasses
 *@param nVertices        number of vertices to be constructed
 
 */
Places_base::Places_base(int handle, string className, int boundary_width, int dimension,
                        void* argument, int arg_size, int nVertices)
                        :handle(handle),
                         className(className),
                         dimension(dimension),
                         boundary_width(boundary_width),
                         fileType(FILE_TYPE_ENUMS::HIPPIE), //this a default and dummy 
                         filename("NOT_FROM_FILE_NAME"),
                         numVertices(nVertices){

    ostringstream convert;
    if (printOutput) {
        convert << "Places_base constructed: handle = " << handle
            << ", class = " << className
            << ", argument = " << (char*)argument
            << ", argument_size = " << arg_size
            << ", dimension = " << dimension << ", boundary_width = " << boundary_width << endl;
        MASS_base::log(convert.str());
    }
    if(dimension >= 1){
        this->size = new int[dimension];
        for(int i = 0; i < dimension; i++){
            size[i] = nVertices;
        }
    }
    else{
        this->size = new int [1];
        this->size[0] = nVertices;
    }

    init_all(argument, arg_size);
}

/*-------------------------------------------------------------------------------------------------------
 ++++++++++++++++++++++ End of graph feature constructors +++++++++++++++++++++++++++++++++++++++++++++++
 ----------------------------------------------------------------------------------------------------------*/


/**
 * Constructor for Places_base Objects. These Objects encompass the basic
 * features that are found in all Places Objects, enabling MASS to be able to
 * operate across all Places in a unified manner, regardless of each individual
 * Place instantiation.
 *
 * @param handle          a unique identifer (int) that designates a group of
 *                        places. Must be unique over all machines.
 * @param className       name of the user-created Place classes to load
 * @param boundary_width  width of the boundary to place between stripes
 * @param argument        array of arguments to pass into each Place constructor
 * @param argument_size   size of each argument (e.g. - sizeof( int ) )
 * @param dim             how many dimensions this simulation encompasses
 * @param size            array of numbers (int), representing the size of each
 *                        corresponding dimension in the simulation space
 */
Places_base::Places_base(int handle, string className, int boundary_width,
                         void *argument, int argument_size, int dim, int size[])
    : handle(handle),
      className(className),
      dimension(dim),
      boundary_width(boundary_width),
      //places_base_distributed_map(NULL),
      fileType(FILE_TYPE_ENUMS::HIPPIE), //this a default and dummy 
      filename("NOT_FROM_FILE_NAME"),
      numVertices(0){
    ostringstream convert;
    if (printOutput == true) {
        convert << "Places_base handle = " << handle
                << ", class = " << className
                << ", argument_size = " << argument_size
                << ", argument = " << (char *)argument
                << ", boundary_width = " << boundary_width << ", dim = " << dim
                << endl;
        MASS_base::log(convert.str());
    }

    if (size == NULL)
        // if given in "int dim, ..." format, init_all() must be called later
        return;
    this->size = new int[dim];
    for (int i = 0; i < dim; i++) this->size[i] = size[i];

    init_all(argument, argument_size);
}




/**
 * Destructor for core Places_base Objects. Frees dynamic space set up to store
 * all Places in this stripe, in addition to any shadow Places that may have
 * been created at runtime for this stripe.
 */
Places_base::~Places_base() {
    //destroy( places ); to be debugged
    DllClass *dllclass = MASS_base::dllMap[handle];
    for (int i = 0; i < places_size; i++)
        dllclass->destroy(dllclass->places[i]);

    if (dllclass->left_shadow != NULL)
        for (int i = 0; i < shadow_size; i++)
            dllclass->destroy(dllclass->left_shadow[i]);

    if (dllclass->right_shadow != NULL)
        for (int i = 0; i < shadow_size; i++)
            dllclass->destroy(dllclass->right_shadow[i]);

    dlclose(dllclass->stub);
  
  //delete the distributed map on MASS_base
    // auto myMap = MASS_base::getDistributedMap(handle);
    // if(myMap != NULL){
    //     delete myMap;
    //     myMap = NULL;
    // }

    //the associated distributed map
    if(places_base_distributed_map != NULL){
        delete places_base_distributed_map;
        places_base_distributed_map = NULL;
    }

    //delete placesVector
    for(int i = 0; i < (int)placesVector.size(); i++){
        std::vector<VertexPlace*> vecPlace = placesVector.at(i);
        for(int j = 0; j < (int)vecPlace.size(); j++){
            if(vecPlace.at(j) != NULL){
                delete vecPlace.at(j);
                vecPlace.at(j) = NULL;
            }

        }
    }
}

/**
 * Creates the individual Place Objects that live within this stripe - contained
 * within this Places collection. This method also sets up any shadows across
 * boundaries in the simulation space - enabling cross boundary communication to
 * occur during the course of a simulation.
 *
 * @param argument        argument to pass into each Place constructor
 * @param argument_size   total size of the argument
 */
void Places_base::init_all(void *argument, int argument_size) {
    // For debugging
    ostringstream convert;
    if (printOutput == true) {
        convert << "init_all handle = " << handle << ", class = " << className
                << ", argument_size = " << argument_size
                << ", argument = " << (char *)argument
                << ", dimension = " << dimension << endl;
        for (int i = 0; i < dimension; i++)
            convert << "size[" << i << "] = " << size[i] << "  ";
        convert << endl;
        MASS_base::log(convert.str());
    }
    // load the construtor and destructor
    DllClass *dllclass = new DllClass(className);

    MASS_base::dllMap.insert(map<int, DllClass *>::value_type(handle, dllclass));
  
    // calculate lower_boundary and upper_boundary
    int total = 1;
    for (int i = 0; i < dimension; i++) total *= size[i];
    int stripe = total / MASS_base::systemSize;

    lower_boundary = stripe * MASS_base::myPid;
    upper_boundary = (MASS_base::myPid < MASS_base::systemSize - 1)
                         ? lower_boundary + stripe - 1
                         : total - 1;
    places_size = upper_boundary - lower_boundary + 1;

    // instantiate_from_file Places objects within dlclass
    this->places_size = places_size;
    //  maintaining an entire set
    dllclass->places = new Place *[places_size];
    vector<int> index;
    index.reserve(dimension);

    // initialize all Places objects
    for (int i = 0; i < places_size; i++) {
        // instanitate a new place
        dllclass->places[i] = (Place *)(dllclass->instantiate(argument));
        dllclass->places[i]->size.reserve(dimension);
        for (int j = 0; j < dimension; j++)
            // define size[] and index[]
            dllclass->places[i]->size.push_back(size[j]);
        dllclass->places[i]->index = getGlobalArrayIndex(lower_boundary + i);
    }

    // allocate the left/right shadows
    if (boundary_width <= 0) {
        // no shadow space.
        shadow_size = 0;
        dllclass->left_shadow = NULL;
        dllclass->right_shadow = NULL;
        return;
    }
    shadow_size =
        (dimension == 1) ? boundary_width : total / size[0] * boundary_width;
    if (printOutput == true) {
        ostringstream convert;
        convert << "Places_base.shadow_size = " << shadow_size;
        MASS_base::log(convert.str());
    }
    dllclass->left_shadow =
        (MASS_base::myPid == 0) ? NULL : new Place *[shadow_size];
    dllclass->right_shadow = (MASS_base::myPid == MASS_base::systemSize - 1)
                                 ? NULL
                                 : new Place *[shadow_size];

    // initialize the left/right shadows
    for (int i = 0; i < shadow_size; i++) {
        // left shadow initialization
        if (dllclass->left_shadow != NULL) {
            // instanitate a new place
            dllclass->left_shadow[i] =
                (Place *)(dllclass->instantiate(argument));
            dllclass->left_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->left_shadow[i]->size.push_back(size[j]);
            }
            dllclass->left_shadow[i]->index =
                getGlobalArrayIndex(lower_boundary - shadow_size + i);
            dllclass->left_shadow[i]->outMessage = NULL;
            dllclass->left_shadow[i]->outMessage_size = 0;
            dllclass->left_shadow[i]->inMessage_size = 0;
        }

        // right shadow initialization
        if (dllclass->right_shadow != NULL) {
            // instanitate a new place
            dllclass->right_shadow[i] =
                (Place *)(dllclass->instantiate(argument));
            dllclass->right_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->right_shadow[i]->size.push_back(size[j]);
            }
            dllclass->right_shadow[i]->index =
                getGlobalArrayIndex(upper_boundary + i);
            dllclass->right_shadow[i]->outMessage = NULL;
            dllclass->right_shadow[i]->outMessage_size = 0;
            dllclass->right_shadow[i]->inMessage_size = 0;
        }
    } 
    if(printOutput){
        MASS_base::log("Places_base:init_all finished and exit");
    }
}

/**
 * Converts a given plain single index into a multi-dimensional index. This
 * allows absolute index values that would correspond to a single Place within
 * the global simulation space (ordering in a one dimensional array) to be
 * referenced by the actual index, according to the number of dimensions and
 * size of each dimension in the simulation space.
 *
 * @param singleIndex an index in a plain single dimension that will be
 *                    converted to a multi-dimensional index.
 * @return            a multi-dimensional index
 */
vector<int> Places_base::getGlobalArrayIndex(int singleIndex) {
    return getGlobalArrayIndex(singleIndex, 0);  // x-axis based ordering
}

/**
 * Converts a given plain single index into a multi-dimensional index. This
 * allows absolute index values that would correspond to a single Place within
 * the global simulation space (ordering in a one dimensional array) to be
 * referenced by the actual index, according to the number of dimensions and
 * size of each dimension in the simulation space.
 *
 * While the other Places_base::getGlobalArrayIndex( int singleIndex) method
 * assumes that the starting index is based on a flattening algorithm that
 * assumes first dimension-priority in the global array (e.g. - 'x' dimension is
 * evaluated first, then 'y' dimension, so that the singleIndex of 4 would
 * correspond to {4, 0} in a two dimensional array), this method allows the
 * user to specify the dimension that should take priority.
 *
 * @param index       an index into a single dimension ordering of places that
 *                    will be converted to a multi-dimensional coordinate index
 * @param dim         the dimension that will take priority in the resulting
 *                    indexing algorithm. Since dimension numbering is
 *                    zero-based ('x' dimension is 0), the value of dim should
 *                    be less than the number of dimensions
 *                    (Places_base::dimension) in the simulation space. Large
 *                    values will wrap around, so no out of bounds exception
 *                    will be thrown, but the result will not match with
 *                    expectations at runtime (unpredictable)
 * @return            a multi-dimensional index
 */
vector<int> Places_base::getGlobalArrayIndex(int index, int dim) {
    vector<int> coords;        // a multi-dimensional coordinate (index)
    coords.resize(dimension);  // must match size of dimensions in model

    // start at dimension user has indicated and proceed around loop to stop
    // at dimension value just before the starting point
    for (int i = dim; i < dim + (dimension - 1); i++) {
        // calculate from designated dimension
        coords[i % dimension] = index % size[i % dimension];
        index /= size[i % dimension];
    }
    // assign remainder to dimension value just before the starting point...
    coords[dim + (dimension - 1) % dimension] = index;

    return coords;
}

/**
 * Calls the referenced function, passing along a related argument, at each
 * Place contained within the simulation space (Places).
 *
 * @param functionId  the id of the function to call at each Place
 * @param argument    the address (pointer) of an argument to send to
 *                    the function called at each Place
 * @param arg_size    the total size of the argument
 * @param tid         the id of the thread
 * @return            the address of the address of (pointer to a pointer) the
 *                    return values stored from each function call
 */
void Places_base::callAll(int functionId, void *argument, int tid) {
    int range[2];
    getLocalRange(range, tid);

    DllClass *dllclass = MASS_base::dllMap[handle];

    // debugging
    ostringstream convert;
    //if (printOutput == true) {
        convert << "thread[" << tid << "] callAll functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1]
                << ", dllclass = " << (void *)dllclass;
        MASS_base::log(convert.str());
	//}

    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            if (printOutput == true) {
                convert.str("");
                convert << "thread[" << tid << "]: places[" << i
                        << "] = " << dllclass->places[i]
                        << ", vptr = " << *(int **)(dllclass->places[i]);
                MASS_base::log(convert.str());
            }
            dllclass->places[i]->callMethod(functionId,
                                            argument);  // <-- seg fault
        }
    }
}

/**
 * Calls the referenced function, passing along a related argument, at each
 * Place contained within the simulation space (Places). Any return values
 * generated from individual function calls are stored in an array, a pointer to
 * the address (pointer to a pointer) of which is returned as a result of this
 * call.
 *
 * @param functionId  the id of the function to call at each Place
 * @param argument    the address (pointer) of an array of arguments to send to
 *                    the function called at each Place
 * @param arg_size    the size (int) of each argument
 * @param ret_size    the size (int) of each return value
 * @param tid         the id of the thread
 * @return            the address of the address of (pointer to a pointer) the
 *                    return values stored from each function call
 */
void **Places_base::callAll(int functionId, void *argument, int arg_size,
                            int ret_size, int tid) {
    int range[2];
    getLocalRange(range, tid);

    // debugging
    ostringstream convert;
    if (printOutput == true) {
        convert << "thread[" << tid
                << "] callAll_return object functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1]
                << ", return_size = " << ret_size;
        MASS_base::log(convert.str());
    }

    DllClass *dllclass = MASS_base::dllMap[handle];
    char *return_values = MASS_base::currentReturns + range[0] * ret_size;
    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            if (printOutput == true) {
                convert.str("");
                convert << "thread[" << tid
                        << "]: places[i] = " << dllclass->places[i];
                MASS_base::log(convert.str());
            }
            memcpy((void *)return_values,
                   dllclass->places[i]->callMethod(
                       functionId, (char *)argument + arg_size * i),
                   ret_size);
            return_values += ret_size;
        }
    }

    return NULL;
}

/**
 * Calls the referenced function, passing along a related argument, at each
 * Place referenced by this call within the simulation space (Places).
 *
 * @param functionId  the id of the function to call at each Place
 * @param argument    the address (pointer) of an argument to send to
 *                    the function called at each Place
 * @param arg_size    the total size of the argument
 * @param tid         the id of the thread
 * @return            the address of the address of (pointer to a pointer) the
 *                    return values stored from each function call
 */
void Places_base::callSome(int functionId, void *argument, int tid) {
    int range[2];
    getLocalRange(range, tid);

    DllClass *dllclass = MASS_base::dllMap[handle];

    // debugging
    ostringstream convert;
    if (printOutput == true) {
        convert << "thread[" << tid << "] callSome functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1]
                << ", dllclass = " << (void *)dllclass;
        MASS_base::log(convert.str());
    }

    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            if (printOutput == true) {
                convert.str("");
                convert << "thread[" << tid << "]: places[" << i
                        << "] = " << dllclass->places[i]
                        << ", vptr = " << *(int **)(dllclass->places[i]);
                MASS_base::log(convert.str());
            }
            dllclass->places[i]->callMethod(functionId,
                                            argument);  // <-- seg fault
        }
    }
}

/**
 * Calls the referenced function, passing along a related argument, at each
 * Place referenced by this call contained within the simulation space (Places).
 * Any return values generated from individual function calls are stored in an
 * array, a pointer to the address (pointer to a pointer) of which is returned
 * as a result of this call.
 *
 * @param functionId  the id of the function to call at each Place
 * @param argument    the address (pointer) of an array of arguments to send to
 *                    the function called at each Place
 * @param arg_size    the size (int) of each argument
 * @param ret_size    the size (int) of each return value
 * @param tid         the id of the thread
 * @return            the address of the address of (pointer to a pointer) the
 *                    return values stored from each function call
 */
void **Places_base::callSome(int functionId, void *argument, int arg_size,
                             int ret_size, int tid) {
    int range[2];
    getLocalRange(range, tid);

    // debugging
    ostringstream convert;
    if (printOutput == true) {
        convert << "thread[" << tid
                << "] callAll_return object functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1]
                << ", return_size = " << ret_size;
        MASS_base::log(convert.str());
    }

    DllClass *dllclass = MASS_base::dllMap[handle];
    char *return_values = MASS_base::currentReturns + range[0] * ret_size;
    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            if (printOutput == true) {
                convert.str("");
                convert << "thread[" << tid
                        << "]: places[i] = " << dllclass->places[i];
                MASS_base::log(convert.str());
            }
            memcpy((void *)return_values,
                   dllclass->places[i]->callMethod(
                       functionId, (char *)argument + arg_size * i),
                   ret_size);
            return_values += ret_size;
        }
    }

    return NULL;
}

/**
 * Returns the first and last of the range that should be allocated to a given
 * thread.
 *
 * @param  tid  an id of the thread that calls this function
 * @return      an array of two integers: element 0 = the first and element 1 =
 *              the last
 */
void Places_base::getLocalRange(int range[], int tid) {
  cerr << "getLocalRange: nThreads = " << MASS_base::threads.size( ) << ", tid = " << tid << endl;
    int nThreads = MASS_base::threads.size();
    int portion = places_size / nThreads;  // a range to be allocated per thread
    int remainder = places_size % nThreads;

    if (portion == 0) {
        // there are more threads than elements in the MASS.Places_base object
        if (remainder > tid) {
            range[0] = tid;
            range[1] = tid;
        } else {
            range[0] = -1;
            range[1] = -1;
        }
    } else {
        // there are more MASS.Places than threads
        int first = tid * portion;
        int last = (tid + 1) * portion - 1;
        if (tid < remainder) {
            // add in remainders
            first += tid;
            last = last + tid + 1;  // 1 is one of remainders.
        } else {
            // remainders have been assigned to previous threads
            first += remainder;
            last += remainder;
        }
        range[0] = first;
        range[1] = last;
    }
}

void Places_base::exchangeAll(Places_base *dstPlaces, int functionId,
                              vector<int *> *destinations, int tid) {
    int range[2];
    getLocalRange(range, tid);
    ostringstream convert;
    // debugging
    if (printOutput == true) {
        convert << "thread[" << tid
                << "] exchangeAll functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1];
        MASS_base::log(convert.str());
    }

    DllClass *src_dllclass = MASS_base::dllMap[handle];
    DllClass *dst_dllclass = MASS_base::dllMap[dstPlaces->handle];

    if (printOutput == true) {
        convert.str("");
        convert << "tid[" << tid << "]: checks destinations:";
        for (int i = 0; i < int(destinations->size()); i++) {
            int *offset = (*destinations)[i];
            convert << "[" << offset[0] << "][" << offset[1] << "]  ";
        }
        MASS_base::log(convert.str());
    }
    // now scan all places within range[0] ~ range[1]
    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            // for each place
            Place *srcPlace = (Place *)(src_dllclass->places[i]);

            // check its neighbors
            for (int j = 0; j < int(destinations->size()); j++) {
                // for each neighbor
                int *offset = (*destinations)[j];
                int neighborCoord[dstPlaces->dimension];

                // compute its coordinate
                getGlobalNeighborArrayIndex(
                    srcPlace->index, offset, dstPlaces->size,
                    dstPlaces->dimension, neighborCoord);
                if (printOutput == true) {
                    convert.str("");
                    convert << "tid[" << tid << "]: calls from"
                            << "[" << srcPlace->index[0] << "]["
                            << srcPlace->index[1] << "]"
                            << " (neighborCord[" << neighborCoord[0] << "]["
                            << neighborCoord[1] << "]"
                            << " dstPlaces->size[" << dstPlaces->size[0] << "]["
                            << dstPlaces->size[1] << "]";
                }
                if (neighborCoord[0] != -1) {
                    // destination valid
                    int globalLinearIndex =
                        getGlobalLinearIndexFromGlobalArrayIndex(
                            neighborCoord, dstPlaces->size,
                            dstPlaces->dimension);
                    if (printOutput == true) {
                        convert << " linear = " << globalLinearIndex
                                << " lower = " << dstPlaces->lower_boundary
                                << " upper = " << dstPlaces->upper_boundary
                                << ")";
                    }

                    if (globalLinearIndex >= dstPlaces->lower_boundary &&
                        globalLinearIndex <= dstPlaces->upper_boundary) {
                        // local destination
                        int destinationLocalLinearIndex =
                            globalLinearIndex - dstPlaces->lower_boundary;
                        Place *dstPlace =
                            (Place
                                 *)(dst_dllclass
                                        ->places[destinationLocalLinearIndex]);

                        if (printOutput == true) {
                            convert << " to [" << dstPlace->index[0] << "]["
                                    << dstPlace->index[1] << "]";
                        }
                        // call the destination function
                        void *inMessage = dstPlace->callMethod(
                            functionId, srcPlace->outMessage);

                        // store this inMessage:
                        // note that callMethod must return a dynamic memory
                        // space
                        srcPlace->inMessages.push_back(inMessage);

                        // for debug
                        if (printOutput == true) {
                            convert << " inMessage = "
                                    << *(int *)(srcPlace->inMessages.back());
                        }
                    } else {
                        // remote destination

                        // find the destination node
                        int destRank =
                            getRankFromGlobalLinearIndex(globalLinearIndex);

                        // create a request
                        int orgGlobalLinearIndex =
                            getGlobalLinearIndexFromGlobalArrayIndex(
                                &(srcPlace->index[0]), size, dimension);
                        RemoteExchangeRequest *request =
                            new RemoteExchangeRequest(
                                globalLinearIndex, orgGlobalLinearIndex,
                                j,  // inMsgIndex
                                srcPlace->inMessage_size, srcPlace->outMessage,
                                srcPlace->outMessage_size, false);

                        // enqueue the request to this node.map
                        pthread_mutex_lock(&MASS_base::request_lock);
                        MASS_base::remoteRequests[destRank]->push_back(request);

                        if (printOutput == true) {
                            convert.str("");
                            convert
                                << "remoteRequest[" << destRank
                                << "]->push_back:"
                                << " org = " << orgGlobalLinearIndex
                                << " dst = " << globalLinearIndex
                                << " size( ) = "
                                << MASS_base::remoteRequests[destRank]->size();
                            MASS_base::log(convert.str());
                        }
                        pthread_mutex_unlock(&MASS_base::request_lock);
                    }
                } else {
                    if (printOutput == true)
                        convert << " to destination invalid";
                }
                if (printOutput == true) {
                    MASS_base::log(convert.str());
                }
            }
        }
    }

    // all threads must barrier synchronize here.
    Mthread::barrierThreads(tid);
    if (tid == 0) {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid
                    << "] now enters processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
        // the main thread spawns as many communication threads as the number of
        // remote computing nodes and let each invoke processRemoteExchangeReq.

        // args to threads: rank, srcHandle, dstHandle, functionId,
        // lower_boundary
        int comThrArgs[MASS_base::systemSize][5];
        pthread_t thread_ref[MASS_base::systemSize];  // communication thread id
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;

            // set arguments
            comThrArgs[rank][0] = rank;
            comThrArgs[rank][1] = handle;
            comThrArgs[rank][2] = dstPlaces->handle;
            comThrArgs[rank][3] = functionId;
            comThrArgs[rank][4] = lower_boundary;

            // start a communication thread
            if (pthread_create(&thread_ref[rank], NULL,
                               Places_base::processRemoteExchangeRequest,
                               comThrArgs[rank]) != 0) {
                MASS_base::log(
                    "Places_base.exchangeAll: failed in pthread_create");
                exit(-1);
            }
        }

        // wait for all the communication threads to be terminated
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;
            pthread_join(thread_ref[rank], NULL);
        }
    } else {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid << "] skips processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
    }
}

/**
 * Set the neighbors for a set of places.
 * @param dstPlaces - places whose neighbors we are setting
 * @param destinations - coordinates of the neighboring places
 * @param tid
 */
void Places_base::setAllPlacesNeighbors(Places_base *dstPlaces,
                                        vector<int *> destinations, int tid) {
    int range[2];
    getLocalRange(range, tid);
    ostringstream convert;
    // debugging
    if (printOutput == true) {
        convert << "thread[" << tid
                << "] setAllPlacesNeighbors: range[0] = " << range[0]
                << " range[1] = " << range[1];
        MASS_base::log(convert.str());
    }

    DllClass *src_dllclass = MASS_base::dllMap[handle];

    if (printOutput == true) {
        convert.str("");
        convert << "tid[" << tid << "]: checks destinations:";
        for (int i = 0; i < int(destinations.size()); i++) {
            int *offset = destinations[i];
            convert << "[" << offset[0] << "][" << offset[1] << "]  ";
        }
        MASS_base::log(convert.str());
    }

    // now scan all places within range[0] ~ range[1]
    if (range[0] >= 0 && range[1] >= 0) {
        if (printOutput == true) {
            convert << "Range is within range[0] to range[1]\n";
            MASS_base::log(convert.str());
        }

        for (int i = range[0]; i <= range[1]; i++) {
            //      if(printOutput == true){
            // convert << "Adding neighbors to  place " << i << "\n";
            //	MASS_base::log( convert.str( ) );
            //}
            // for each place
            Place *srcPlace = (Place *)(src_dllclass->places[i]);

            // update its neighbors list
            // srcPlace->neighbors = destinations;

            srcPlace->addNeighbors(destinations);
        }
    }

    if (printOutput == true) {
        convert << "Finished adding neighbors to all places.\n";
        MASS_base::log(convert.str());
    }
}

/**
 * Replicates the original exchangeAll which acted upon destinations included
 * in the parameters.  This version now performs sanity checks on the
 * destinations.
 * @param dstPlaces
 * @param functionId
 * @param destinations
 * @param tid
 */
void Places_base::exchangeAll(Places_base *dstPlaces, int functionId, int tid) {
    int range[2];
    getLocalRange(range, tid);
    ostringstream convert;
    // debugging
    if (printOutput == true) {
        convert << "thread[" << tid
                << "] exchangeAll functionId = " << functionId
                << ", range[0] = " << range[0] << " range[1] = " << range[1];
        MASS_base::log(convert.str());
    }

    DllClass *src_dllclass = MASS_base::dllMap[handle];
    DllClass *dst_dllclass = MASS_base::dllMap[dstPlaces->handle];
    // now scan all places within range[0] ~ range[1]
    if (range[0] >= 0 && range[1] >= 0) {
        for (int i = range[0]; i <= range[1]; i++) {
            // for each place
            Place *srcPlace = (Place *)(src_dllclass->places[i]);

            // check its neighbors
            // for ( int j = 0; j < int( srcPlace->neighbors.size( ) ); j++ ) {
            int inMsgIndex = -1;
            for (vector<int *>::iterator it = srcPlace->neighbors.begin();
                 it != srcPlace->neighbors.end(); ++it) {
                inMsgIndex++;
                // for each neighbor
                int *offset = *it;  //(srcPlace->neighbors)[j];
                int neighborCoord[dstPlaces->dimension];

                // compute its coordinate
                getGlobalNeighborArrayIndex(
                    srcPlace->index, offset, dstPlaces->size,
                    dstPlaces->dimension, neighborCoord);
                if (printOutput == true) {
                    convert.str("");
                    convert << "tid[" << tid << "]: calls from"
                            << "[" << srcPlace->index[0] << "]["
                            << srcPlace->index[1] << "]"
                            << " (neighborCord[" << neighborCoord[0] << "]["
                            << neighborCoord[1] << "]"
                            << " dstPlaces->size[" << dstPlaces->size[0] << "]["
                            << dstPlaces->size[1] << "]";
                }
                if (neighborCoord[0] != -1) {
                    // destination valid
                    int globalLinearIndex =
                        getGlobalLinearIndexFromGlobalArrayIndex(
                            neighborCoord, dstPlaces->size,
                            dstPlaces->dimension);
                    if (printOutput == true) {
                        convert << " linear = " << globalLinearIndex
                                << " lower = " << dstPlaces->lower_boundary
                                << " upper = " << dstPlaces->upper_boundary
                                << ")";
                    }

                    if (globalLinearIndex >= dstPlaces->lower_boundary &&
                        globalLinearIndex <= dstPlaces->upper_boundary) {
                        // local destination
                        int destinationLocalLinearIndex =
                            globalLinearIndex - dstPlaces->lower_boundary;
                        Place *dstPlace =
                            (Place
                                 *)(dst_dllclass
                                        ->places[destinationLocalLinearIndex]);

                        if (printOutput == true) {
                            convert << " to [" << dstPlace->index[0] << "]["
                                    << dstPlace->index[1] << "]";
                        }
                        // call the destination function
                        void *inMessage = dstPlace->callMethod(
                            functionId, srcPlace->outMessage);

                        // store this inMessage:
                        // note that callMethod must return a dynamic memory
                        // space
                        srcPlace->inMessages.push_back(inMessage);

                        // for debug
                        if (printOutput == true) {
                            convert << " inMessage = "
                                    << *(int *)(srcPlace->inMessages.back());
                        }
                    } else {
                        // remote destination

                        // find the destination node
                        int destRank =
                            getRankFromGlobalLinearIndex(globalLinearIndex);

                        // create a request
                        int orgGlobalLinearIndex =
                            getGlobalLinearIndexFromGlobalArrayIndex(
                                &(srcPlace->index[0]), size, dimension);
                        RemoteExchangeRequest *request =
                            new RemoteExchangeRequest(
                                globalLinearIndex, orgGlobalLinearIndex,
                                inMsgIndex,  // inMsgIndex
                                srcPlace->inMessage_size, srcPlace->outMessage,
                                srcPlace->outMessage_size, false);

                        // enqueue the request to this node.map
                        pthread_mutex_lock(&MASS_base::request_lock);
                        MASS_base::remoteRequests[destRank]->push_back(request);

                        if (printOutput == true) {
                            convert.str("");
                            convert
                                << "remoteRequest[" << destRank
                                << "]->push_back:"
                                << " org = " << orgGlobalLinearIndex
                                << " dst = " << globalLinearIndex
                                << " size( ) = "
                                << MASS_base::remoteRequests[destRank]->size();
                            MASS_base::log(convert.str());
                        }
                        pthread_mutex_unlock(&MASS_base::request_lock);
                    }
                } else {
                    if (printOutput == true)
                        convert << " to destination invalid";
                }
                if (printOutput == true) {
                    MASS_base::log(convert.str());
                }
            }
        }
    }

    // all threads must barrier synchronize here.
    Mthread::barrierThreads(tid);
    if (tid == 0) {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid
                    << "] now enters processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
        // the main thread spawns as many communication threads as the number of
        // remote computing nodes and let each invoke processRemoteExchangeReq.

        // args to threads: rank, srcHandle, dstHandle, functionId,
        // lower_boundary
        int comThrArgs[MASS_base::systemSize][5];
        pthread_t thread_ref[MASS_base::systemSize];  // communication thread id
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;

            // set arguments
            comThrArgs[rank][0] = rank;
            comThrArgs[rank][1] = handle;
            comThrArgs[rank][2] = dstPlaces->handle;
            comThrArgs[rank][3] = functionId;
            comThrArgs[rank][4] = lower_boundary;

            // start a communication thread
            if (pthread_create(&thread_ref[rank], NULL,
                               Places_base::processRemoteExchangeRequest,
                               comThrArgs[rank]) != 0) {
                MASS_base::log(
                    "Places_base.exchangeAll: failed in pthread_create");
                exit(-1);
            }
        }

        // wait for all the communication threads to be terminated
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;
            pthread_join(thread_ref[rank], NULL);
        }
    } else {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid << "] skips processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
    }

    // all threads must barrier synchronize here.
    Mthread::barrierThreads(tid);
    if (tid == 0) {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid
                    << "] now enters processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
        // the main thread spawns as many communication threads as the number of
        // remote computing nodes and let each invoke processRemoteExchangeReq.

        // args to threads: rank, srcHandle, dstHandle, functionId,
        // lower_boundary
        int comThrArgs[MASS_base::systemSize][5];
        pthread_t thread_ref[MASS_base::systemSize];  // communication thread id
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;

            // set arguments
            comThrArgs[rank][0] = rank;
            comThrArgs[rank][1] = handle;
            comThrArgs[rank][2] = dstPlaces->handle;
            comThrArgs[rank][3] = functionId;
            comThrArgs[rank][4] = lower_boundary;

            // start a communication thread
            if (pthread_create(&thread_ref[rank], NULL,
                               Places_base::processRemoteExchangeRequest,
                               comThrArgs[rank]) != 0) {
                MASS_base::log(
                    "Places_base.exchangeAll: failed in pthread_create");
                exit(-1);
            }
        }

        // wait for all the communication threads to be terminated
        for (int rank = 0; rank < MASS_base::systemSize; rank++) {
            if (rank == MASS_base::myPid)  // don't communicate with myself
                continue;
            pthread_join(thread_ref[rank], NULL);
        }
    } else {
        if (printOutput == true) {
            convert.str("");
            convert << "tid[" << tid << "] skips processRemoteExchangeRequest";
            MASS_base::log(convert.str());
        }
    }
}

/**
 *
 * @param param
 * @return
 */
void *Places_base::processRemoteExchangeRequest(void *param) {
    int destRank = ((int *)param)[0];
    int srcHandle = ((int *)param)[1];
    int destHandle_at_src = ((int *)param)[2];
    int functionId = ((int *)param)[3];
    int my_lower_boundary = ((int *)param)[4];

    vector<RemoteExchangeRequest *> *orgRequest = NULL;
    ostringstream convert;

    if (printOutput == true) {
        convert.str("");
        convert << "rank[" << destRank
                << "]: starts processRemoteExchangeRequest";
        MASS_base::log(convert.str());
    }
    // pick up the next rank to process
    orgRequest = MASS_base::remoteRequests[destRank];

    // for debugging
    pthread_mutex_lock(&MASS_base::request_lock);

    if (printOutput == true) {
        convert.str("");
        convert << "tid[" << destRank
                << "] sends an exhange request to rank: " << destRank
                << " size() = " << orgRequest->size() << endl;
        for (int i = 0; i < int(orgRequest->size()); i++) {
            convert << "send from " << (*orgRequest)[i]->orgGlobalLinearIndex
                    << " to " << (*orgRequest)[i]->destGlobalLinearIndex
                    << " at " << (*orgRequest)[i]->inMessageIndex
                    << " inMsgSize: " << (*orgRequest)[i]->inMessageSize
                    << " outMsgSize: " << (*orgRequest)[i]->outMessageSize
                    << endl;
        }
        MASS_base::log(convert.str());
    }
    pthread_mutex_unlock(&MASS_base::request_lock);

    // now compose and send a message by a child
    Message *messageToDest =
        new Message(Message::PLACES_EXCHANGE_ALL_REMOTE_REQUEST, srcHandle,
                    destHandle_at_src, functionId, orgRequest);

    struct ExchangeSendMessage rankNmessage;
    rankNmessage.rank = destRank;
    rankNmessage.message = messageToDest;
    pthread_t thread_ref;
    pthread_create(&thread_ref, NULL, sendMessageByChild, &rankNmessage);

    // receive a message by myself
    Message *messageFromSrc = MASS_base::exchange.receiveMessage(destRank);

    // at this point,
    // at this point, the message must be exchanged.
    pthread_join(thread_ref, NULL);

    // process a message
    vector<RemoteExchangeRequest *> *receivedRequest =
        messageFromSrc->getExchangeReqList();

    int destHandle_at_dst = messageFromSrc->getDestHandle();
    Places_base *dstPlaces = MASS_base::placesMap[destHandle_at_dst];
    DllClass *dst_dllclass = MASS_base::dllMap[destHandle_at_dst];

    if (printOutput == true) {
        convert.str("");
        convert << "request from rank[" << destRank
                << "] = " << receivedRequest;
        convert << " size( ) = " << receivedRequest->size();
    }

    // get prepared for a space to sotre return values
    int inMessageSizes = 0;
    for (int i = 0; i < int(receivedRequest->size()); i++)
        inMessageSizes += (*receivedRequest)[i]->inMessageSize;
    char retVals[inMessageSizes];

    if (printOutput == true) {
        convert << " retVals = " << (void *)retVals
                << " total inMessageSizes = " << inMessageSizes << endl;
    }
    // for each place, call the corresponding callMethod( ).
    char *retValPos = retVals;
    for (int i = 0; i < int(receivedRequest->size()); i++) {
        if (printOutput == true) {
            convert << "received from "
                    << (*receivedRequest)[i]->orgGlobalLinearIndex << " to "
                    << (*receivedRequest)[i]->destGlobalLinearIndex << " at "
                    << (*receivedRequest)[i]->inMessageIndex
                    << " inMsgSize: " << (*receivedRequest)[i]->inMessageSize
                    << " outMsgSize: " << (*receivedRequest)[i]->outMessageSize
                    << " dstPlaces->lower = " << dstPlaces->lower_boundary
                    << " dstPlaces->upper = " << dstPlaces->upper_boundary;
        }
        int globalLinearIndex = (*receivedRequest)[i]->destGlobalLinearIndex;
        void *outMessage = (*receivedRequest)[i]->outMessage;

        if (globalLinearIndex >= dstPlaces->lower_boundary &&
            globalLinearIndex <= dstPlaces->upper_boundary) {
            // local destination
            int destinationLocalLinearIndex =
                globalLinearIndex - dstPlaces->lower_boundary;

            if (printOutput == true)
                convert << " dstLocal = " << destinationLocalLinearIndex
                        << endl;

            Place *dstPlace =
                (Place *)(dst_dllclass->places[destinationLocalLinearIndex]);

            // call the destination function
            void *inMessage = dstPlace->callMethod(functionId, outMessage);
            memcpy(retValPos, inMessage, (*receivedRequest)[i]->inMessageSize);
            retValPos += (*receivedRequest)[i]->inMessageSize;
        }
    }
    if (printOutput == true) {
        MASS_base::log(convert.str());
    }
    delete messageFromSrc;

    // send return values by a child thread
    Message *messageToSrc =
        new Message(Message::PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT, retVals,
                    inMessageSizes);
    rankNmessage.rank = destRank;
    rankNmessage.message = messageToSrc;
    pthread_create(&thread_ref, NULL, sendMessageByChild, &rankNmessage);

    // receive return values by myself in parallel
    Message *messageFromDest = MASS_base::exchange.receiveMessage(destRank);

    // at this point, the message must be exchanged.
    pthread_join(thread_ref, NULL);
    delete messageToSrc;

    // store return values to the orignal places
    int argumentSize = messageFromDest->getArgumentSize();
    char argument[argumentSize];
    messageFromDest->getArgument(argument);

    int pos = 0;
    DllClass *src_dllclass = MASS_base::dllMap[srcHandle];

    if (printOutput == true) {
        convert.str("");
        convert << "return values: argumentSize = " << argumentSize
                << " src_dllclass = " << src_dllclass
                << " orgRequest->size( ) = " << orgRequest->size();
        MASS_base::log(convert.str());
    }

    for (int i = 0; i < int(orgRequest->size()); i++) {
        // local source
        int orgLocalLinearIndex =
            (*orgRequest)[i]->orgGlobalLinearIndex - my_lower_boundary;

        // locate a local place
        Place *srcPlace = (Place *)(src_dllclass->places[orgLocalLinearIndex]);

        // store a return value to it
        char *inMessage = new char[srcPlace->inMessage_size];
        memcpy(inMessage, argument + pos, srcPlace->inMessage_size);
        pos += srcPlace->inMessage_size;

        // insert an item at inMessageIndex or just append it.
        if (int(srcPlace->inMessages.size()) > (*orgRequest)[i]->inMessageIndex)
            srcPlace->inMessages.insert(
                srcPlace->inMessages.begin() + (*orgRequest)[i]->inMessageIndex,
                (void *)inMessage);
        else
            srcPlace->inMessages.push_back((void *)inMessage);

        if (printOutput == true) {
            convert.str("");
            convert << "srcPlace[" << srcPlace->index[0] << "]["
                    << srcPlace->index[1] << "] inserted "
                    << "at " << (*orgRequest)[i]->inMessageIndex;
            MASS_base::log(convert.str());
        }
    }
    delete messageToDest;  // messageToDest->orgReuqest is no longer used.
                           // delete it.
    delete messageFromDest;

    return NULL;
}

/**
 *
 * @param param
 * @return NULL
 */
void *Places_base::sendMessageByChild(void *param) {
    int rank = ((struct ExchangeSendMessage *)param)->rank;
    Message *message =
        (Message *)((struct ExchangeSendMessage *)param)->message;
    MASS_base::exchange.sendMessage(rank, message);
    return NULL;
}

/**
 *
 */
void Places_base::exchangeBoundary() {
    if (shadow_size == 0) {  // no boundary, no exchange
        ostringstream convert;
        convert << "places (handle = " << handle
                << ") has NO boundary, and thus invokes NO exchange boundary";
        MASS_base::log(convert.str());
        return;
    }

    pthread_t thread_ref = 0l;

    if (printOutput == true) {
        MASS_base::log("exchangeBoundary starts");
    }

    int param[2][4];
    if (MASS_base::myPid < MASS_base::systemSize - 1) {
        // create a child in charge of handling the right shadow.
        param[0][0] = 'R';
        param[0][1] = handle;
        param[0][2] = places_size;
        param[0][3] = shadow_size;
        if (printOutput == true) {
            ostringstream convert;
            convert << "exchangeBoundary: pthreacd_create( helper, R ) "
                       "places_size="
                    << places_size;
            MASS_base::log(convert.str());
        }
        pthread_create(&thread_ref, NULL, exchangeBoundary_helper, param[0]);
    }

    if (MASS_base::myPid > 0) {
        // the main takes charge of handling the left shadow.
        param[1][0] = 'L';
        param[1][1] = handle;
        param[1][2] = places_size;
        param[1][3] = shadow_size;
        if (printOutput == true) {
            ostringstream convert;
            convert << "exchangeBoundary: main thread( helper, L ) places_size="
                    << places_size;
            MASS_base::log(convert.str());
        }
        exchangeBoundary_helper(param[1]);
    }

    if (thread_ref != 0l) {
        // we are done with exchangeBoundary
        int error_code = pthread_join(thread_ref, NULL);
        if (error_code != 0) {  // if we remove this if-clause, we will get
                                // a segmentation fault! Why?
            ostringstream convert;
            convert << "exchangeBoundary: the main performs pthread_join "
                       "with the child...error_code = "
                    << error_code;
            MASS_base::log(convert.str());
        }
    }
}

/**
 *
 * @param param
 * @return
 */
void *Places_base::exchangeBoundary_helper(void *param) {
    // identifiy the boundary space;
    char direction = ((int *)param)[0];
    int handle = ((int *)param)[1];
    int places_size = ((int *)param)[2];
    int shadow_size = ((int *)param)[3];
    DllClass *dllclass = MASS_base::dllMap[handle];

    ostringstream convert;
    if (printOutput == true) {
        convert << "Places_base.exchangeBoundary_helper direction = "
                << direction << ", handle = " << handle
                << ", places_size = " << places_size << ", shadow_size = "
                << shadow_size
                //<< ", outMessage_size = " << outMessage_size
                << endl;
        MASS_base::log(convert.str());
    }

    Place **boundary = (direction == 'L')
                           ? dllclass->places
                           : dllclass->places + (places_size - shadow_size);

    // allocate a buffer to contain all outMessages in this boundary
    int outMessage_size = boundary[0]->outMessage_size;
    int buffer_size = shadow_size * outMessage_size;
    char *buffer = (char *)(malloc(buffer_size));

    // copy all the outMessages into the buffer
    char *pos = buffer;  // we shouldn't change the buffer pointer.
    for (int i = 0; i < shadow_size; i++) {
        memcpy(pos, boundary[i]->outMessage, outMessage_size);
        pos += outMessage_size;
    }

    if (printOutput == true) {
        convert.str("");
        convert << "Places_base.exchangeBoundary_helper direction = "
                << direction << ", outMessage_size = " << outMessage_size
                << endl;
        pos = buffer;
        for (int i = 0; i < shadow_size; i++) {
            convert << *(int *)pos << endl;
            pos += outMessage_size;
        }
        MASS_base::log(convert.str());
    }

    // create a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message
    Message *messageToDest = new Message(
        Message::PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST, buffer, buffer_size);

    // compose a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message
    int destRank =
        (direction == 'L') ? MASS_base::myPid - 1 : MASS_base::myPid + 1;
    struct ExchangeSendMessage rankNmessage;
    rankNmessage.rank = destRank;
    rankNmessage.message = messageToDest;

    if (printOutput == true) {
        convert.str("");
        convert << "Places_base.exchangeBoundary_helper direction = "
                << direction << ", rankNmessage.rank = " << rankNmessage.rank
                << endl;
        MASS_base::log(convert.str());
    }

    // send it to my neighbor with a child
    pthread_t thread_ref = 0l;
    pthread_create(&thread_ref, NULL, sendMessageByChild, &rankNmessage);

    // receive a PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST message from my
    // neighbor
    Message *messageFromDest = MASS_base::exchange.receiveMessage(destRank);

    if (printOutput == true) {
        convert.str("");
        convert << "Places_base.exchangeBoundary_helper direction = "
                << direction << ", messageFromDest = " << messageFromDest
                << endl;
        MASS_base::log(convert.str());
    }

    // wait for the child termination
    if (thread_ref != 0l) {
        if (pthread_join(thread_ref, NULL) == 0) {
            if (printOutput == true) {
                convert.str("");
                convert << "Places_base.exchangeBoundary_helper direction = "
                        << direction << ", sendMessageByChild terminated"
                        << endl;
                MASS_base::log(convert.str());
            }
        }
    }

    // delete the message sent
    free(buffer);  // this is because messageToDest->argument_in_heap is
                   // false
    buffer = 0;
    delete messageToDest;
    messageToDest = 0;

    // extract the message reeived and copy it to the corresponding shadow.
    Place **shadow =
        (direction == 'L') ? dllclass->left_shadow : dllclass->right_shadow;
    buffer = (char *)(messageFromDest->getArgumentPointer());

    // copy the buffer contents into the corresponding shadow
    for (int i = 0; i < shadow_size; i++) {
        if (shadow[i]->outMessage_size == 0) {
            // first allocate a space
            shadow[i]->outMessage_size = outMessage_size;
            shadow[i]->outMessage = malloc(outMessage_size);
        }
        memcpy(shadow[i]->outMessage, buffer, outMessage_size);
        if (printOutput == true) {
            convert.str("");
            convert << "Places_base.exchangeBoundary_helper direction = "
                    << direction << ", shadow[" << i
                    << "]->outMessage = " << shadow[i]->outMessage
                    << ", buffer = " << *(int *)buffer << endl;
            MASS_base::log(convert.str());
        }
        buffer += outMessage_size;
    }

    // delete the message received
    // note that messageFromDest->argument is deleted automatically.
    delete messageFromDest;

    return NULL;
}

/**
 * This method takes the coordinates of a given Place in the simulation and
 * determines the coordinates of a neighbor, based on the offset values. The
 * neighbor coordinates are stored within the dest_index argument, which
 * allows for multi-dimensional array support (e.g. - x, y, and z axis
 * coordinates).
 * @param src_index       coordinates of source index
 * @param offset          coordinate offsets from source index
 * @param dst_size        size of each dimension in simulation space
 * @param dest_dimension  number of dimensions the destination simulation
 * space contains (likely equivalent to number of dimensions in source
 * simulation)
 * @param dest_index      storage for the destination index values (can't
 * simply return a single value and easily support multi-dimensional
 * coordinates)
 */
void Places_base::getGlobalNeighborArrayIndex(vector<int> src_index,
                                              int offset[], int dst_size[],
                                              int dest_dimension,
                                              int dest_index[]) {
    for (int i = 0; i < dest_dimension; i++) {
        dest_index[i] = src_index[i] + offset[i];  // calculate dest index

        if (dest_index[i] < 0 || dest_index[i] >= dst_size[i]) {
            // out of range
            for (int j = 0; j < dest_dimension; j++) {
                // all index must be set -1
                dest_index[j] = -1;
                return;
            }
        }
    }
}

/**
 * Retrieves the absolute index within the global array of Places in this
 * simulation. This method will convert the index array passed in, which may
 * refer to specific locations within a multi-dimensional array, into a
 * single index value that refers to the same Place within the global
 * (single dimensional) collection of Places for a simulation.
 *
 * @param index     multi-dimensional indexes to a specific Place within
 * your Places collection
 * @param size      sizes for each dimension in your simulation
 * @param dimension the number of dimensions in your simulation
 * @return          the global linear index (single dimensional index) to a
 *                  Place - or, INT_MIN if location does not exist (out of
 *                  bounds, etc)
 */
int Places_base::getGlobalLinearIndexFromGlobalArrayIndex(int index[],
                                                          int size[],
                                                          int dimension) {
    int retVal = 0;

    for (int i = 0; i < dimension; i++) {
        if (size[i] <= 0) continue;
        if (index[i] >= 0 && index[i] < size[i]) {
            retVal = retVal * size[i];
            retVal += index[i];
        } else
            return INT_MIN;  // out of space
    }

    return retVal;
}

/**
 * This method returns the rank of the node that contains the Place
 * indicated by the index passed in (globalLinearIndex).
 *
 * @param globalLinearIndex the absolute, single-dimensional index of a
 * Place
 * @return                  the rank of the node that contains this Place
 */
int Places_base::getRankFromGlobalLinearIndex(int globalLinearIndex) {
    static int total = 0;
    static int stripe = 0;

    if (total == 0) {
        // first time computation
        total = 1;
        for (int i = 0; i < dimension; i++) total *= size[i];
        stripe = total / MASS_base::systemSize;
    }

    int rank, scope;
    for (rank = 0, scope = stripe; rank < MASS_base::systemSize;
         rank++, scope += stripe) {
        if (globalLinearIndex < scope) break;
    }

    return (rank == MASS_base::systemSize) ? rank - 1 : rank;
}



/*Elias --->=========================================================================================
 *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

 *++++++++++++++++++++++++++ Added for Graph features ++++++++++++++++++++++++++++++++++++++++++++++++

 *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *====================================================================================================*/

/**
 * Constructor for Place Objects for remote nodes. These Objects encompass the basic
 * features that are found in all Places Objects, enabling MASS to be able to
 * operate across all Places in a unified manner, regardless of each individual
 * Place instantiation.
 *
 *@filename                  the name of the file the vertices are rad from.
 *@param filetypes           the file type as enums(MATSim, HIPPIE, TXT etc...)
 *@argument                  the argument to instantiate the Place objects from.
 
 */
void Places_base::init_all_graph_for_worker_nodes(std::string filename, FILE_TYPE_ENUMS filetype, void* argument){


    this->places_base_distributed_map = new std::unordered_map<string, int>();
    places_base_distributed_map = MASS_base::getDistributedMap(handle);
    
    if(places_base_distributed_map->size() <= 0){
        MASS_base::log(" size of  the distributed_map can not be zero");
        exit(-1);
    }
    
    if(printOutput){
        string inputFileType = FileParser::fromEnumToString(fileType);
        ostringstream convert;
        convert << "Places_base::init_all_graph_for_worker_nodes method called:\n "
        << "handle = " << handle << ", class name = " << className
        << ", filename = " << filename << ", filetype = " << inputFileType 
        << ", argument = " << (char*)argument << ", argument size = " << sizeof(argument)
        << ", size of distributed_map = " << places_base_distributed_map->size() << endl;
        MASS_base::log(convert.str());
    }

    size = new int[dimension];
    for(int i = 0; i < dimension; i++) {
        size[i] = places_base_distributed_map->size();
    }
     
    int total = 1; 
    //fold total for multiple dimenstion graph
    for(int i = 0; i < dimension; i++){
            total *= size[i];
    }
    // load the construtor and destructor
    DllClass *dllclass = new DllClass(this->className);
  
    MASS_base::dllMap.insert(map<int, DllClass *>::value_type(handle, dllclass));  

     // calculate lower_boundary and upper_boundary  
    int stripeSize = total / MASS_base::systemSize;

    // lower_boundary is the first place managed by this node
    lower_boundary = stripeSize * MASS_base::myPid;

    // upperBoundary is the last place managed by this node
    upper_boundary = (MASS_base::myPid < MASS_base::systemSize - 1)
                         ? lower_boundary + stripeSize - 1: total - 1;

    this->places_size = upper_boundary - lower_boundary + 1;
    //MASS_base::log("after places_size calculated -----------------------------");

    if(places_size <= 0){

        if(printOutput){
            ostringstream convert;
            convert << "Place_size <= 0, a minimum of 1 required for this node to continue." << endl;
            MASS_base::log(convert.str());
            exit(-1);
        }
    }

    //  maintaining an entire set 
    std::vector<int> indexVector;
    indexVector.reserve(dimension);
     dllclass->places = new Place *[places_size]; 

    // initialize all Places objects
    for (int i = 0; i < places_size; i++) {
        // instanitate a new place         
        indexVector = getGlobalArrayIndex(lower_boundary + i);   
         //ostringstream convert;
         //convert << " inside for loop iteration i = " << i << ", vertexPlace global Id = " << indexVector.at(0) << endl; 
         //MASS_base::log(convert.str());
        dllclass->places[i] = (Place *)(dllclass->instantiate_from_file(filename, filetype, 
                               indexVector.at(0), argument,  places_base_distributed_map));
        //MASS_base::log("inside the foor loop after instantiate_from_file factory  method ---------------");         
        dllclass->places[i]->size.reserve(dimension);
        for (int j = 0; j < dimension; j++)
            // define size[] and index[]
            dllclass->places[i]->size.push_back(size[j]);  
         dllclass->places[i]->index =  indexVector;            
    }

    // allocate the left/right shadows
    if (boundary_width <= 0) {
        // no shadow space.
        shadow_size = 0;
        dllclass->left_shadow = NULL;
        dllclass->right_shadow = NULL;
        return;
    }
    shadow_size =(dimension == 1) ? boundary_width : total / size[0] * boundary_width;
    if (printOutput == true) {
        ostringstream convert;
        convert << "Places_base.shadow_size = " << shadow_size;
        MASS_base::log(convert.str());
    }
    dllclass->left_shadow =
        (MASS_base::myPid == 0) ? NULL : new Place *[shadow_size];
    dllclass->right_shadow = (MASS_base::myPid == MASS_base::systemSize - 1)
                                 ? NULL
                                 : new Place *[shadow_size];

    // initialize the left/right shadows
    std::vector<int> shadowIndex;
    shadowIndex.reserve(dimension);
    for (int i = 0; i < shadow_size; i++) {
        // left shadow initialization
        if (dllclass->left_shadow != NULL) {
            // instanitate a new place
            shadowIndex = getGlobalArrayIndex(lower_boundary - shadow_size + i);
            dllclass->left_shadow[i] = (Place *)(dllclass->instantiate_from_file(filename, filetype,
                                        shadowIndex.at(0), argument, places_base_distributed_map));
            dllclass->left_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->left_shadow[i]->size.push_back(size[j]);
            }
            dllclass->left_shadow[i]->index =  shadowIndex;
                
            dllclass->left_shadow[i]->outMessage = NULL;
            dllclass->left_shadow[i]->outMessage_size = 0;
            dllclass->left_shadow[i]->inMessage_size = 0;
        }

        // right shadow initialization
        std::vector<int> righShadowIndex;
        righShadowIndex.reserve(dimension);
        if (dllclass->right_shadow != NULL) {
            // instanitate a new place
            righShadowIndex = getGlobalArrayIndex(upper_boundary + i);
            dllclass->right_shadow[i] =
                (Place *)(dllclass->instantiate_from_file(filename, filetype, righShadowIndex.at(0),
                                                         argument, places_base_distributed_map));
            dllclass->right_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->right_shadow[i]->size.push_back(size[j]);
            }
            dllclass->right_shadow[i]->index = righShadowIndex;
                
            dllclass->right_shadow[i]->outMessage = NULL;
            dllclass->right_shadow[i]->outMessage_size = 0;
            dllclass->right_shadow[i]->inMessage_size = 0;
        }

    }
    if(printOutput){
        ostringstream convert;
        convert << "Places_base: init_all_graph_for_worker_nodes furnished "
        << "number of vertices read " << MASS_base::getDistributedMap(handle)->size()
        << ", places_size " << places_size << endl;
        MASS_base::log(convert.str());
    }

}



/*Elias --->=========================================================================================
                        ***** init_all_graph *****    
 *====================================================================================================*/

/**
 * Constructor for Place Objects for reading vertices from the given filename
 * and populate Place ojects into MASS_base's Place map,
 *a features that are found in all Places Objects, enabling MASS to be able to
 * operate across all Places in a unified manner, regardless of each individual
 * Place instantiation.
 *
 *@filename                  the name of the file the vertices are rad from.
 *@param filetypes           the file type as enums(MATSim, HIPPIE, TXT etc...)
 *@argument                  the argument to instantiate the Place objects from.
 
 */
void Places_base::init_all_graph(std::string filename, FILE_TYPE_ENUMS filetype, void* argument){
   
    if(MASS_base::myPid != 0){
        init_all_graph_for_worker_nodes(filename,filetype, argument);
        return;
     }

    //only the master will read the vertices from file and populate the distributed map
    places_base_distributed_map = new std::unordered_map<string, int>();
    int totalVertex = FileParser::open(filename, filetype, this->places_base_distributed_map);
    

    MASS_base::distributed_map.insert({handle, places_base_distributed_map});
    if (printOutput) {
        ostringstream convert;  
        string filetypeString = FileParser::fromEnumToString(filetype);

        convert << "Places_base: init_all_graph() ---> handle = " << handle << ", class = " << className
        << ", dimension = " << dimension <<", filetype enum = " << filetypeString << ", #vertices read = " 
         << places_base_distributed_map->size() <<endl;        
        MASS_base::log(convert.str());
    }


    if(totalVertex <= 0){
        if(printOutput){
            ostringstream convert;
            convert << "graph initialization failed: unable to open and read Vertices into MASS::distributed_map."
            <<" Make sure passing the full path of the correct file name" 
            << " and the correct file type from the fileType enums." << endl;
            MASS_base::log(convert.str());
        }

        exit(-1);
   }
    /** size --> Defines the size of each dimension in the simulation space. Intuitively,
     * size[0], size[1], and size[2] correspond to the size of x, y, and z, or
     * that of i, j, and k.*/
    this->size = new int[dimension];
     for (int i = 0; i < dimension; i++)
          this->size[i] = totalVertex;

    // load the construtor and destructor
    DllClass *dllclass = new DllClass(this->className);
  
    MASS_base::dllMap.insert(map<int, DllClass *>::value_type(handle, dllclass));  
   
   int total = 1; 
    //fold total for multiple dimenstion graph
    for(int i = 0; i < dimension; i++){
            total *= size[i];
    }
     // calculate lower_boundary and upper_boundary  
    int stripeSize = total / MASS_base::systemSize;

    // lower_boundary is the first place managed by this node
    lower_boundary = stripeSize * MASS_base::myPid;

    // upperBoundary is the last place managed by this node
    upper_boundary = (MASS_base::myPid < MASS_base::systemSize - 1)
                         ? lower_boundary + stripeSize - 1: total - 1;

    this->places_size = upper_boundary - lower_boundary + 1;


    if(places_size <= 0){

        if(printOutput){
            ostringstream convert;
            convert << "Place_size <= 0, a minimum of 1 required for this node to continue." << endl;
            MASS_base::log(convert.str());
            exit(-1);
        }
    }

    //  maintaining an entire set 
     dllclass->places = new Place *[places_size];
     vector<int> index;
     index.reserve(dimension);
     ostringstream convert;
    // initialize all Places objects
    for (int i = 0; i < places_size; i++) {
        index = getGlobalArrayIndex(lower_boundary + i);

        // instanitate a new place
        dllclass->places[i] = (Place *)(dllclass->instantiate_from_file(filename, fileType, 
                              index.at(0), argument, places_base_distributed_map));
        dllclass->places[i]->size.reserve(dimension);
        for (int j = 0; j < dimension; j++)
            // define size[] and index[]
            dllclass->places[i]->size.push_back(size[j]);
        dllclass->places[i]->index = index;
    }
    // allocate the left/right shadows
    if (boundary_width <= 0) {
        // no shadow space.
        shadow_size = 0;
        dllclass->left_shadow = NULL;
        dllclass->right_shadow = NULL;
        return;
    }
    shadow_size =(dimension == 1) ? boundary_width : total / size[0] * boundary_width;
    if (printOutput == true) {
        ostringstream convert;
        convert << "Places_base.shadow_size = " << shadow_size;
        MASS_base::log(convert.str());
    }
    dllclass->left_shadow =
        (MASS_base::myPid == 0) ? NULL : new Place *[shadow_size];
    dllclass->right_shadow = (MASS_base::myPid == MASS_base::systemSize - 1)
                                 ? NULL
                                 : new Place *[shadow_size];
    // initialize the left/right shadows                                 
    std::vector<int> shadowIndex;
    shadowIndex.reserve(dimension);
    for (int i = 0; i < shadow_size; i++) {
        // left shadow initialization
        if (dllclass->left_shadow != NULL) {
            // instanitate a new place
            shadowIndex = getGlobalArrayIndex(lower_boundary - shadow_size + i);
            dllclass->left_shadow[i] = (Place *)(dllclass->instantiate_from_file(filename, filetype,
                                        shadowIndex.at(0), argument, places_base_distributed_map));
            dllclass->left_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->left_shadow[i]->size.push_back(size[j]);
            }
            dllclass->left_shadow[i]->index =  shadowIndex;
                
            dllclass->left_shadow[i]->outMessage = NULL;
            dllclass->left_shadow[i]->outMessage_size = 0;
            dllclass->left_shadow[i]->inMessage_size = 0;
        }
        // right shadow initialization
        std::vector<int> righShadowIndex;
        righShadowIndex.reserve(dimension);
        if (dllclass->right_shadow != NULL) {
            // instanitate a new place
            righShadowIndex = getGlobalArrayIndex(upper_boundary + i);
            dllclass->right_shadow[i] =
                (Place *)(dllclass->instantiate_from_file(filename, filetype, righShadowIndex.at(0),
                                                         argument, places_base_distributed_map));
            dllclass->right_shadow[i]->size.reserve(dimension);
            for (int j = 0; j < dimension; j++) {
                // define size[] and index[]
                dllclass->right_shadow[i]->size.push_back(size[j]);
            }
            dllclass->right_shadow[i]->index = righShadowIndex;
                
            dllclass->right_shadow[i]->outMessage = NULL;
            dllclass->right_shadow[i]->outMessage_size = 0;
            dllclass->right_shadow[i]->inMessage_size = 0;
        }

    }
    if(printOutput){
        ostringstream convert;
        convert << "Places_base: init_all_graph furnished "
        << "number of vertices read " << places_base_distributed_map->size()
        << ", places_size " << places_size << endl;
        MASS_base::log(convert.str());
    }
}


/**
This method deletes Graph related data on the cluster.

*/
void Places_base::clearGraphOnTheCluster(){
    MASS_base::reinitializeMap();//dump the Graph related info
    places_size = 0;
    //consider removing other Graph related data too(that lives in one of MASS_base containers )
}

/** 
 *This method adds a vertex to the local machine.

 *Given vertexId and argument along with size of argument to construct VertexPlace object,
 *this method creates the object and inserts it to the simulation space
 *@param vertexId           name of the vertex to be inserted
 *@param argument           to construct VertexPlace obect
 *@arg_size                 size of the argument
 *@return                   positive number if inserted, -1 otherwise

 */
int Places_base::addPlaceLocally(string vertexId, void* argument, int arg_size){
    int total = 1;
    
    for(int i = 0; i < dimension; i++){
        total *= Places_base::size[i];
    }
    
    //[0] ->nextPlaceIndex/total, [1] -> nextPlaceIndex % total
    int* placesIndex = getPlacesIndex();
     
    int stripSize = total / MASS_base::systemSize;
    int lowerBound = stripSize * MASS_base::myPid * placesIndex[0];
    int upperBound = lowerBound + stripSize;

    if((int)placesVector.size() == 0 || (int)placesVector.size() < placesIndex[0]){
        std::vector<VertexPlace*>  v (stripSize);
        placesVector.push_back(v);
    }

    else if(placesIndex[1] < lowerBound || placesIndex[1] >= upperBound){
        ostringstream convert;      
        convert << " GreaphPlaces--> AddPlaceLocally() --> unable to add vertex. out of bound error raised. " << endl;      
        MASS_base::log(convert.str());
        return -1;
    }

    // load the construtor and destructor
    DllClass* dllclass = new DllClass(this->className);

    VertexPlace * vertexPlace = (VertexPlace *)(dllclass->instantiate(argument));

    vertexPlace->setVertexName(vertexId);
    int globalIndex = total + stripSize * MASS_base::myPid + nextPlaceIndex;    
    vertexPlace->setGlobalId(globalIndex);

    
    std::vector<VertexPlace*> &v  = placesVector.at(placesIndex[0]);
    v.insert(v.begin() + placesIndex[1], vertexPlace);
    places_base_distributed_map->insert({vertexId, globalIndex});
    nextPlaceIndex++;

    if(printOutput){
        ostringstream convert;
        convert << "GraphPlaces--> addPlaceLocally()-->Place object inserted into the simualtion space "
        <<" with globalIndex of " << globalIndex << endl;
        MASS_base::log(convert.str());
    }
    return globalIndex;
}

/**
 * This method creates edges between two vertices

 *create an edge between two vertex with the given weight on the local machine.

 *@pama vertexId        The string representation of the vertex.
 *@pama neighborId      the name of the neighbor
 *@pama weight          the weight of the connection between the two vertices
 *@return               true if edge formed, false otherwise

 */
bool Places_base::addEdgeLocally(string vertexId, string neighborId, double weight){
    unordered_map<string, int>*my_disMap = getThisDistributedMap(handle);
    auto it  = my_disMap->find(vertexId);
    int globalIndex = it->second;
    int total = 1;
    for(int i = 0; i < dimension; i++){
        total *= Places_base::size[i];
    }

    int strip = total / MASS_base::systemSize;
    int placesIndex = globalIndex / total - 1;

    //determine the position to insert the neighbor and the connection 
    if(placesIndex >= 0 && (int)placesVector.size() >= 0 && (int)placesVector.size() > placesIndex){
        int localPlaceIndex =globalIndex % strip;
        VertexPlace *vplace = placesVector.at(placesIndex).at(localPlaceIndex);
        vplace->addNeighbor(neighborId, weight);

        if(printOutput){
            ostringstream convert;
            convert << "GraphPaces-->addEdgeLocally() ---> Edge formed between " << vertexId << " and "
            << neighborId << " with weight = " << weight << endl;
            MASS_base::log(convert.str());
        }
        return true;
    }

    if(printOutput){
        ostringstream convert;
        convert << "GraphPaces-->addEdgeLocally() ---> Edge cound not be formed between " << vertexId <<" and "
        << neighborId << " !! " << endl;
        MASS_base::log(convert.str());
    }
    return false;
}


/**
 *this method removes the given vertex from the local machine

 *if the given vertex found on the local machine, this method removes the vertex
 *and returns true, false otherwise.
 *@param vertexId            the unique name of the vertex
 *@return                    true if the vertex found removed, false otherwise

 */
bool Places_base::removeVertexLocally(string vertexId){
    auto it = places_base_distributed_map->find(vertexId);
    if(it == places_base_distributed_map->end()){
        if(printOutput){
            ostringstream convert;
            convert << "vertex required to be removed doesn't exist on the MASS::distributed_map." << endl;
            MASS_base::log(convert.str());
        }
        return false;
    }
    
    //get the global index 
    int globalIndex = it->second;
    VertexPlace *vrtxplace = nullptr;
    // Remove this vertex and its neighbor from the simulation space on this node
    for (int i = 0; i < (int)placesVector.size(); i++){ //std::vector<VertexPlace*> &places :placesVector) 
        vector<VertexPlace*> &places = placesVector.at(i);
        for (int j = 0; j < (int)places.size(); j++) {
            VertexPlace *singlePlace  = nullptr;
            singlePlace = places.at(j);

            if (singlePlace != nullptr && singlePlace->getGlobalIndex() == globalIndex) {
                vrtxplace = singlePlace;
                //singlePlace->clearAllNeighborsAndWeight();
            }
            if(singlePlace)singlePlace->removeNeighbor(vertexId);
        }         
        //memory leak proof removal of VertexPlace pointers.
        if (vrtxplace != nullptr) {
            for(int k = 0 ; k < (int)places.size(); k++){

                if(places.at(k)->getVertexName() == vrtxplace->getVertexName() ||
                    places.at(k)->getGlobalIndex() == vrtxplace->getGlobalIndex()){
                    
                    if(places.at(k) != nullptr){
                        delete places.at(k);
                        places.at(k) = nullptr;
                    }
                    
                }
            }
            auto iteratorBegin = std::remove(places.begin(), places.end(), nullptr);
            places.erase(iteratorBegin, places.end());
        }
    }
   
    return true;
}

/**
 *this method removes an edge between two neighbors on a local machine.

 *The method finds the two vertices and remves the edge between them if exists and returns
 *true, returns false othewise.
 *@param vertexId           the principal vertex
 *@param neighborId         the neighbor vertex
 *@return                   true for successful removal of edge, false otheriwse.

 */
bool Places_base::removeEdgeLocally(string vertexId, string neighborId){
    unordered_map<string, int> *dist_map = getThisDistributedMap(handle);
    auto it = dist_map->find(vertexId);
    int globalIndex = it->second;
    int total = 1;

    for(int i = 0; i < Places_base::dimension; i++){
        total = Places_base::size[i];
    }

    int strip = total / MASS_base::systemSize;

    int placesIndex = globalIndex / total - 1;

    if(placesIndex >= 0 && (int)placesVector.size() >= 0 && (int)placesVector.size() > placesIndex){
         int localPlaceIndex = globalIndex % strip;

         VertexPlace *place = placesVector.at(placesIndex).at(localPlaceIndex);
         place->removeNeighbor(neighborId);

         if(printOutput){
            ostringstream convert;
            convert << "GraphPlaces-->removeEdgeLocally()--> edge  between " 
            << vertexId << " and " << neighborId << " removed "<< endl;
            MASS_base::log(convert.str());
        }
        return true;
    }
    else{
        if(printOutput){
            ostringstream convert;
            convert << "GraphPlaces-->removeEdgeLocally()--> couldn't remove edge between " 
            << vertexId << " and " << neighborId << endl;
            MASS_base::log(convert.str());
        }
        return false;
    }
}

/**
 *This method helps neighbors to exchange info.

 *given a function and the index of a particular Place object,
 *this method returns the result of excecution of the method.
 *@param functionId             the unuque ID of the function,
 *@param neighbor               the vector of indices of a particular Place object.
 *@param argument               a void pointer argument for the function
 *@return                       void 

 */
void* Places_base::exchangeNeighbor(int functionId, vector<int> neighbor, void* argument){

    int indexFound = -1;
    VertexPlace* vertex;
    for(int i = 0; i < (int)placesVector.size(); i++){
        vector<VertexPlace*> vp = placesVector.at(i);
        for(int j = 0; j < (int)vp.size(); j++){
            VertexPlace* place = vp.at(j);
            if(neighbor.at(0) == place->index.at(0)){
                indexFound = 0;
                vertex = place;
                break;
            }           
        }
    }
    if(indexFound != -1){
        return vertex->callMethod(functionId, argument);
    }
    return NULL;    
}

int* Places_base::getPlacesIndex(){
    int total = 1;
    if(this->dimension > 1){
        for(int i = 0; i < this->dimension; i++){
            total *= this->size[i]; 
        }
    }
    else{
        total = this->size[0];
    }

    int* array = new int[2];

    array[0] = (nextPlaceIndex / total); 
    array[1] = (nextPlaceIndex % total);

    return array;
}

/**
 *this method returns the distributed_map(storage of vertices) for a particular 
 *GraphPlace instance, given the handle as an argument.

 */
unordered_map<string, int>* Places_base::getThisDistributedMap(int handle){
    auto myMap = MASS_base::getDistributedMap(handle);
    if(myMap == NULL){
        return NULL;
    }
    return myMap;
}


/**
 *given the vertex name as a string, this method returns the Place(VertexPlace pointer),
 *The place might exist either in the master node or in one of remote nodes, the location
 *of the vertex is hidden from the applciation view point

 */
VertexPlace* Places_base::getPlaceFromVertexName(string vertexname){

    unordered_map<string, int> *myMap = getThisDistributedMap(handle);

    //is the vertex name in the distributed map? if yes, find the place object on the 
    //master node or on the remote nodes, otherwise return NULL.
    std::unordered_map<std::string,int>::const_iterator found = myMap->find (vertexname);
    DllClass * dllclass  = nullptr;
    if(found != myMap->end()){
        
        auto it = MASS_base::dllMap.find(handle);

        DllClass * dllclass = it->second;
        
        for(int i = 0; i < places_size; i++){
            VertexPlace* place = (VertexPlace*)dllclass->places[i];
            if(place->getVertexName() == vertexname){
                return place;
            }
        }            
    }
    for(auto vecOfPlace:placesVector){
        for(auto vertexPlace:vecOfPlace){
            if(vertexPlace->getVertexName() == vertexname){
                return vertexPlace;
            }
        }
    }
    //vertex cound't found in the cluster
    return NULL;
}


//discar the current Places information 
void Places_base::deleteAndRenitializeGraph() {
    clearGraphOnTheCluster();
    this->nextPlaceIndex = 0;
    this->placesVector.clear();

    //Send reinitialize message to all remote nodes
    //Message(ACTION_TYPE action, int handle, int dummy);
    
    Message *message = new Message(Message::MAINTENANCE_REINITIALIZE, getHandle(), 0);
    // send a MAINTENANCE_REINITIALIZE message to each worker node
    // for (int i = 0; i < int(MASS::mNodes.size()); i++) {
    //     MASS::mNodes[i]->sendMessage(message);

    //     if (printOutput == true) {
    //         cerr << "PLACES_INITIALIZE sent to rank " << MASS::mNodes[i]->getHostName() << endl;
    //     }
    // }
    delete message;
    // Synchronized with all slave processes
   // MASS::barrier_all_slaves();

}


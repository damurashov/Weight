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
#include <algorithm>

#include "Place.h"
#include "VertexPlace.h"
#include "FileParser.h"

using namespace std;

class Places_base{
    friend class MProcess;
    friend class Agents_base;
    friend class Place;
    friend class VertexPlace;

   public:


    /*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     **-------------------Elias: Added for graph features-------------------------------------------------------
     *+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    Places_base(int handle, string className,int boundary_width,int dimension, string filename,
                     FILE_TYPE_ENUMS type, void* argument, int arg_size);

  
    Places_base (int handle, string className, int boundary_width, int dimension,void* argument, int arg_size, int nVertices);

    //Places_base(int handle, string className, int boundary_width, int dimension, void* argument, int argument_size);
    
    /*--------------------------------------------------------------------------------------------------------------
     +++++++++++++++++ End of graph feature constructors ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     ----------------------------------------------------------------------------------------------------------------*/

    Places_base(int handle, string className, int boundary_width, void *argument, int argument_size,
                     int dim, int size[]);
    virtual ~Places_base();

    void callAll(int functionId, void *argument, int tid);
    void **callAll(int functionId, void *argument, int arg_size, int ret_size,
                   int tid);
    void exchangeAll(Places_base *dstPlaces, int functionId,
                     vector<int *> *destinations, int tid);
    // exchangeAll that relies on neighbor functionality
    void exchangeAll(Places_base *dstPlaces, int functionId, int tid);
    void exchangeBoundary();  // called from Places.exchangeBoundary( )
    void setAllPlacesNeighbors(Places_base *dstPlaces,
                               vector<int *> destinations, int tid);
    int getHandle() { return handle; };
    int getPlacesSize() { return places_size; };
    void callSome(int functionId, void *argument, int tid);
    void **callSome(int functionId, void *argument, int arg_size, int ret_size,
                    int tid);
    int* getSize(){return this->size;};
    int getLowerBoundary(){return lower_boundary;};
    int getDimension(){return dimension;};

    /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
     *----Elias --> Added for parallel I/O to calculate total places -----------------
     @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
    int getTotalPlaces(){
        int total = 1;
        for(int i = 0; i < dimension; i++){
            total *= size[i];
        }
        return total;

    };

   protected:
    const int handle;  // handle
    const string className;
    const int dimension;

    /**
     * contains map of vertices  
     */
    std::unordered_map<string, int>* places_base_distributed_map = NULL;

    //seter of the map
    void setDistributedMap(std::unordered_map<string, int> *my_dist_map){
        places_base_distributed_map = my_dist_map;
    };

    //getter of the map
     std::unordered_map<string, int>* getDistributedMap(){
        return places_base_distributed_map;
    };

    int getPlaces_Size(){
        return places_size;
    };


    /**
     * The smallest, valid index present in a given stripe of the simulation
     * space
     */
    int lower_boundary;

    /**
     * The largest, valid index present in a given stripe (any remainders from
     * stripe calculation will be included in final stripe - may be larger than
     * other stripes)
     */
    int upper_boundary;

    /**
     * The total number of Places that exist within this stripe of the
     * simulation
     */
    int places_size;

    /**
     * Defines the size of each dimension in the simulation space. Intuitively,
     * size[0], size[1], and size[2] correspond to the size of x, y, and z, or
     * that of i, j, and k.
     */
    int *size;

    /*this member variable holds the last index taken by the place object.
    the next place should have an index after this one in the simulation space there for
    each places have a distict index in the global simulation space. */
    int nextIndex [];//Note--> not used yet

    /**
     * Function of the simulation size and the boundary_width defined. This
     * value stores the total number of Place Objects that constitute the shadow
     * for boundaries (can think of as overlap)
     */
    int shadow_size;

    /**
     * Width of any boundaries between stripes in simulation
     */
    int boundary_width;


    
    /*======================================================================================================
     * +++++++++++++++++++++++++++++ Elias-->Added for Graph feature +++++++++++++++++++++++++++++++++++++++
     *======================================================================================================*/
    
    //memebers for Graph Features
    const FILE_TYPE_ENUMS fileType;
    const std::string filename;
    const int numVertices;//number of vertices
    int nextPlaceIndex = 0;
    std::vector<std::vector<VertexPlace*> > placesVector; // Graph maintenance

    
    void init_all_graph(string filename, FILE_TYPE_ENUMS type, void* argument);
    void init_all_graph_for_worker_nodes(std::string filename, FILE_TYPE_ENUMS filetype, void* argument);
    void clearGraphOnTheCluster();    
    int* getPlacesIndex();
    unordered_map<string, int>* getThisDistributedMap(int handle);
    
    void init_all(void *argument, int argument_size);
    vector<int> getGlobalArrayIndex(int singleIndex);
    vector<int> getGlobalArrayIndex(int index, int dim);
    void getLocalRange(int range[], int tid);
    static void *processRemoteExchangeRequest(void *param);
    void getGlobalNeighborArrayIndex(vector<int> src_index, int offset[],int dst_size[], int dst_dimension,int *dest_index);
    int getGlobalLinearIndexFromGlobalArrayIndex(int dest_index[],int dest_size[], int dest_dimension);
    int getRankFromGlobalLinearIndex(int globalLinearIndex);
    static void *exchangeBoundary_helper(void *param);
    static void *sendMessageByChild(void *param);
    struct ExchangeSendMessage {
        int rank;
        void *message;
    };


private:
     /*======================================================================================================
     *+++++++++++++++++++++++++++++ Elias-->Added for Graph feature addition +++++++++++++++++++++++++++++++++
     *======================================================================================================*/
    
    int addPlaceLocally(string vertexId, void* argument, int arg_size);
    bool addEdgeLocally(std::string vertexId, std::string neighborId, double weight);
    bool removeVertexLocally(string vertexId);
    bool removeEdgeLocally(std::string vertexId, std::string neighborId);
    void* exchangeNeighbor(int functionId, vector<int> neighbor, void*argument);
    VertexPlace* getPlaceFromVertexName(string vertexname);
    void deleteAndRenitializeGraph();

    /*======================================================================================================
     * +++++++++++++++++++++++++++++ Elias-->end of  Graph feature addition +++++++++++++++++++++++++++++++++
     *======================================================================================================*/

};

#endif

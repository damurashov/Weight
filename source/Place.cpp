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

#include "Place.h"
#include <iostream>
#include <sstream>  // ostringstream

#include "MASS_base.h"
#include "limits.h"
#include "Places_base.h"

// Used to toggle comments from Places.cpp
#ifndef LOGGING
const bool printOutput = true;
#else
const bool printOutput = true;
#endif

/**
 * Returns the size of the matrix that consists of application-specific
 * places. Intuitively, size[0], size[1], and size[2] correspond to the size
 * of x, y, and z, or that of i, j, and k.
 *
 * @return  a collection of sizes that correspond to each dimension in the
 *          simulation space
 */
vector<int> Place::getSizeVect() { return size; }

/**
 * Method to retrieve the location (pointer) of a Place within the simulation.
 * The Place returned will exist relative to the Place represented by this
 * Object, using the 'handle' to ensure that they both live within the same
 * simulation space, and the value(s) passed as offset to determine the relative
 * location of the Place to retrieve.
 *
 * @param handle  unique identifier for a set of Places in this simulation
 * @param offset  relative location of destination (Place) to retrieve output
 *                message from
 * @return        pointer to output message for this Place, else NULL
 */
Place *Place::findDstPlace(int handle, int offset[]) {
    // compute the global linear index from offset[]
    Places_base *places = MASS_base::placesMap[handle];
    int neighborCoord[places->dimension];
    places->getGlobalNeighborArrayIndex(index, offset, places->size,
                                        places->dimension, neighborCoord);
    int globalLinearIndex = places->getGlobalLinearIndexFromGlobalArrayIndex(
        neighborCoord, places->size, places->dimension);
    /*
    ostringstream convert;
    convert << "globalLinearIndex = " << globalLinearIndex << endl;
    MASS_base::log( convert.str( ) );
    */

    if (globalLinearIndex == INT_MIN) return NULL;

    // identify the destination place
    DllClass *dllclass = MASS_base::dllMap[handle];
    int destinationLocalLinearIndex =
        globalLinearIndex - places->lower_boundary;

    Place *dstPlace = NULL;
    int shadow_index;
    if (destinationLocalLinearIndex >= 0 &&
        destinationLocalLinearIndex < places->places_size)
        dstPlace = dllclass->places[destinationLocalLinearIndex];
    else if (destinationLocalLinearIndex < 0 &&
             (shadow_index =
                  destinationLocalLinearIndex + places->shadow_size) >= 0)
        dstPlace = dllclass->left_shadow[shadow_index];
    else if ((shadow_index =
                  destinationLocalLinearIndex - places->places_size) >= 0 &&
             shadow_index < places->shadow_size)
        dstPlace = dllclass->right_shadow[shadow_index];

    return dstPlace;
}

/**
 * Checks if a given neighbor exists in a Place's list of neighbors.
 * @param destination - The neighbor to check for
 * @return  true if the neighbor exists, false if it does not.
 */
bool Place::isNeighbor(int *destination) {
    for (unsigned int i = 0; i < neighbors.size(); i++) {
        if (neighbors[i] == destination) {
            return true;
        }
    }
    return false;
}

/**
 *
 * @param handle
 * @param offset
 * @return
 */
void *Place::getOutMessage(int handle, int offset[]) {
    Place *dstPlace = findDstPlace(handle, offset);

    // return the destination outMessage
    return (dstPlace != NULL) ? dstPlace->outMessage : NULL;
}

/**
 * Method to store an input message within a given Place of the simulation.
 *
 * @param handle    unique identifier for a set of Places in this simulation
 * @param offset    relative location of destination (Place) to store input
 * message
 * @param position  numerical index to place input message (in message array)
 * @param value     pointer to value to store in message array
 */
void Place::putInMessage(int handle, int offset[], int position, void *value) {
    Place *dstPlace = findDstPlace(handle, offset);

    // fill out the space if inMessages are empty
    for (int i = 0; i <= position; i++)
        if (int(dstPlace->inMessages.size()) <= i &&
            dstPlace->inMessage_size > 0)
            dstPlace->inMessages.push_back(malloc(dstPlace->inMessage_size));

    // write to the destination inMessage[position]
    if (dstPlace != NULL && position < int(dstPlace->inMessages.size()))
        memcpy(dstPlace->inMessages[position], value, dstPlace->inMessage_size);
}
/**
 * clears the neighbor vector and fully clears the heap from any int[]'s stored
 * there
 */
void Place::cleanNeighbors() {
    for (unsigned int i = 0; i < neighbors.size(); i++) {
        if (neighbors.at(i)) {
            delete[](neighbors.at(i));
        }
    }
    neighbors.clear();
}

/**
 * replace neighbors completely using a vector of places
 */
void Place::addNeighbors(vector<int *> indexes) {
    cleanNeighbors();
    copy(indexes.begin(), indexes.end(), back_inserter(neighbors));
}
/**
 * Add a single neighbor to this place, neighbor vector should be created in the
 * heap, program will clean it up later, no need for the user to do so.
 */
void Place::addNeighbor(int *index, int dimension) {
    // check for redundant connections?
    // for (int i = 0; i < neighbors.size(); i++) {
    //     if (neighbors.at(i)) {
    //     }
    int tmp[dimension];
    for (int i = 0; i < dimension; i++) {
        tmp[i] = index[i];
    }
    neighbors.push_back(tmp);
}
/**
 * adds neighbors in a given pattern if they exist.
 */
void Place::addNeighbors(neighborPattern pattern) {
    cleanNeighbors();
    switch (pattern) {
        case MOORE2D: {
            neighbors = getMooreNeighbors2d();
            break;
        }
        case VON_NEUMANN2D: {
            neighbors = getVNNeighbors2d();
            break;
        }
        case MOORE3D: {
            neighbors = getMooreNeighbors3d();
            break;
        }
        case VON_NEUMANN3D: {
            neighbors = getVNNeighbors3d();
            break;
        }
        default:
            break;
    }
}

vector<int *> Place::getVNNeighbors2d() {
    vector<int *> result;
    // 4 neighbors for Mead
    int *x = new int[2]{1, 0};
    result.push_back(x);
    int *y = new int[2]{0, 1};
    result.push_back(y);
    int *_x = new int[2]{-1, 0};
    result.push_back(_x);
    int *_y = new int[2]{0, -1};
    result.push_back(_y);
    return result;
}
vector<int *> Place::getMooreNeighbors2d() {
    vector<int *> result;
    // 4 vectors covers all neighbors in 3d
    // 4 more neighbors for Moore
    result = getVNNeighbors2d();
    // 4 more for von neumann
    int *_xy = new int[2]{-1, 1};
    result.push_back(_xy);
    int *xy = new int[2]{1, 1};
    result.push_back(xy);
    int *_x_y = new int[2]{-1, -1};
    result.push_back(_x_y);
    int *x_y = new int[2]{1, -1};
    result.push_back(x_y);
    return result;
}
vector<int *> Place::getVNNeighbors3d() {
    vector<int *> result;
    // unit vectors only
    int *x = new int[3]{1, 0, 0};
    result.push_back(x);
    int *y = new int[3]{0, 1, 0};
    result.push_back(y);
    int *z = new int[3]{0, 0, 1};
    result.push_back(z);
    int *_x = new int[3]{-1, 0, 0};
    result.push_back(_x);
    int *_y = new int[3]{0, -1, 0};
    result.push_back(_y);
    int *_z = new int[3]{0, 0, -1};
    result.push_back(_z);
    return result;
}
vector<int *> Place::getMooreNeighbors3d() {
    vector<int *> result;
    // 26 neighbors for full coverage
    // unit vectors
    result = getMooreNeighbors2d();
    // x and y
    int *xy = new int[3]{1, 1, 0};
    result.push_back(xy);
    int *_x_y = new int[3]{-1, -1, 0};
    result.push_back(_x_y);
    int *_xy = new int[3]{-1, 1, 0};
    result.push_back(_xy);
    int *x_y = new int[3]{1, -1, 0};
    result.push_back(x_y);
    // y and z
    int *yz = new int[3]{0, 1, 1};
    result.push_back(yz);
    int *_y_z = new int[3]{0, -1, -1};
    result.push_back(_y_z);
    int *_yz = new int[3]{0, -1, 1};
    result.push_back(_yz);
    int *y_z = new int[3]{0, 1, -1};
    result.push_back(y_z);
    // x and z
    int *xz = new int[3]{1, 0, 1};
    result.push_back(xz);
    int *_x_z = new int[3]{-1, 0, -1};
    result.push_back(_x_z);
    int *_xz = new int[3]{-1, 0, 1};
    result.push_back(_xz);
    int *x_z = new int[3]{1, 0, -1};
    result.push_back(x_z);
    // all none zero
    int *xyz = new int[3]{1, 1, 1};
    result.push_back(xyz);
    int *_x_y_z = new int[3]{-1, -1, -1};
    result.push_back(_x_y_z);
    // single negative
    int *_xyz = new int[3]{-1, 1, 1};
    result.push_back(_xyz);
    int *x_yz = new int[3]{1, -1, 1};
    result.push_back(x_yz);
    int *xy_z = new int[3]{1, 1, -1};
    result.push_back(xy_z);
    // two negative
    int *_x_yz = new int[3]{-1, -1, 1};
    result.push_back(_x_yz);
    int *x_y_z = new int[3]{1, -1, -1};
    result.push_back(x_y_z);
    int *_xy_z = new int[3]{-1, 1, -1};
    result.push_back(_xy_z);
    return result;
}

/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

 *----Fucntionalities below this line are added to support Parallel IO feature --------------------------

 @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/

unordered_map<int, File*> Place::fileTable; 
int Place::allPlaceFileDescriptor = -1;
unordered_map<int, bool> Place::filesAttemptedToClose;
File* Place::writeFile = NULL;     

/**

 *A single Place opens a file specified by the given filePath and ioType. If ioType is 0 then the file
 * is opened for reading, if the ioType is 1 then the file is opened for writing (it is
 * expected that the ioType is either 0 or 1; otherwise, -1 is returned). A file that is opened for reading
 * will be opened in memory and added to the fileTable so that the file can be accessed by all Places. A
 * file that is opened for writing will be opened on the disk and a temporary buffer to write to is added to the
 * fileTable so that the temp buffer can be accessed by all Places. A successfully opened file is given a unique
 * file descriptor (integer) and the file descriptor is returned. An unsuccessfully opened file will result in
 * an exception being thrown
 *
 * @param filepath: the filepath of the file to be opened
 * @param ioType: READ_OR_WRITE either READ for read or WRITE for write
 * @return unique file descriptor for the newly opened file
 
 */
int Place::open(string filepath, READ_OR_WRITE ioType){
    std::lock_guard <std::mutex> lock (this->fileMutex);
    if(!fileTable.count(thisPlaceFileDescriptor)){
        openFile(filepath, ioType);
    }
    else {
        thisPlaceFileDescriptor++;
    }

    return allPlaceFileDescriptor;
 }

 //helper method of the 'open' method
void Place::openFile(string filepath, READ_OR_WRITE ioType){

    File *file = fileFactory(filepath);
    std::ios::openmode mode;
    if(file == NULL){
        MASS_base::log("invalid file extension");
        exit(-1);
    }
    if(ioType == READ_OR_WRITE::READ){
        mode = std::ios::in;
        file->open(mode); 
    }

    else{
        mode = std::ios::out;
        file->open(mode);
    }
    
    incrementFileDescriptors();
    fileTable.insert({allPlaceFileDescriptor, file});

    if(printOutput){
        ostringstream convert;
        convert << "size = "<< size[0] <<  "index = " << index [0] << "openedile =  " << filepath
        << "Node PID = " << MASS_base::myPid << endl;
        MASS_base::log(convert.str());
    }

}


//--------This method needs implementaion according to this specification--------------------.

/**
 * Reads a specific portion of the a NetCDF file's variable based on this place's order and returns the results
 * as a 1 dimensional array (i.e. char[]). Each place reads only a portion of the file, but if
 * each place calls this method, then the entire file will be read and parts of the data will be contained on
 * each place involved in the computation.
 *
 * @param fileDescriptor specifies the NetCDF file to read from (must be in the fileTable)
 * @param variableToRead specifies the NetCDF variable to read (must be in the NetCDF file)

 * @return a 1 dimensional vector of doubles representing a portion of the NetCDF variable data read by this
 * place - if an error occurs during the read process, then null is returned 

 */
vector<double> Place::read(int fileDescriptor, string variableToRead){
    std::vector<double> v;
    return v;//dummy return

}

/**
* Gets the file descriptor from the file table
 *@param fileDescriptor unique identifier for the file attribute to return
 *@return the file object pointer corresponding to the given file descriptor

 */
File* Place::getFileFromFileTable(int fileDescriptor){

    auto it = fileTable.find(fileDescriptor);
    if(it == fileTable.end()){
        if(printOutput){
            MASS_base::log("file descriptor not found");
        }
        return NULL;
    }

    File* file = it->second;
    return file;
}

//-------------Thsi method needs implementation according to the following specification-----------.

/*
 *Write a specific portion of the a NetCDF file's variable based on this place's order into a buffer
 * @param dataToWrite data to be written
 * @param variableName the NetCDF variable name to write
 * @param shape shape of the netCDF data to be written

 */
void Place::write(int fileDescriptor, double dataToWrite [], string variableName, int shape []){


}

/**
 * A single Place opens a file specified by the given filePath. If ioType is READ  then the file
 * is opened for reading, if the ioType is WRITE then the file is opened for writing (it is
 * expected that the ioType is either READ or WRITE; otherwise, -1 is returned). A file that is opened for reading
 * will be opened in memory and added to the fileTable so that the file can be accessed by all Places. A
 * file that is opened for writing will be opened on the disk and a temporary buffer to write to is added to the
 * fileTable so that the temp buffer can be accessed by all Places. A successfully opened file is given a unique
 * file descriptor (integer) and the file descriptor is returned. An unsuccessfully opened file will result in
 * an exception being thrown
 *
 * @param filepath the filepath of the file to be opened
 * @return unique file descriptor for the newly opened file 

 */
bool Place::openForWrite(string filepath, string variableName, int shape []){
     std::lock_guard <std::mutex> lock (this->fileMutex);
     if(writeFile == NULL){
        openFileUsingOnePlaceForWrite(filepath, variableName, shape);
     }
     bool fileIsNull = writeFile == NULL;
     return fileIsNull;
}

/*this method needs implementaion*/
 void Place::openFileUsingOnePlaceForWrite(string filepath, string variableName, int shape []){

 }



/**
 * Reads a specific portion of the a TXT file based on this place's order and returns the results
 * as a char array. Each place reads only a portion of the TXT file, but if each place calls this method, then the
 * entire file will be read and parts of the data will be contained on each place involved in the computation.
 *
 * @param fileDescriptor unique identifier for the file to read
 * @return the portion of the file read by this place - if an error occurs then vector of null is returned

 */
 vector<char> Place::read(int fileDescriptor){
    
    File *file = getFileFromFileTable(fileDescriptor);
    TxtFile *txtFile = (TxtFile*)file;
    int placeOrder = getPlaceOrderPerNode();
    return txtFile->read(placeOrder);

}

/**
 * A single Place opens a file specified by the given filePath (TxtFile).
 *
 * @param filepath the filepath of the file to be opened
 * @return unique file descriptor for the newly opened file 

 */
bool Place::openForWrite(string filepath, int size){
    std::lock_guard <std::mutex> lock (this->fileMutex);
    if(writeFile == NULL) {
        openFileUsingOnePlaceForWrite(filepath, size);
    }
    bool isNull = writeFile != NULL;
    return isNull;
}


/**
 * Opens the file only if the file has not been opened and added to the fileTable
 * @param filepath file to open for write 
 *@return void

 */
void Place::openFileUsingOnePlaceForWrite(string filepath, int size){
    IO_FILE_TYPE_ENUMS type = IO_FILE_TYPE_ENUMS::TXT;
    writeFile = new TxtFile(filepath, type);
    if(writeFile != NULL){
        TxtFile *txtWriteFile = (TxtFile*)writeFile;
        std::ios::openmode mode = std::ios::out;
        txtWriteFile->open(mode);
    }
}

/**
 * Write a specific portion of the a Txt file's variable based on this place's order into a buffer
 * @param dataToWrite data to be written

 */
void Place::write(int fileDescriptor, vector<char> dataToWrite){

    TxtFile *txtfile = (TxtFile*)writeFile;
    //void write(const std::vector<char> &vec, int &placeIndex);
    int placeIndex = getPlaceOrderPerNode();
    txtfile->write(dataToWrite, placeIndex);
}

/**
 * @return this place's order number determined by its index
 
 */
int Place::getPlaceOrderPerNode(){
    Places_base* pbase =  MASS_base::getCurrentPlaces();
    int lower_boundary = pbase->getLowerBoundary();
    int dimension = pbase->getDimension();
    int index [dimension];
    int size [dimension];
    for(int i = 0; i < dimension; i++){
        index[i] = this->index.at(i);
        size[i] = this->size.at(i);
    } 
    
    int globalIndex = pbase->getGlobalLinearIndexFromGlobalArrayIndex(index, size, dimension);
    return globalIndex - lower_boundary;
}
  
/**
 *Closes the specified file descriptor and removes it from the file table
 * @param fileDescriptor the file descriptor to close
 * @return true if the file is successfully found in the file table, closed, and removed; otherwise false

 */
bool Place::close(int fileDescriptor){
    std::lock_guard <std::mutex> lock (this->fileMutex);
    auto it2 = filesAttemptedToClose.find(fileDescriptor);
    if(writeFile != NULL) {
        writeFile->close(); 
    }
    auto it = fileTable.find(fileDescriptor);
    if(it != fileTable.end()){
        it->second->close();
        fileTable.erase(it);
        filesAttemptedToClose.insert({fileDescriptor, false});
        return true;
    }
    else if ( it2 != filesAttemptedToClose.end()) {
        bool isClosed = it2->second;
        return isClosed;
    } 
    else {
        return false;
    }

}

/**
 *Increment the filedescripted counter

 */
void Place::incrementFileDescriptors(){
    allPlaceFileDescriptor = thisPlaceFileDescriptor;
    thisPlaceFileDescriptor++;
}


/**
 *Given the filepath with a file name, this method returns the extension (.txt, .nc etc.)of the file

*/
std::string Place::getFileExtension(std::string filePath){
    // Find the last position of '.' in given string
    std::size_t pos = filePath.rfind('.');
    // If last '.' is found
    if (pos != std::string::npos) {
        // return the substring
        return filePath.substr(pos);
    }
    // In case of no extension return empty string
    return "";
}

/**
 *Depending on the file extension, this method creates the a pointer to 
 *File object from  either TxtFile or Netcdf */
File* Place::fileFactory(string filepath){
    string extension = getFileExtension(filepath);
    IO_FILE_TYPE_ENUMS type;
    if(extension == ".txt"){
        //const std::string &, const IO_FILE_TYPE_ENUMS &
        type = IO_FILE_TYPE_ENUMS::TXT;
        File * txt = (File*)new TxtFile(filepath, type);
        return txt;
    }
    else if(extension == ".nc"){
        //NetcdfFile(const std::string &filepath, const IO_FILE_TYPE_ENUMS & ioType)
        type = IO_FILE_TYPE_ENUMS::NETCDF;
        File *netcdf = (File*)new NetcdfFile(filepath, type);
        return netcdf; 
    }
    return NULL;
}

















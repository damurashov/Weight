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

#include "Message.h"
#include <iostream>
#include <sstream>  // ostringstream
#include "MASS_base.h"

// Used to toggle output in Messages
#ifndef LOGGING
const bool printOutput = false;
#else
const bool printOutput = true;
#endif

/**
 * Deconstructor for Message Objects. Will free the following resources, if they
 * are currently taking up dynamic memory space, before Message itself is
 * deleted:
 * <ol>
 *   <li>argument: array of arguments</li>
 *   <li>hosts: vector of hosts participating in computation</li>
 *   <li>destinations: vector of all destinations in exchangeAll</li>
 *   <li>RemoteExchangeRequest Objects held in exchangeReqList (does not delete
 * list)</li> <li>AgentMigrationRequest Objects held in migrationReqList (does
 * not delete list)</li>
 * </ol>
 */
Message::~Message() {
    if (argument_in_heap == true && argument != NULL) delete (char *)argument;
    //if(distributed_map != NULL)delete distributed_map;
    if (hosts != NULL) delete hosts;
    if (argument_in_heap == true && destinations != NULL) delete destinations;
    if (exchangeReqList != NULL) {
        while (exchangeReqList->size() > 0) {
            RemoteExchangeRequest *req = (*exchangeReqList)[0];
            exchangeReqList->erase(exchangeReqList->begin());
            delete req;
        }
        // exchangeReqList is MASS_base::remoteRequests[rank]
        // this should not be deleted.
    }
    if (migrationReqList != NULL) {
        while (migrationReqList->size() > 0) {
            AgentMigrationRequest *req = (*migrationReqList)[0];
            migrationReqList->erase(migrationReqList->begin());
            delete req;
        }
        // migrationReqList is MASS_base::migrationRequests[rank]
        // this should not be deleted.
    }
}

/**
 * Method to serialize the various Messages that can be communicated within a
 * MASS simulation (enumerated by Message::ACTION_TYPE Enum).
 *
 * @param msg_size  address of the int, which indicates the size of the message
 *                  to be serialized (and later sent)
 * @return          location of (pointer) the char message in serialized form
 */
char *Message::serialize(int &msg_size) {
    ostringstream convert;

    char *msg = NULL;
    char *pos = NULL;
    msg_size = sizeof(ACTION_TYPE);

    if (printOutput == true)
        cerr << "Message::serialize begin: action = " << action << endl;
    switch (action) {
        case EMPTY:
        case ACK:
            // calculate msg_size
            msg_size += sizeof(int);  // agent_population;

            if (argument != NULL && argument_size > 0) {
                msg_size += sizeof(int);    // argument_size;
                msg_size += argument_size;  // void *argument;
            }

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)msg = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = agent_population;
            pos += sizeof(int);  // agent_population

            if (argument != NULL && argument_size > 0) {
                *(int *)pos = argument_size;
                pos += sizeof(int);  // argument_size
                memcpy((void *)pos, argument, argument_size);
            }
            break;

        

        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case PLACES_INITIALIZE_GRAPH_FROM_FILE: /*Message::PLACES_INITIALIZE_GRAPH_FROM_FILE, size_vector,
                                                    handle, this->className, argument, arg_size, boundary_width,
                                                    tmp_hosts, fileType, filename, places_base_distributed_map*/
            // calculate msg_size
            msg_size += (sizeof(int) * (size->size() + 1));  // vector<int> size;
            msg_size += sizeof(int);                 // int handle;
            msg_size += sizeof(int);                 // classname.size( );
            msg_size += classname.size();            // classname
            msg_size += sizeof(int);                 // argument_size;
            msg_size += argument_size;               // void *argument
            msg_size += sizeof(int);                 // int boundary_width;
            msg_size += hosts->size();               // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                msg_size += sizeof(int);
                msg_size += (*hosts)[i].size();  // hosts[i]
            }

            msg_size += sizeof(int);                  // fileType.size( );
            msg_size += fileType.size();             // fileType
            msg_size += sizeof(int);                // filename.size( );
            msg_size += filename.size();            // filename
            msg_size += distributed_map->size();   //size of the map
            for(auto it = distributed_map->begin(); it != distributed_map->end(); it++){
                msg_size += sizeof(int);         //key.size()
                msg_size += (it->first).size();  // key
                msg_size += sizeof(int);        //value                
            }

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;  
            //cerr<< *(ACTION_TYPE *)pos << endl;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = size->size();  // cerr << *(int *)pos << endl;
            pos += sizeof(int);          // size.size( );
            for (int i = 0; i < (int)size->size(); i++) {
                *(int *)pos = (*size)[i];
                // cerr << *(int *)pos << endl;//
                pos += sizeof(int);  // size[i]
            }
            *(int *)pos = handle;
            //cerr << *(int *)pos << endl;//
            pos += sizeof(int);  // handle
            *(int *)pos = classname.size();
            //cerr << *(int *)pos << endl;//
            pos += sizeof(int);  // classname.size( )
            strncpy(pos, classname.c_str(), classname.size());  // classname
             //cerr << pos << endl;//
            pos += classname.size();
            *(int *)pos = argument_size;
            // cerr << "argument_size = " << argument_size << endl;
             //cerr << "*(int *)pos = " << *(int *)pos << endl;
            pos += sizeof(int);                            // argument_size
            memcpy((void *)pos, argument, argument_size);  // argument
            pos += argument_size;
            *(int *)pos = boundary_width;  // bounary_width
            pos += sizeof(int);
            *(int *)pos = hosts->size();
            pos += sizeof(int);  // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                *(int *)pos = (*hosts)[i].size();
                pos += sizeof(int);  // hosts[i].size( )
                strncpy(pos, (*hosts)[i].c_str(), (*hosts)[i].size());
                pos += (*hosts)[i].size();
            }

            /*serialize fileType memebr data field*/
            *(int *)pos = fileType.size();
            pos += sizeof(int);  // fileType.size()
            strncpy(pos, fileType.c_str(), fileType.size());  // fileType
            pos += fileType.size();

            /*serialize filename memebr data field*/
            *(int *)pos = filename.size();
            pos += sizeof(int);  // fileType.size()
            strncpy(pos, filename.c_str(), filename.size());  // fileType
            pos += filename.size();

            /*serialize distributed_map<string, int> memebr data*/
            *(int *)pos = distributed_map->size();
            pos += sizeof(int);  //distributed_map->size()
            for(auto it = distributed_map->begin(); it != distributed_map->end(); it++){
                *(int *)pos = it->first.size();
                pos += sizeof(int);  // key.size( )
                strncpy(pos, it->first.c_str(), it->first.size()); //copy the key
                pos += it->first.size(); //size of the key

                *(int*)pos = it->second;//value
                pos += sizeof(int); // 
            }

            break;


        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case PLACES_INITIALIZE_GRAPH_FROM_EMPTY:
             // calculate msg_size
            msg_size += (sizeof(int) * (size->size() + 1));  // vector<int> size;
            msg_size += sizeof(int);                 // int handle;
            msg_size += sizeof(int);                 // classname.size( );
            msg_size += classname.size();            // classname
            msg_size += sizeof(int);                 // argument_size;
            msg_size += argument_size;               // void *argument
            msg_size += sizeof(int);                 // int boundary_width;
            msg_size += hosts->size();               // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                msg_size += sizeof(int);
                msg_size += (*hosts)[i].size();  // hosts[i]
            }

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;  // cerr<< *(ACTION_TYPE *)pos << endl;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = size->size();  // cerr << *(int *)pos << endl;
            pos += sizeof(int);          // size.size( );
            for (int i = 0; i < (int)size->size(); i++) {
                *(int *)pos = (*size)[i];
                // cerr << *(int *)pos << endl;
                pos += sizeof(int);  // size[i]
            }
            *(int *)pos = handle;
            // cerr << *(int *)pos << endl;
            pos += sizeof(int);  // handle
            *(int *)pos = classname.size();
            // cerr << *(int *)pos << endl;
            pos += sizeof(int);  // classname.size( )
            strncpy(pos, classname.c_str(), classname.size());  // classname
            // cerr << pos << endl;
            pos += classname.size();
            *(int *)pos = argument_size;
            // cerr << "argument_size = " << argument_size << endl;
            // cerr << "*(int *)pos = " << *(int *)pos << endl;
            pos += sizeof(int);                            // argument_size
            memcpy((void *)pos, argument, argument_size);  // argument
            pos += argument_size;
            *(int *)pos = boundary_width;  // bounary_width
            pos += sizeof(int);
            *(int *)pos = hosts->size();
            pos += sizeof(int);  // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                *(int *)pos = (*hosts)[i].size();
                pos += sizeof(int);  // hosts[i].size( )
                strncpy(pos, (*hosts)[i].c_str(), (*hosts)[i].size());
                pos += (*hosts)[i].size();
            }
            break;

        case PLACES_INITIALIZE:
            //calculate msg_size
            msg_size += (sizeof(int) * (size->size() + 1));  // vector<int> size;
            msg_size += sizeof(int);                 // int handle;
            msg_size += sizeof(int);                 // classname.size( );
            msg_size += classname.size();            // classname
            msg_size += sizeof(int);                 // argument_size;
            msg_size += argument_size;               // void *argument
            msg_size += sizeof(int);                 // int boundary_width;
            msg_size += hosts->size();               // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                msg_size += sizeof(int);
                msg_size += (*hosts)[i].size();  // hosts[i]
            }

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;  // cerr<< *(ACTION_TYPE *)pos << endl;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = size->size();  // cerr << *(int *)pos << endl;
            pos += sizeof(int);          // size.size( );
            for (int i = 0; i < (int)size->size(); i++) {
                *(int *)pos = (*size)[i];
                // cerr << *(int *)pos << endl;
                pos += sizeof(int);  // size[i]
            }
            *(int *)pos = handle;
            // cerr << *(int *)pos << endl;
            pos += sizeof(int);  // handle
            *(int *)pos = classname.size();
            // cerr << *(int *)pos << endl;
            pos += sizeof(int);  // classname.size( )
            strncpy(pos, classname.c_str(), classname.size());  // classname
            // cerr << pos << endl;
            pos += classname.size();
            *(int *)pos = argument_size;
            // cerr << "argument_size = " << argument_size << endl;
            // cerr << "*(int *)pos = " << *(int *)pos << endl;
            pos += sizeof(int);                            // argument_size
            memcpy((void *)pos, argument, argument_size);  // argument
            pos += argument_size;
            *(int *)pos = boundary_width;  // bounary_width
            pos += sizeof(int);
            *(int *)pos = hosts->size();
            pos += sizeof(int);  // hosts->size( );
            for (int i = 0; i < (int)hosts->size(); i++) {
                *(int *)pos = (*hosts)[i].size();
                pos += sizeof(int);  // hosts[i].size( )
                strncpy(pos, (*hosts)[i].c_str(), (*hosts)[i].size());
                pos += (*hosts)[i].size();
            }
            break;
       
        case PLACES_CALL_ALL_VOID_OBJECT:
        case PLACES_CALL_ALL_RETURN_OBJECT:
            // calculate msg_size
            msg_size += sizeof(int);    // int handle,
            msg_size += sizeof(int);    // int functionId
            msg_size += sizeof(int);    // argument_size;
            msg_size += sizeof(int);    // return_size
            msg_size += argument_size;  // void *argument

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = functionId;
            pos += sizeof(int);  // functionId
            *(int *)pos = argument_size;
            pos += sizeof(int);  // arg_size
            *(int *)pos = return_size;
            pos += sizeof(int);                            // return_size
            memcpy((void *)pos, argument, argument_size);  // argument
            break;

        case PLACES_CALL_SOME_VOID_OBJECT:
            break;

        case PLACES_EXCHANGE_ALL:
            // calculate msg_size
            msg_size += sizeof(int);  // int handle,
            msg_size += sizeof(int);  // int dest_handle,
            msg_size += sizeof(int);  // int functionId
            msg_size += sizeof(int);  // int dimension
            msg_size += sizeof(int);  // destinations.size( );
            msg_size += sizeof(int) * (dimension * destinations->size());

            // compose a message
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = dest_handle;
            pos += sizeof(int);  // dest_handle
            *(int *)pos = functionId;
            pos += sizeof(int);  // functionId
            *(int *)pos = dimension;
            pos += sizeof(int);  // dimension
            *(int *)pos = destinations->size();
            pos += sizeof(int);  // dest->size()

            for (int i = 0; i < int(destinations->size()); i++) {
                int *dest = (*destinations)[i];
                for (int j = 0; j < dimension; j++) {
                    *(int *)pos = dest[j];
                    pos += sizeof(int);  // destination[i][j]
                }
            }
            break;

        case PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
            // calculate msg_size
            msg_size += sizeof(int);  // int handle,
            msg_size += sizeof(int);  // int dest_handle,
            msg_size += sizeof(int);  // int functionId
            msg_size += sizeof(int);  // exchangeReqList->size( );

            if (int(exchangeReqList->size()) > 0) {
                msg_size +=
                    (sizeof(int) * 5 + (*exchangeReqList)[0]->outMessageSize) *
                    (exchangeReqList->size());  // rmtExReq
            }

            if (printOutput == true) {
                convert.str("");
                convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST to be sent:";
                MASS_base::log(convert.str());
            }

            // compose a message
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = dest_handle;
            pos += sizeof(int);  // dest_handle
            *(int *)pos = functionId;
            pos += sizeof(int);  // functionId
            *(int *)pos = exchangeReqList->size();
            pos += sizeof(int);  // list->size

            if (printOutput == true) {
                convert.str("");
                convert << " functionId = " << functionId
                        << " exchangeReqList->size = "
                        << exchangeReqList->size();
                MASS_base::log(convert.str());
            }

            for (int i = 0; i < int(exchangeReqList->size());
                 i++) {  // rmtExReq
                *(int *)pos = (*exchangeReqList)[i]->destGlobalLinearIndex;
                pos += sizeof(int);
                *(int *)pos = (*exchangeReqList)[i]->orgGlobalLinearIndex;
                pos += sizeof(int);
                *(int *)pos = (*exchangeReqList)[i]->inMessageIndex;
                pos += sizeof(int);
                *(int *)pos = (*exchangeReqList)[i]->inMessageSize;
                pos += sizeof(int);
                *(int *)pos =
                    (*exchangeReqList)[i]->outMessageSize;  // outMsgSize
                pos += sizeof(int);
                memcpy((void *)pos,
                       (*exchangeReqList)[i]->outMessage,  // outMessage
                       (*exchangeReqList)[i]->outMessageSize);
                pos += (*exchangeReqList)[i]->outMessageSize;
            }

            break;
        case AGENTS_MIGRATION_REMOTE_REQUEST:

            msg_size += sizeof(int);  // Agent Handle
            msg_size += sizeof(int);  // Place Handle
            msg_size += sizeof(int);  // migrationReqList->size( );

            if (int(migrationReqList->size()) > 0) {
                for (int i = 0; i < int(migrationReqList->size()); i++) {
                    msg_size += sizeof(int) * 6;
                    msg_size +=
                        (*migrationReqList)[i]->agent->migratableDataSize;
                }
            }

            if (printOutput == true) {
                convert.str("");
                convert << "AGENTS_MIGRATION_REMOTE_REQUEST to be sent";
                MASS_base::log(convert.str());
            }

            // Compose message
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action type
            *(int *)pos = handle;
            pos += sizeof(int);  // agent handle
            *(int *)pos = dest_handle;
            pos += sizeof(int);  // place handle
            *(int *)pos = migrationReqList->size();
            pos += sizeof(int);  // MigrationReqListy->size

            if (printOutput == true) {
                convert.str("");
                convert << "Agent handle: " << handle
                        << " place handle: " << dest_handle
                        << " size = " << migrationReqList->size();
                MASS_base::log(convert.str());
            }

            for (int i = 0; i < int(migrationReqList->size());
                 i++) {  // each migrationReq
                *(int *)pos = (*migrationReqList)[i]->destGlobalLinearIndex;
                pos += sizeof(int);
                *(int *)pos = (*migrationReqList)[i]->agent->agentsHandle;
                pos += sizeof(int);
                *(int *)pos = (*migrationReqList)[i]->agent->placesHandle;
                pos += sizeof(int);
                *(int *)pos = (*migrationReqList)[i]->agent->agentId;
                pos += sizeof(int);
                *(int *)pos = (*migrationReqList)[i]->agent->parentId;
                pos += sizeof(int);
                *(int *)pos = (*migrationReqList)[i]->agent->migratableDataSize;
                pos += sizeof(int);
                if ((*migrationReqList)[i]->agent->migratableDataSize > 0)
                    memcpy((void *)pos,
                           (*migrationReqList)[i]->agent->migratableData,
                           (*migrationReqList)[i]->agent->migratableDataSize);
                pos += (*migrationReqList)[i]->agent->migratableDataSize;
            }
            break;

        case PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
        case PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
            // calculate msg_size
            msg_size += sizeof(int);  // int inMessageSizes a.k.a. argument_size
            msg_size += argument_size;  // retVals a.k.a. argument

            // compose a message
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = argument_size;
            pos += sizeof(int);                    // arg_size
            memcpy(pos, argument, argument_size);  // argument

            break;
        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_ADD_PLACE:/*MAINTENANCE_ADD_PLACE, handle, vertex(classname), argument, arg_size)*/
        case GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:/* handle,neighborKey)*/
        case MAINTENANCE_REMOVE_PLACE:/*MAINTENANCE_REMOVE_PLACE, handle,vertexId);*/
             // calculate msg_size
            msg_size += sizeof(int);       // int agent_population
            msg_size += sizeof(int);       // int handle;
            msg_size += sizeof(int);       // int dest_handle a.k.a. placeHandle
            msg_size += sizeof(int);       // classname.size( );
            msg_size += classname.size();  // classname
            msg_size += sizeof(int);       // argument_size;
            msg_size += argument_size;     // void *argument

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = agent_population;
            pos += sizeof(int);  // agent_population

            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = dest_handle;
            pos += sizeof(int);  // placeHandle

            *(int *)pos = classname.size();
            pos += sizeof(int);  // classname.size( )
            strncpy(pos, classname.c_str(), classname.size());  // classname
            pos += classname.size();
            *(int *)pos = argument_size;
            pos += sizeof(int);                            // argument_size
            memcpy((void *)pos, argument, argument_size);  // argument
            break;

        case GET_VERTEXPLACE_FROM_VERTEXNAME:

        case AGENTS_INITIALIZE:
            // calculate msg_size
            msg_size += sizeof(int);       // int agent_population
            msg_size += sizeof(int);       // int handle;
            msg_size += sizeof(int);       // int dest_handle a.k.a. placeHandle
            msg_size += sizeof(int);       // classname.size( );
            msg_size += classname.size();  // classname
            msg_size += sizeof(int);       // argument_size;
            msg_size += argument_size;     // void *argument

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = agent_population;
            pos += sizeof(int);  // agent_population

            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = dest_handle;
            pos += sizeof(int);  // placeHandle

            *(int *)pos = classname.size();
            pos += sizeof(int);  // classname.size( )
            strncpy(pos, classname.c_str(), classname.size());  // classname
            pos += classname.size();
            *(int *)pos = argument_size;
            pos += sizeof(int);                            // argument_size
            memcpy((void *)pos, argument, argument_size);  // argument
            break;
       
        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_ADD_EDGE:/*MAINTENANCE_ADD_EDGE, handle, vertexId, neighborId, weight);*/
        case MAINTENANCE_REMOVE_EDGE:/*MAINTENANCE_REMOVE_EDGE, handle,vertexId, neighborId, -0.0);*/
             msg_size += sizeof(handle);//handle
             msg_size += sizeof(int);//size for vertexId
             msg_size += vertexId.size();//vertexId.size()
             msg_size += sizeof(int);//size for neihborId
             msg_size += neighborId.size();//neighborId.size()
             msg_size += sizeof(int); //weight
             
             // compose a msg
            pos = msg = new char[msg_size];

            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle

            *(int *)pos = vertexId.size(); 
            pos += sizeof(int);
            strncpy(pos, vertexId.c_str(), vertexId.size());  // vertexId
            pos += vertexId.size();

            *(int *)pos = neighborId.size(); 
            pos += sizeof(int);
            strncpy(pos, neighborId.c_str(), neighborId.size());  // neighborId
            pos += neighborId.size();

            *(int *)pos = weight;
            pos += sizeof(int);
            break;

        case AGENTS_CALL_ALL_VOID_OBJECT:
        case AGENTS_CALL_ALL_RETURN_OBJECT:
            msg_size += sizeof(int);    // int handle
            msg_size += sizeof(int);    // int functionId
            msg_size += sizeof(int);    // argument_size
            msg_size += sizeof(int);    // return_size
            msg_size += argument_size;  // void *argument

            // compose a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action

            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle
            *(int *)pos = functionId;
            pos += sizeof(int);  // functionId
            *(int *)pos = argument_size;
            pos += sizeof(int);  // arg_size
            *(int *)pos = return_size;
            pos += sizeof(int);                            // return_size
            memcpy((void *)pos, argument, argument_size);  // argument

            break;
       /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_GET_PLACES://MAINTENANCE_GET_PLACES, handle, 0);   
        case MAINTENANCE_REINITIALIZE: //MAINTENANCE_REINITIALIZE, handle, 0);

        case AGENTS_MANAGE_ALL:
        case PLACES_EXCHANGE_BOUNDARY:
            // calculate msg_size
            msg_size += sizeof(int);  // it handle;

            // compse a msg
            pos = msg = new char[msg_size];
            *(ACTION_TYPE *)pos = action;
            pos += sizeof(ACTION_TYPE);  // action
            *(int *)pos = handle;
            pos += sizeof(int);  // handle

            break;

        case FINISH:
            if (printOutput == true)
                cerr << "serialize: FINISH started" << endl;
            msg = new char[msg_size];
            *(ACTION_TYPE *)msg = action;
            if (printOutput == true) cerr << "serialize: FINISH ended" << endl;
            break;

        case   MAINTENANCE_GET_PLACES_RESPONSE:/**/
                        break;//TODO implement it
       default:
            msg_size = 0;
            break;
    }
    if (printOutput == true)
        cerr << "Message::serialize end: action = " << *(ACTION_TYPE *)msg
             << endl;
    return msg;
}

/**
 * Method to deserialize the various Messages that can be communicated within a
 * MASS simulation (enumerated by Message::ACTION_TYPE Enum). The value of, and
 * therefore 'type' of, the Message::ACTION_TYPE for a message is stored at the
 * beginning of the message. So, a simple cast to the Object at the beginning of
 * the char dereferenced value (char *msg) will resolve to the ACTION_TYPE and
 * drive the remainder of the deserialization process (varies with type).
 *
 * @param msg       location of (pointer) the serialized char message to be
 *                  decoded
 * @param msg_size  address of the int, which indicates the size of the message
 *                  to be serialized (and later sent)
 */
void Message::deserialize(char *msg, int msg_size) {
    char *cur = msg;
    int size_size = 0;
    int classname_size = 0;
    int filename_size = 0;
    int fileType_size = 0;
    int vertexId_size = 0;
    int neighborId_size = 0;
    int hosts_size = 0;
    int key_size = 0;
    //int map_size = 0;
    int map_size_for_file = 0;
    int destinations_size = 0;
    int exchangeReqListSize = 0;
    int migrationReqListSize = 0;
    DllClass *agentsDllClass = NULL;
    Agent *agent = NULL;
    AgentMigrationRequest *request = NULL;
    int destIndex = 0;

    ostringstream convert;
    if (printOutput == true) {
        convert << "deserialize: action = " << *(ACTION_TYPE *)cur << endl;
        MASS_base::log(convert.str());
    }

    switch (*(ACTION_TYPE *)cur) {
        case EMPTY:
            action = EMPTY;
            return;
        case ACK:
            if (printOutput == true) {
                convert.str("");
                convert << *(int *)cur << " ";
            }

            action = ACK;
            cur += sizeof(ACTION_TYPE);

            if (printOutput == true) {
                convert << *(int *)cur << " ";
            }

            agent_population = *(int *)cur;
            cur += sizeof(int);  // agent_population

            if (printOutput == true) {
                convert << "coming agent_pouplation = " << agent_population;
                MASS_base::log(convert.str());
            }

            if (msg_size > int(sizeof(ACTION_TYPE) + sizeof(int))) {
                argument_size = *(int *)cur;
                cur += sizeof(int);  // argument_size
                argument_in_heap =
                    ((argument = new char[argument_size]) != NULL);
                /*
                convert.str( "" ); convert << "argument_size = " <<
                argument_size; MASS_base::log( convert.str( ) );
                */
                memcpy(argument, (void *)cur, argument_size);  // argument
                cur += argument_size;
            }
            if (printOutput == true) cerr << "deserialize completed" << endl;
            return;

        
        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         +++++++++Added by Elias --> for Graph feature support ++++++++++++++++++++++++++++++

         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/

        case MAINTENANCE_ADD_EDGE:/*MAINTENANCE_ADD_EDGE, handle, vertexId, neighborId, weight);*/
        case MAINTENANCE_REMOVE_EDGE:/*MAINTENANCE_REMOVE_EDGE, handle,vertexId, neighborId, -0.0);*/
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle

            vertexId_size = *(int *)cur;
            cur += sizeof(int);  // vertexId_size
            vertexId = "";
            vertexId.append(cur, vertexId_size);  // classname
            cur += vertexId_size;

            neighborId_size = *(int *)cur;
            cur += sizeof(int);  // neighborId_size
            neighborId = "";
            neighborId.append(cur, neighborId_size);  // neighborId
            cur += neighborId_size;
            weight = *(int *)cur;
            cur += sizeof(int);  // handle
            return;
        
        /*ACTION_TYPE action, vector<int> *size, int handle, string classname, 
          void *argument,int arg_size, int boundary_width, vector<string> *hosts,
          string type, string filename, unordered_map<std::string, int> *dist_map)*/
        case PLACES_INITIALIZE_GRAPH_FROM_FILE:
            action = PLACES_INITIALIZE_GRAPH_FROM_FILE;
            cur += sizeof(ACTION_TYPE);
            size_size = *(int *)cur;
            cur += sizeof(int);  // size.size( );
            cerr << *(int*)cur;//********************************************************
            
            //convert.str( "" );convert << "size = " << size_size << endl;//****************************************
            //MASS_base::log( convert.str( ) );//********************************************************
            
            size = new vector<int>;
            for (int i = 0; i < size_size; i++) {
                
                //convert.str( "" ); convert << *(int *)cur << endl;//********************************************************
                //MASS_base::log( convert.str( ) );//********************************************************
                
                size->push_back(*(int *)cur);
                cur += sizeof(int);  // size[i];
            }
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            classname_size = *(int *)cur;
            cur += sizeof(int);  // classname_size
            classname = "";
            classname.append(cur, classname_size);  // classname
            MASS_base::log( classname.c_str( ) );

            cur += classname_size;
            argument_size = *(int *)cur;
            cur += sizeof(int);  // argument_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            
            //convert.str( "" ); //********************************************************
            //convert << "argument_size = " << argument_size << //********************************************************
            //endl; MASS_base::log( convert.str( ) );//********************************************************
            
            memcpy(argument, (void *)cur, argument_size);  // argument
            cur += argument_size;
            boundary_width = *(int *)cur;
            cur += sizeof(int);  // boundary_width
            hosts_size = *(int *)cur;
            cur += sizeof(int);  // hosts.size( );
            
            //convert.str( "" ); //********************************************************
            //convert << "host_size = " << hosts_size << endl;//********************************************************
           // MASS_base::log( convert.str( ) );//********************************************************
            
            hosts = new vector<string>;
            for (int i = 0; i < hosts_size; i++) {
                int hostname_size = *(int *)cur;
                cur += sizeof(int);  // hosts[i].size()
                string hostname = "";
                hostname.append(cur, hostname_size);
                cur += hostname_size;
                MASS_base::log( hostname );
                hosts->push_back(hostname);  // hosts[i]
            }

            fileType_size = *(int *)cur;
            cur += sizeof(int);  // fileType_size
            fileType = "";
            fileType.append(cur, fileType_size);  // fileType
            cur += fileType_size;
           // MASS_base::log(fileType); //********************************************************

            filename_size = *(int *)cur;
            cur += sizeof(int);  // filename_size
            filename = "";
            filename.append(cur, filename_size);  // filename
            cur += filename_size;
            //MASS_base::log(filename); //********************************************************
            map_size_for_file = *(int *)cur;
            cur += sizeof(int);  // distribted_map->size( );

            distributed_map = new unordered_map<string, int>();
            //convert << " map size "<< map_size_for_file << endl; //***********************************************
            //MASS_base::log(convert.str()); //********************************************************
            //MASS_base::log("key-protein        id "); //********************************************************
            for(int i = 0; i < map_size_for_file; i++){
                int key_size = *(int *)cur;
                cur += sizeof(int);  // key.size()
                string key = "";
                key.append(cur, key_size);
                cur += key_size;

                int value = *(int*)cur;
                cur += sizeof(int); //value
               // convert << key << "        " <<  value << endl;//***********************************
                //MASS_base::log(convert.str());//********************************************************

                distributed_map->insert({key, value});
            }
            return;
            
         /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case PLACES_INITIALIZE_GRAPH_FROM_EMPTY:
             action = PLACES_INITIALIZE_GRAPH_FROM_EMPTY;
            cur += sizeof(ACTION_TYPE);
            size_size = *(int *)cur;
            cur += sizeof(int);  // size.size( );
            /*
            convert.str( "" ); convert << "size = " << size_size << endl;
            MASS_base::log( convert.str( ) );
            */
            size = new vector<int>;
            for (int i = 0; i < size_size; i++) {
                /*
                convert.str( "" ); convert << *(int *)cur << endl;
                MASS_base::log( convert.str( ) );
                */
                size->push_back(*(int *)cur);
                cur += sizeof(int);  // size[i];
            }
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            classname_size = *(int *)cur;
            cur += sizeof(int);  // classname_size
            classname = "";
            classname.append(cur, classname_size);  // classname
            // MASS_base::log( classname.c_str( ) );

            cur += classname_size;
            argument_size = *(int *)cur;
            cur += sizeof(int);  // argument_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            /*
            convert.str( "" ); convert << "argument_size = " << argument_size <<
            endl; MASS_base::log( convert.str( ) );
            */
            memcpy(argument, (void *)cur, argument_size);  // argument
            cur += argument_size;
            boundary_width = *(int *)cur;
            cur += sizeof(int);  // boundary_width
            hosts_size = *(int *)cur;
            cur += sizeof(int);  // hosts.size( );
            /*
            convert.str( "" ); convert << "host_size = " << hosts_size << endl;
            MASS_base::log( convert.str( ) );
            */
            hosts = new vector<string>;
            for (int i = 0; i < hosts_size; i++) {
                int hostname_size = *(int *)cur;
                cur += sizeof(int);  // hosts[i].size()
                string hostname = "";
                hostname.append(cur, hostname_size);
                cur += hostname_size;
                // MASS_base::log( hostname );
                hosts->push_back(hostname);  // hosts[i]
            }
            
            return;

        case PLACES_INITIALIZE:
            action = PLACES_INITIALIZE;
            cur += sizeof(ACTION_TYPE);
            size_size = *(int *)cur;
            cur += sizeof(int);  // size.size( );
            /*
            convert.str( "" ); convert << "size = " << size_size << endl;
            MASS_base::log( convert.str( ) );
            */
            size = new vector<int>;
            for (int i = 0; i < size_size; i++) {
                /*
                convert.str( "" ); convert << *(int *)cur << endl;
                MASS_base::log( convert.str( ) );
                */
                size->push_back(*(int *)cur);
                cur += sizeof(int);  // size[i];
            }
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            classname_size = *(int *)cur;
            cur += sizeof(int);  // classname_size
            classname = "";
            classname.append(cur, classname_size);  // classname
            // MASS_base::log( classname.c_str( ) );

            cur += classname_size;
            argument_size = *(int *)cur;
            cur += sizeof(int);  // argument_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            /*
            convert.str( "" ); convert << "argument_size = " << argument_size <<
            endl; MASS_base::log( convert.str( ) );
            */
            memcpy(argument, (void *)cur, argument_size);  // argument
            cur += argument_size;
            boundary_width = *(int *)cur;
            cur += sizeof(int);  // boundary_width
            hosts_size = *(int *)cur;
            cur += sizeof(int);  // hosts.size( );
            /*
            convert.str( "" ); convert << "host_size = " << hosts_size << endl;
            MASS_base::log( convert.str( ) );
            */
            hosts = new vector<string>;
            for (int i = 0; i < hosts_size; i++) {
                int hostname_size = *(int *)cur;
                cur += sizeof(int);  // hosts[i].size()
                string hostname = "";
                hostname.append(cur, hostname_size);
                cur += hostname_size;
                // MASS_base::log( hostname );
                hosts->push_back(hostname);  // hosts[i]
            }
            return;

        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_ADD_PLACE:
        case PLACES_CALL_ALL_VOID_OBJECT:
        case PLACES_CALL_ALL_RETURN_OBJECT:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            functionId = *(int *)cur;
            cur += sizeof(int);  // functionId
            argument_size = *(int *)cur;
            cur += sizeof(int);  // arg_size
            return_size = *(int *)cur;
            cur += sizeof(int);  // return_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            memcpy(argument, (void *)cur, argument_size);  // argument
            return;

        case PLACES_CALL_SOME_VOID_OBJECT:
            break;
        case PLACES_EXCHANGE_ALL:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            dest_handle = *(int *)cur;
            cur += sizeof(int);  // dest_handle
            functionId = *(int *)cur;
            cur += sizeof(int);  // functionId
            dimension = *(int *)cur;
            cur += sizeof(int);  // dimension
            destinations_size = *(int *)cur;
            cur += sizeof(int);  // dest->size()
            destinations = new vector<int *>;
            argument_in_heap = true;

            for (int i = 0; i < destinations_size; i++) {
                int *dest = new int[dimension];
                for (int j = 0; j < dimension; j++) {
                    dest[j] = *(int *)cur;
                    cur += sizeof(int);  // destination[i][j]
                }
                destinations->push_back(dest);
            }
            return;

        case PLACES_EXCHANGE_ALL_REMOTE_REQUEST:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            dest_handle = *(int *)cur;
            cur += sizeof(int);  // dest_handle
            functionId = *(int *)cur;
            cur += sizeof(int);  // functionId
            exchangeReqListSize = *(int *)cur;
            cur += sizeof(int);  // list->size
            exchangeReqList =
                new vector<RemoteExchangeRequest *>;  // list created
            argument_in_heap = true;

            if (printOutput == true) {
                convert.str("");
                convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST: "
                        << " action = " << action << " handle = " << handle
                        << " dest_handle = " << dest_handle
                        << " functionId = " << functionId
                        << " exchangeReqListSize = " << exchangeReqListSize;
                MASS_base::log(convert.str());
            }

            for (int i = 0; i < exchangeReqListSize; i++) {
                int destIndex = *(int *)cur;
                cur += sizeof(int);
                int orgIndex = *(int *)cur;
                cur += sizeof(int);
                int inMsgIndex = *(int *)cur;
                cur += sizeof(int);
                int inMsgSize = *(int *)cur;
                cur += sizeof(int);
                int outMessageSize = *(int *)cur;
                cur += sizeof(int);  // outMsgSize
                char *outMessage = new char[outMessageSize];
                memcpy(outMessage, (void *)cur, outMessageSize);  // outMessage
                cur += outMessageSize;

                if (printOutput == true) {
                    convert.str("");
                    convert << "PLACES_EXCHANGE_ALL_REMOTE_REQUEST:"
                            << " i = " << i << " destIndex = " << destIndex
                            << " orgIndex = " << orgIndex
                            << " inMsgIndex = " << inMsgIndex
                            << " inMsgSize = " << inMsgSize
                            << " outMessageSize = " << outMessageSize
                            << " outMessage = " << *(int *)outMessage;
                    MASS_base::log(convert.str());
                }

                RemoteExchangeRequest *request = new RemoteExchangeRequest(
                    destIndex, orgIndex, inMsgIndex, inMsgSize, outMessage,
                    outMessageSize, true);
                exchangeReqList->push_back(request);
            }
            return;

        case AGENTS_MIGRATION_REMOTE_REQUEST:

            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action type
            handle = *(int *)cur;
            cur += sizeof(int);  // agent handle
            dest_handle = *(int *)cur;
            cur += sizeof(int);  // place handle
            migrationReqListSize = *(int *)cur;
            cur += sizeof(int);  // MigrationReqListy->size
            migrationReqList =
                new vector<AgentMigrationRequest *>;  // list created
            argument_in_heap = true;

            if (printOutput == true) {
                convert.str("");
                convert << "Deserialize: Agent handle: " << handle
                        << " place handle: " << dest_handle;
                MASS_base::log(convert.str());
            }

            agentsDllClass = MASS_base::dllMap[handle];
            for (int i = 0; i < migrationReqListSize;
                 i++) {  // each migrationReq
                destIndex = *(int *)cur;
                cur += sizeof(int);
                agent = (Agent *)(agentsDllClass->instantiate(NULL));
                agent->agentsHandle = *(int *)cur;
                cur += sizeof(int);
                agent->placesHandle = *(int *)cur;
                cur += sizeof(int);
                agent->agentId = *(int *)cur;
                cur += sizeof(int);
                agent->parentId = *(int *)cur;
                cur += sizeof(int);
                agent->migratableDataSize = *(int *)cur;
                cur += sizeof(int);
                agent->alive = true;
                agent->newChildren = 0;

                if (printOutput == true) {
                    convert.str("");
                    convert << "Deserialize: agentId(" << agent << ")["
                            << agent->agentId
                            << "] data size = " << agent->migratableDataSize;
                    MASS_base::log(convert.str());

                    convert.str("");
                    convert << "Deserialize: agentId(" << agent << ")["
                            << agent->agentId << "] data = " << (char *)cur;
                    MASS_base::log(convert.str());
                }

                if (agent->migratableDataSize > 0) {
                    if (printOutput == true) MASS_base::log("A");

                    // agent->migratableData = malloc( agent->migratableDataSize
                    // );
                    agent->migratableData =
                        (void *)(new char[agent->migratableDataSize]);

                    if (printOutput == true) {
                        convert.str("");
                        convert << "Deserialize: agentId(" << agent << ")["
                                << agent->agentId
                                << "] malloc = " << agent->migratableData;
                        MASS_base::log(convert.str());
                    }

                    memcpy(agent->migratableData, (void *)cur,
                           agent->migratableDataSize);

                    if (printOutput == true) MASS_base::log("C");

                    cur += agent->migratableDataSize;

                    if (printOutput == true) MASS_base::log("D");
                }

                if (printOutput == true) {
                    convert.str("");
                    convert << "Deserialize: agentId(" << agent << ")["
                            << agent->agentId
                            << "] migratableData = " << agent->migratableData;
                    MASS_base::log(convert.str());
                }

                request = new AgentMigrationRequest(destIndex, agent);
                migrationReqList->push_back(request);
            }

            return;

        case PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT:
        case PLACES_EXCHANGE_BOUNDARY_REMOTE_REQUEST:
            if (printOutput == true) {
                convert.str("");
                convert << "PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: will "
                           "deserialize"
                        << " argument_size = " << argument_size;
                MASS_base::log(convert.str());
            }
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            argument_size = *(int *)cur;
            cur += sizeof(int);  // arg_size

            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            memcpy(argument, (void *)cur, argument_size);  // argument

            if (printOutput == true) {
                convert.str("");
                convert << "PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: "
                           "deserialization done"
                        << " argument_size = " << argument_size;
                MASS_base::log(convert.str());
            }

            return;

        
        case GRAPH_PLACES_EXCHANGE_ALL_REMOTE_RETURN_OBJECT: /*action, handle,vertexId, neighborId, weight*/
              action = *(ACTION_TYPE *)cur;
              cur += sizeof(ACTION_TYPE);  // action
              handle = *(int *)cur;
              cur += sizeof(int);  // handle
              vertexId_size = *(int*)cur;
              cur += sizeof(int);  // vertexId_size
              vertexId = "";
              vertexId.append(cur, vertexId_size);  // vertexId
              cur += vertexId_size;
              neighborId_size = *(int*)cur;
              cur += sizeof(int);  // neighborId_size
              neighborId = "";
              neighborId.append(cur, neighborId_size);
              cur += vertexId_size;
              weight = *(int *)cur;
              cur += sizeof(int);  // weight

              if (printOutput == true) {
                convert.str("");
                convert << "handle = " << handle
                        << " vertexId = " << vertexId
                        << " neighborId = " << neighborId
                        << " weight = " << weight;
                MASS_base::log(convert.str());
            }

            return;
            
        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_REMOVE_PLACE: /*action, handle,vertexId, neighborId, 0*/
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            agent_population = *(int *)cur;
            cur += sizeof(int);  // population
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            dest_handle = *(int *)cur;
            cur += sizeof(int);  // placesHandle
            classname_size = *(int *)cur;
            cur += sizeof(int);  // classname_size
            classname = "";
            classname.append(cur, classname_size);  // classname
            cur += classname_size;
            argument_size = *(int *)cur;
            cur += sizeof(int);  // arg_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            memcpy(argument, (void *)cur, argument_size);  // argument

            if (printOutput == true) {
                convert.str("");
                convert << "population = " << agent_population
                        << " handle = " << handle
                        << " placeHandle = " << dest_handle
                        << " classname = " << classname
                        << " argument_size = " << argument_size;
                MASS_base::log(convert.str());
            }

            return;
        case GET_VERTEXPLACE_FROM_VERTEXNAME:
        case AGENTS_INITIALIZE:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            agent_population = *(int *)cur;
            cur += sizeof(int);  // population
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            dest_handle = *(int *)cur;
            cur += sizeof(int);  // placesHandle
            classname_size = *(int *)cur;
            cur += sizeof(int);  // classname_size
            classname = "";
            classname.append(cur, classname_size);  // classname
            cur += classname_size;
            argument_size = *(int *)cur;
            cur += sizeof(int);  // arg_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            memcpy(argument, (void *)cur, argument_size);  // argument

            if (printOutput == true) {
                convert.str("");
                convert << "population = " << agent_population
                        << " handle = " << handle
                        << " placeHandle = " << dest_handle
                        << " classname = " << classname
                        << " argument_size = " << argument_size;
                MASS_base::log(convert.str());
            }

            return;

        case AGENTS_CALL_ALL_VOID_OBJECT:
        case AGENTS_CALL_ALL_RETURN_OBJECT:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle
            functionId = *(int *)cur;
            cur += sizeof(int);  // functionId
            argument_size = *(int *)cur;
            cur += sizeof(int);  // arg_size
            return_size = *(int *)cur;
            cur += sizeof(int);  // return_size
            argument_in_heap = ((argument = new char[argument_size]) != NULL);
            memcpy(argument, (void *)cur, argument_size);  // argument
            return;

            return;

        /*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

         *++++++++++ Added by Elias --> for Graph feature +++++++++++++++++++++++++++++++++++++
        
         *@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
         *------------------------------------------------------------------------------------*/
        case MAINTENANCE_GET_PLACES:
        case MAINTENANCE_REINITIALIZE:

        case AGENTS_MANAGE_ALL:
        case PLACES_EXCHANGE_BOUNDARY:
            action = *(ACTION_TYPE *)cur;
            cur += sizeof(ACTION_TYPE);  // action
            handle = *(int *)cur;
            cur += sizeof(int);  // handle

            return;

        case FINISH:
            action = FINISH;
            return;

        default:
            break;
    }
}

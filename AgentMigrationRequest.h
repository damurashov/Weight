#ifndef AGENTMIGRATIONREQUEST_H
#define AGENTMIGRATIONREQUEST_H

#include "Agent.h"

class AgentMigrationRequest {
 friend class Agents_base;
 friend class Message;
 public:
  AgentMigrationRequest( int destIndex, Agent *agent ) :
    destGlobalLinearIndex( destIndex ), agent( agent ) {
  };

  ~AgentMigrationRequest( ) {
    if ( agent->migratableDataSize > 0 && agent->migratableData != NULL )
      free( agent->migratableData );
    delete agent;
  }

  int destGlobalLinearIndex;
  Agent *agent;
};

#endif

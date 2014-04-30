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
    delete agent;
    }

  int destGlobalLinearIndex;
  Agent *agent;
};

#endif

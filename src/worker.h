
#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <zmq.hpp>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <thread>
#include <functional>
#include <messages.pb.h>
#include "protobufsocket.h"

class Node;

class Worker
{
    public:
      Worker(zmq::context_t& context, std::string myAddress_, const Node &node_) :
	    myAddress(myAddress_),
	    node(node_),
	    workerSocket(context, ZMQ_REP)
	    
      {
	workerSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
      }
  
      void listener();
    
    
  
  
  private:
    messages::proto::Msg exec(const messages::proto::Command& msgReq);
    std::string execCmd(const std::string& cmd);
  
    
    std::string myAddress;
    const Node &node;
    
    ProtoBufSocketWrapper workerSocket;
    
};

#endif // WORKER_H

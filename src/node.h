#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <messages.pb.h>
#include <zmq.hpp>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <boost/program_options.hpp>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include "balancingserver.h"
#include "protobufsocket.h"
#include "client.h"
#include "registernodeaddress.h"
#include "masterlistener.h"

class Node
{
 public:
    Node(std::string myAddress_, int priority_);
    void start();
    
    std::string getMasterAddress() const;
    std::string getMyAddress() const;
    int getPriority() const;
    
    void changeMasterAddress(std::string newMasterAddress);
    
    static std::string getRegisterEndpoint();
    static std::string getBroadcatMasterEndpoint();
private:
  //void pinger(std::string address, int priority);
  
  
  zmq::context_t context;
  
  mutable std::mutex masterAddressMutex; //in C++ 17 could change to shared_lock
  std::string masterAddress;
  std::string myAddress;
  int priority;
  
  BalancingServer server;
  Client client;
  RegisterNodeAddress registerAddress; 
  MasterListener masterListener;
  
  static const std::string registerServiceAddress;
  static const int registerPort = 4112;
  static const int broadcastMasterPort = 4113;
};


#endif // NODE_H

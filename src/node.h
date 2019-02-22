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

class Node
{
 public:
    Node(std::string myAddress_, int priority_);
    void start();
    
    std::string getMasterAddress() const;
private:
  void client();
  void masterListener();

  void registerMyAddress(int interval);

  void pinger(std::string address, int priority);

  void aliver(int interval);
  
  void prompt();
  
  void changeMasterAddress(std::string newMasterAddress);
  
  
  zmq::context_t context;
  ProtoBufSocketWrapper clientSocket;
  ProtoBufSocketWrapper subscriberSocket;
  ProtoBufSocketWrapper registerSocket;
  
  mutable std::mutex masterAddressMutex; //in C++ 17 could change to shared_lock
  std::string masterAddress;
  std::string myAddress;
  int priority;
  
  static const std::string registerServiceAddress;
  static const int registerPort = 4112;
  static const int broadcastPort = 4113;
  static const int alivePort = 4114;
  
};

#endif // NODE_H

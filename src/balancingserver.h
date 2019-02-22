#ifndef BALANCINGSERVER_H
#define BALANCINGSERVER_H

#include <iostream>
#include <zmq.hpp>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <thread>
#include <functional>
#include "protobufsocket.h"

class Node;

class BalancingServer {
public:
    BalancingServer() = delete;
    BalancingServer(BalancingServer&) = delete;
    BalancingServer& operator=(BalancingServer&) = delete;
    
    BalancingServer(zmq::context_t& context, std::string myAddress_, const Node &node_);

    enum { kMaxThread = 10 };

    void run();

private:
    std::string myAddress;
    
    const Node &node;
    zmq::context_t& context;
    
    ProtoBufSocketWrapper frontend;
    ProtoBufSocketWrapper backend;
};

#endif // BALANCINGSERVER_H

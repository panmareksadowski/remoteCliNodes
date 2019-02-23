/*
 * Copyright 2019 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifndef CLIENT_H
#define CLIENT_H

#include "protobufsocket.h"
#include <mutex>

class Node;

class Client
{
public:
  Client(zmq::context_t& context, Node &node_);
  
  void run();
  
  void reconnect(std::string oldMasterAddress, std::string newMasterAddress);
private:
  bool send(const ::google::protobuf::Message& msg);
  bool recv(::google::protobuf::Message& msg);
  
  void prompt();
  
  Node &node;
  
  std::mutex clientSocketMutex;
  ProtoBufSocketWrapper clientSocket;
};

#endif // CLIENT_H

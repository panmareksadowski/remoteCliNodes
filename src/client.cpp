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

#include "client.h"
#include "node.h"

Client::Client(zmq::context_t& context, Node &node_)
  : node(node_), 
    clientSocket(context, ZMQ_REQ)
{
  clientSocket.setsockopt(ZMQ_RCVTIMEO, 6000);
  clientSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
  clientSocket.setsockopt(ZMQ_REQ_RELAXED, 1);
}

void Client::run()
{ 
  while (true) 
  {
    while(node.getMasterAddress() == "")
    {
      std::cout<<"Waiting for master..."<<std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    prompt();
    std::string cmdStr;
    std::getline(std::cin, cmdStr);
    
    messages::proto::Msg cmdReq;
    cmdReq.set_type(messages::proto::Msg_MessageType_TYPE_Command);
    cmdReq.mutable_comand()->set_cmd(cmdStr); 
    if(!send (cmdReq))
    {
      std::cout<<"Master unavailable."<< std::endl;
      reconnect(node.getMasterAddress(), node.getMasterAddress());
      continue;
    }

    messages::proto::Msg cmdResult;
    if(!recv (cmdResult))
    {
      std::cout<<"Timeout, master not respond."<< std::endl;
      reconnect(node.getMasterAddress(), node.getMasterAddress());
    }
    else if (cmdResult.result().return_code() == 0)
    {
      std::cout<<cmdResult.result().cmd() << std::endl;
    }
    else
    {
      std::cout<<"Error, Error code: " << cmdResult.result().return_code() << std::endl;
    }
  }
}

bool Client::send(const ::google::protobuf::Message& msg)
{
  std::lock_guard<std::mutex> clientSocketGuard(clientSocketMutex);
  return clientSocket.send (msg);
}

bool Client::recv(::google::protobuf::Message& msg)
{
  std::lock_guard<std::mutex> clientSocketGuard(clientSocketMutex);
  return clientSocket.recv (msg);
}

void Client::reconnect(std::string oldMasterAddress, std::string newMasterAddress)
{
  std::lock_guard<std::mutex> clientSocketGuard(clientSocketMutex);
  if(oldMasterAddress != "")
    clientSocket.disconnect("tcp://"+oldMasterAddress);
  if(newMasterAddress != "")
    clientSocket.connect ("tcp://"+newMasterAddress);
}


void Client::prompt()
{
  std::cout<<"#############################\n\
#enter command which needs to be executed #\n\
#############################"<<std::endl;
  
}
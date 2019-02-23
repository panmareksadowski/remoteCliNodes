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

#include "registernodeaddress.h"
#include "node.h"

RegisterNodeAddress::RegisterNodeAddress(zmq::context_t& context, Node &node_)
  : node(node_), 
    registerSocket(context, ZMQ_REQ)
{
    registerSocket.connect ("tcp://"+node.getRegisterEndpoint());
}

void RegisterNodeAddress::run(int intervalSec)
{
  while(true)
  {
    messages::proto::Msg registerMsg;
    registerMsg.set_type(messages::proto::Msg_MessageType_TYPE_RegisterReq);
    registerMsg.mutable_registerreq()->set_address(node.getMyAddress());
    registerMsg.mutable_registerreq()->set_priority(node.getPriority());
    registerSocket.send (registerMsg);
    
    messages::proto::Msg respMsg;
    registerSocket.recv (respMsg);
    node.changeMasterAddress(respMsg.registerres().masteraddress());
    
    std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
  }
}
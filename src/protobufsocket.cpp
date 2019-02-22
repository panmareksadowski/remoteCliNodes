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

#include "protobufsocket.h"

  
bool ProtoBufSocketWrapper::send(const ::google::protobuf::Message& msg, int flags)
{
  std::string response;
  msg.SerializeToString(&response);
  zmq::message_t reply (response.size());
  memcpy ((void *) reply.data (), response.c_str(), response.size());
  return zmq::socket_t::send (reply, flags);
}

bool ProtoBufSocketWrapper::recv(::google::protobuf::Message& msg, int flags)
{
  zmq::message_t request;
  bool ret = zmq::socket_t::recv (&request, flags);
  std::string msg_str(static_cast<char*>(request.data()), request.size());
  msg.ParseFromString(msg_str);
  
  return ret;
}
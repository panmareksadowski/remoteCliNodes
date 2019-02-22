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

#ifndef PROTOBUFSOCKET_H
#define PROTOBUFSOCKET_H

#include <zmq.hpp>
#include <google/protobuf/message.h>

class ProtoBufSocketWrapper : public zmq::socket_t
{
public:
  ProtoBufSocketWrapper(zmq::context_t& ctx, int type) : zmq::socket_t(ctx, type)
  {}
  
  bool send(const ::google::protobuf::Message& msg, int flags = 0);
  bool recv(::google::protobuf::Message& msg, int flags = 0);
};

#endif // PROTOBUFSOCKET_H

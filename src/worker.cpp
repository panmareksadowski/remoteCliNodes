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

#include "worker.h"
#include "node.h"
#include <future>  

 void Worker::listener()
  {
    workerSocket.connect("inproc://backend");

    while (true) 
    {
	messages::proto::Msg msgReq;
	workerSocket.recv (msgReq);
	
	messages::proto::Msg msgRes;
	if(msgReq.type() == messages::proto::Msg_MessageType_TYPE_Command && node.getMasterAddress() == node.getMyAddress())
	{
	  //std::cout<<msgReq.comand().cmd()<<std::endl;
	  
	  std::future<messages::proto::Msg> futureMsgRes = std::async (std::launch::async, &Worker::exec, this, msgReq.comand());
	  
	  if( futureMsgRes.wait_for( std::chrono::seconds(5)) ==  std::future_status::ready)
	  {
	    msgRes = futureMsgRes.get();
	  }
	  else
	  {
	    msgRes.set_type(messages::proto::Msg_MessageType_TYPE_Result);	
	    msgRes.mutable_result()->set_return_code(-2);
	  }
	}
	else
	{
	  msgRes.set_type(messages::proto::Msg_MessageType_TYPE_Error);
	  msgRes.mutable_error()->set_error_code(1);
	  msgRes.mutable_error()->set_reason("Unsupported msg type.");
	}
	workerSocket.send (msgRes);
    }
  }  
    
    
  messages::proto::Msg Worker::exec(const messages::proto::Command& msgReq)
  {
      messages::proto::Msg msg;
      msg.set_type(messages::proto::Msg_MessageType_TYPE_Result);
      try{
      std::string result = execCmd(msgReq.cmd());
      //std::cout<<"Result:"<<std::endl<<result<<std::endl;
      
      msg.mutable_result()->set_return_code(0);
      msg.mutable_result()->set_cmd(result);
      
      //std::cout<<"After msg proccess"<<std::endl;
      
      }
      catch (...)
      {
	msg.mutable_result()->set_return_code(-1);
      }
      return msg;
  }
  
  std::string Worker::execCmd(const std::string& cmd) {
    char buffer[128];
    std::string result = "";
    std::FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
	while (fgets(buffer, sizeof buffer, pipe) != NULL) {
	    result += buffer;
	}
    } catch (...) {
	pclose(pipe);
	throw;
    }
    pclose(pipe);
    return result;
}
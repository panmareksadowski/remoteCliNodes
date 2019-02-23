#include "node.h"

Node::Node(std::string myAddress_, int priority_) : 
  context(1),
  server(context, *this),
  client(context, *this),
  registerAddress(context, *this),
  masterListener(context, *this),
  myAddress(myAddress_),
  priority(priority_),
  masterAddress("")
  { 
  }
    
void Node::start()
{
  std::cout<<"Strating node"<<std::endl;
  std::cout<<"Server address: "<<myAddress<<std::endl;
  std::cout<<"Priority: "<<priority<<std::endl;
  
  std::thread register_t(std::bind(&RegisterNodeAddress::run, &registerAddress, 2));
  std::thread server_t(std::bind(&BalancingServer::run, &server));
  std::thread masterListener_t(std::bind(&MasterListener::run, &masterListener));
  std::thread client_t(std::bind(&Client::run, &client));
  register_t.join();
  server_t.join();
  masterListener_t.join();
  client_t.join();
  
  std::cout<<"Shutting down node"<<std::endl;
}

std::string Node::getMasterAddress() const
{
  std::lock_guard<std::mutex> masterAddressGuard(masterAddressMutex);
  return masterAddress;
}

std::string Node::getMyAddress() const
{
  return myAddress;
}

int Node::getPriority() const
{
  return priority;
}

std::string Node::getRegisterEndpoint()
{
  return registerServiceAddress+":"+std::to_string(registerPort);
  return registerServiceAddress+":"+std::to_string(registerPort);
}

std::string Node::getBroadcatMasterEndpoint()
{
  return registerServiceAddress+":"+std::to_string(broadcastMasterPort);
}


void Node::changeMasterAddress(std::string newMasterAddress)
{
  std::lock_guard<std::mutex> masterAddressGuard(masterAddressMutex);
  if(masterAddress == newMasterAddress)
    return;
  
  client.reconnect(masterAddress, newMasterAddress);
  masterAddress = newMasterAddress;
}
  
/*not implemented
void Node::pinger(std::string address, int priority)
{
  ProtoBufSocketWrapper pingerSocket (context, ZMQ_REP);
  pingerSocket.connect ("tcp://"+registerServiceAddress+":"+std::to_string(alivePort));
  while (true) 
    {
	messages::proto::Msg msgReq;
	pingerSocket.recv (&msgReq);
	std::cout<<"Recv ping"<<std::endl;
	
	messages::proto::Msg msgRes;
	if(msgReq.type() == messages::proto::Msg_MessageType_TYPE_PING)
	{
	  msgRes.set_type(messages::proto::Msg_MessageType_TYPE_PONG);
	}
	else
	{
	  msgRes.set_type(messages::proto::Msg_MessageType_TYPE_Error);
	  msgRes.mutable_error()->set_error_code(1);
	  msgRes.mutable_error()->set_reason("Unsupported msg type.");
	}
	pingerSocket.send (msgRes);
    }
}*/


const std::string Node::registerServiceAddress = "127.0.0.1";
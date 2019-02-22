#include "node.h"

Node::Node(std::string myAddress_, int priority_) : 
  context(1),
  clientSocket(context, ZMQ_REQ),
  subscriberSocket(context, ZMQ_SUB),
  registerSocket(context, ZMQ_REQ),
  myAddress(myAddress_),
  priority(priority_),
  masterAddress("")
  {
    clientSocket.setsockopt(ZMQ_RCVTIMEO, 5000);
    clientSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
    
    subscriberSocket.connect ("tcp://"+registerServiceAddress+":"+std::to_string(broadcastPort));
    subscriberSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    
    registerSocket.connect ("tcp://"+registerServiceAddress+":"+std::to_string(registerPort));
    registerSocket.setsockopt(ZMQ_RCVTIMEO, 1000);
    registerSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
  }
    
void Node::start()
{
  std::cout<<"Strating node"<<std::endl;
  std::cout<<"Server address: "<<myAddress<<std::endl;
  std::cout<<"Priority: "<<priority<<std::endl;
  
  BalancingServer server(context, myAddress, *this);
  
  std::thread register_t(std::bind(&Node::registerMyAddress, this, 15));
  std::thread server_t(std::bind(&BalancingServer::run, &server));
  std::thread masterListener_t(std::bind(&Node::masterListener, this));
  std::thread client_t(std::bind(&Node::client, this));
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

void Node::client()
{ 
  while (true) 
  {
    while(getMasterAddress() == "")
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
    clientSocket.send (cmdReq);



    messages::proto::Msg cmdResult;
    clientSocket.recv (cmdResult);
    if (cmdResult.result().return_code() == 0)
    {
      std::cout<<cmdResult.result().cmd() << std::endl;
    }
    else
    {
      std::cout<<"Error, Error code: " << cmdResult.result().return_code() << std::endl;
    }
  }
}

void Node::prompt()
{
  std::cout<<"#############################\n\
#enter command which needs to be executed #\n\
#############################"<<std::endl;
  
}



void Node::masterListener()
{

  while (true) 
  {
      messages::proto::Msg msgReq;
      subscriberSocket.recv (msgReq);
      if(msgReq.type() == messages::proto::Msg_MessageType_TYPE_MasterBroadcast)
      {
	changeMasterAddress(msgReq.masterbroadcast().masteraddress());
      }
  }
}

void Node::registerMyAddress(int interval)
{
  while(true)
  {
    messages::proto::Msg registerMsg;
    registerMsg.set_type(messages::proto::Msg_MessageType_TYPE_RegisterReq);
    registerMsg.mutable_registerreq()->set_address(myAddress);
    registerMsg.mutable_registerreq()->set_priority(priority);
    registerSocket.send (registerMsg);

    messages::proto::Msg respMsg;
    registerSocket.recv (respMsg);
    changeMasterAddress(respMsg.registerres().masteraddress());
    
    std::this_thread::sleep_for(std::chrono::seconds(interval));
  }
}

void Node::changeMasterAddress(std::string newMasterAddress)
{
  std::lock_guard<std::mutex> masterAddressGuard(masterAddressMutex);
  if(masterAddress == newMasterAddress)
    return;
  
  if(masterAddress != "")
  {
    clientSocket.disconnect("tcp://"+masterAddress);
  }
  masterAddress = newMasterAddress;
  clientSocket.connect ("tcp://"+newMasterAddress);
  
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
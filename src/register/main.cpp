#include <iostream>
#include <messages.pb.h>
#include <zmq.hpp>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <boost/program_options.hpp>
#include <thread>
#include <random>
#include <chrono>
#include <utility>
#include "../src/protobufsocket.h"

using namespace boost::program_options;


class RegisterService
{  
  public:
  RegisterService(std::string myAddress_ = "127.0.0.1",
		  int registerPort = 4112, int broadcastPort = 4113, int aliverPort = 4114,
		  int maxInactiveNodeTime_ = 30
 		) : 
	myAddress(myAddress_),
	masterAddress(""), 
	ctx(1),
	registerSocket (ctx, ZMQ_REP),
	broadcastSocket (ctx, ZMQ_PUB),
	maxInactiveNodeTime(maxInactiveNodeTime_)
  {
    registerSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
    registerSocket.bind ("tcp://"+myAddress+":"+std::to_string(registerPort));
    broadcastSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
    broadcastSocket.bind ("tcp://"+myAddress+":"+std::to_string(broadcastPort));
    std::cout<<"Started"<<std::endl;
  }
  
  void start()
  {
    std::thread remover_t(std::bind(&RegisterService::removeNotActiveNodes, this, maxInactiveNodeTime, maxInactiveNodeTime/3));
    
    while (true) 
    {
	messages::proto::Msg registerReq;
	registerSocket.recv (registerReq);
	
	messages::proto::Msg registerResponse;
	if(registerReq.type() == messages::proto::Msg_MessageType_TYPE_RegisterReq)
	{
	  registerAddress(registerReq.registerreq().address(), registerReq.registerreq().priority());
	  
	  registerResponse.set_type(messages::proto::Msg_MessageType_TYPE_RegisterRes);
	  registerResponse.mutable_registerres()->set_masteraddress(masterAddress);
	}
	else
	{
	  registerResponse.set_type(messages::proto::Msg_MessageType_TYPE_Error);
	  registerResponse.mutable_error()->set_error_code(1);
	  registerResponse.mutable_error()->set_reason("Unsupported msg type.");
	}
	registerSocket.send (registerResponse);
    }
    
    remover_t.join();
  }
  
  void removeNotActiveNodes(int maxTimeSec, int intervalSec = 10)
  {
    
    while(true)
    {
      std::cout<<"Remove all inactive nodes."<<std::endl;
      std::cout<<"Active nodes:"<<std::endl;
      
      bool is_ereased = false;;
      auto currentTime = std::chrono::steady_clock::now();
      for(auto it = adressesInfo.begin(); it != adressesInfo.end(); )
      {
	if(std::chrono::duration_cast<std::chrono::seconds>(currentTime-it->second.lastSeen).count() > maxTimeSec)
	{
	  adressesInfo.erase(it++);
	  is_ereased = true;
	}
	else
	{
	  std::cout<<it->first<<std::endl;
	  ++it;
	}
      }
      
      if(is_ereased)
	chooseNewMaster();
      
      std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
    }
  }
  

  
  void chooseNewMaster()
  {
    //std::cout<<"ChooseNewMaster"<<std::endl;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator (seed);
    
    std::pair<std::string, int> init = std::make_pair(std::string(""), 0);
    int sumPriority = 0;
    for(const auto& x: adressesInfo)
      sumPriority+= x.second.priority;
    
    std::uniform_int_distribution<int> regular (1,sumPriority);
    int choose = regular(generator);
    
    int sum = 0;
    for(const auto& x: adressesInfo)
    {
      if(sum + x.second.priority > choose)
      {
	if(masterAddress != x.first)
	{
	  masterAddress = x.first;
	  broadcastMaster();
	}
	break;
      }
      sum += x.second.priority;
    }
  }
  
  void broadcastMaster()
  {
    std::cout<<"BroadcastMaster: "<<masterAddress<<std::endl;
    messages::proto::Msg masterBroadcast;
    masterBroadcast.set_type(messages::proto::Msg_MessageType_TYPE_MasterBroadcast);
    masterBroadcast.mutable_masterbroadcast()->set_masteraddress(masterAddress);

    broadcastSocket.send (masterBroadcast);
  }
  
  void registerAddress(std::string address, int priority)
  { 
    if( adressesInfo.find(address) == adressesInfo.end() )
    {
      adressesInfo[address] = {priority, std::chrono::steady_clock::now()};
      std::cout<<"Address registered: "<<address<<", priority: "<<priority<<std::endl;
      chooseNewMaster();
    }
    else if(adressesInfo[address].priority != priority)
    {
      adressesInfo[address] = {priority, std::chrono::steady_clock::now()};
      std::cout<<"Change priority of: "<<address<<", new priority: "<<priority<<std::endl;
      chooseNewMaster();
    }
    else
    {
      adressesInfo[address] = {priority, std::chrono::steady_clock::now()};
    }
  }
  
  
  /*not implemented
  void pingMethod()
  {
    ProtoBufSocketWrapper pingerSocket (ctx, ZMQ_REQ);
    pingerSocket.setsockopt(ZMQ_RCVTIMEO, 1000);
    pingerSocket.setsockopt(ZMQ_SNDTIMEO, 1000);
    pingerSocket.bind ("tcp://"+myAddress+":"+std::to_string(broadcastPort+1));
    
    while(true)
    {
      bool is_changed = false;
      for(auto it = adressesInfo.begin(); it != adressesInfo.end(); ++it)
      {
	  messages::proto::Msg msgPing;
	  msg.set_type(messages::proto::Msg_MessageType_TYPE_PING);
	  
	  if( pingerSocket.send (msgPing) == EAGAIN)
	  {
	    adressesInfo.erase(it);
	    is_changed = true;
	  }
	  
	  messages::proto::Msg msgPong;
	  if( pingerSocket.recv (msgPong) == EAGAIN)
	  {
	    adressesInfo.erase(it);
	    is_changed = true;
	  }
      }
      
      std::this_thread::sleep_for(std::chrono::seconds(15));
      if(is_changed)
      {
	chooseNewMaster();
      }
    }
    
  }*/
  
  std::string myAddress;
  int registerPort;
  int broadcastPort;
  
  struct adressesInfo
  {
    int priority;
    std::chrono::time_point<std::chrono::steady_clock> lastSeen;
  };
  
  std::map<std::string, adressesInfo> adressesInfo;
  std::string masterAddress;
  
  zmq::context_t ctx;
  ProtoBufSocketWrapper registerSocket;
  ProtoBufSocketWrapper broadcastSocket;
  
  int maxInactiveNodeTime;
};


int main(int argc, const char *argv[])
{
  try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("broker-addr", value<std::string>()->default_value("127.0.0.1:4321"), "node address")
      ("priority", value<int>()->default_value(0), "Priority( to be master node)");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
      std::cout << desc << '\n';
    else if (vm.count("broker-addr") && vm.count("priority"))
    {
      RegisterService rs;
      rs.start();
    }
    else
      std::cout << "Address or priority missing option." << '\n';
  }
  catch (const error &ex)
  {
    std::cerr << ex.what() << '\n';
  }
  return 0;
}

#include "balancingserver.h"
#include "worker.h"

BalancingServer::BalancingServer(zmq::context_t& context_, std::string myAddress_, std::string& masterAddress_)
        : myAddress(myAddress_), 
        masterAddress(masterAddress_),
        context(context_),
        frontend(context, ZMQ_ROUTER), 
        backend(context, ZMQ_DEALER)
    {}

void BalancingServer::run() {
    frontend.bind("tcp://"+myAddress);
    backend.bind("inproc://backend");
    std::vector<Worker *> worker;
    std::vector<std::thread *> worker_thread;
    for (int i = 0; i < kMaxThread; ++i) {
	worker.push_back(new Worker(context, myAddress, masterAddress));

	worker_thread.push_back(new std::thread(std::bind(&Worker::listener, worker[i])));
	worker_thread[i]->detach();
    }

    try {
	zmq::proxy(static_cast<void *>(frontend), static_cast<void *>(backend), nullptr);
    }
    catch (std::exception &e) {
      std::cout<<"Error"<<std::endl<<e.what()<<std::endl;
    }

    for (int i = 0; i < kMaxThread; ++i) {
	std::cout<<"deleting"<<std::endl;
	delete worker[i];
	delete worker_thread[i];
    }
}
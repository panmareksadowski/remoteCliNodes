#include <iostream>
#include <messages.pb.h>
#include <zmq.hpp>
#include <stdexcept>
#include <cstdio>
#include <string>
#include <boost/program_options.hpp>
#include <thread>
#include <functional>
#include <chrono>
#include "balancingserver.h"
#include "node.h"

using namespace boost::program_options;


int main(int argc, const char *argv[])
{
  try
  {
    options_description desc{"Options"};
    desc.add_options()
      ("help,h", "Help screen")
      ("broker-addr", value<std::string>()->required(), "node address")
      ("priority", value<int>()->required(), "Priority( to be master node)");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);

    if (vm.count("help"))
      std::cout << desc << '\n';
    else if (vm.count("broker-addr") && vm.count("priority"))
    {
      if(vm["priority"].as<int>() < 1)
	throw error("Priotity must be at least 1");
      
      Node node(vm["broker-addr"].as<std::string>(), vm["priority"].as<int>());
      node.start();
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

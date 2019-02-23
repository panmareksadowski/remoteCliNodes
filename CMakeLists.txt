cmake_minimum_required(VERSION 3.1)
project(network)

set (CMAKE_CXX_STANDARD 11)

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

#proto messages
find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS proto/messages.proto)
add_library(proto_interface ${PROTO_SRCS} ${PROTO_HDRS})
include_directories(${CMAKE_BINARY_DIR})

#zmq
find_library(ZEROMQ zmq)

add_executable(node src/main.cpp 
  src/balancingserver.cpp
  src/worker.cpp
  src/node.cpp
  src/client.cpp
  src/masterlistener.cpp
  src/registernodeaddress.cpp
  src/protobufsocket.cpp
  #here comes new source files
  )
message(${PROTOBUF_LIBRARIES})
target_link_libraries(node
  proto_interface
  ${PROTOBUF_LIBRARIES}
  ${ZEROMQ}
  ${Boost_LIBRARIES}
  )
  
add_executable(register src/register/main.cpp 
  src/protobufsocket.cpp
  #here comes new source files
  )
 target_link_libraries(register
  proto_interface
  ${PROTOBUF_LIBRARIES}
  ${ZEROMQ}
  ${Boost_LIBRARIES}
  )

enable_testing()
include_directories(/usr/src/gmock/)
include_directories(/usr/src/gtest/)

add_executable(node_test 
  /usr/src/gmock/src/gmock-all.cc
  /usr/src/gmock/src/gmock_main.cc
  /usr/src/gtest/src/gtest-all.cc
  test/simple.cpp
  #here comes new test files
  )

target_link_libraries(node_test
  pthread
  )
add_test(NodeTest node_test)

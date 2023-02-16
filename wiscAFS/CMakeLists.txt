# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.8)

project(wiscAFS C CXX)

include(./common.cmake)

# Proto file
get_filename_component(wiscAFS_proto "./wiscAFS.proto" ABSOLUTE)
get_filename_component(wiscAFS_proto_path "${wiscAFS_proto}" PATH)

# Generated sources
set(wiscAFS_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/wiscAFS.pb.cc")
set(wiscAFS_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/wiscAFS.pb.h")
set(wiscAFS_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/wiscAFS.grpc.pb.cc")
set(wiscAFS_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/wiscAFS.grpc.pb.h")
add_custom_command(
	OUTPUT "${wiscAFS_proto_srcs}" "${wiscAFS_proto_hdrs}" "${wiscAFS_grpc_srcs}" "${wiscAFS_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
	-I "${wiscAFS_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
	"${wiscAFS_proto}"
	DEPENDS "${wiscAFS_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# wiscAFS_grpc_proto
add_library(wiscAFS_grpc_proto SHARED
	${wiscAFS_grpc_srcs}
	${wiscAFS_grpc_hdrs}
	${wiscAFS_proto_srcs}
	${wiscAFS_proto_hdrs}
    cache/ClientCache.h
    wiscAFS_client.hh
    wiscAFS_client.cc)
target_link_libraries(wiscAFS_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target
		wiscAFS_client_standalone wiscAFS_server 
  )
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
	  wiscAFS_grpc_proto
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

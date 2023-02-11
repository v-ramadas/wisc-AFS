/*
 *
 * Copyright 2015 gRPC authors.
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

#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "wiscAFS.grpc.pb.h"


using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCRequest;
using wiscAFS::RCPResponse;
using wiscAFS::RPCAttr;


// Logic and data behind the server's behavior.
class wiscAFSImpl final : public AFSController::Service {
 Status OpenFile(ServerContext* context, const RPCRequest* request,
                 RCPResponse* reply) override {
              
   std::ifstream file(request->pathName + request->filename);
   RPCAttr rcp_Attr = new RPCAttr();
   if (file.is_open()) {
    //Call get Attribute 
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string contents = buffer.str();
    reply->set_status(0);
    reply->set_data(contents);
    rcpAttr->set_filesize(contents.length());
    reply->set_rpcAttr(rcp_Attr)
   }
   else{
    reply->set_status(1);
   }
   return Status::OK;
 }
};

void RunServer() {
 std::string server_address("10.10.1.2:50051");
 GreeterServiceImpl service;

 grpc::EnableDefaultHealthCheckService(true);
 grpc::reflection::InitProtoReflectionServerBuilderPlugin();
 ServerBuilder builder;
 // Listen on the given address without any authentication mechanism.
 builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
 // Register "service" as the instance through which we'll communicate with
 // clients. In this case it corresponds to an *synchronous* service.
 builder.RegisterService(&service);
 // Finally assemble the server.
 std::unique_ptr<Server> server(builder.BuildAndStart());
 std::cout << "Server listening on " << server_address << std::endl;

 // Wait for the server to shutdown. Note that some other thread must be
 // responsible for shutting down the server for this call to ever return.
 server->Wait();
}

int main(int argc, char** argv) {
//  RunServer();

  return 0;
}

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
#include <sstream>
#include <cstdio>
#include <unistd.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include "wiscAFS.grpc.pb.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCRequest;
using wiscAFS::RPCResponse;
using wiscAFS::RPCAttr;


// Logic and data behind the server's behavior.
class wiscAFSImpl final : public AFSController::Service {

 Status OpenFile(ServerContext* context, const RPCRequest* request,
                 RPCResponse* reply) override {
    
   // Error handle the path and filename
   std::string path = request->path();
   std::string filename = request->filename();
   int mode = request->mode();
   //int fileDescriptor = open((path + filename).c_str(), mode);
   std::ifstream f(path+filename);
   int fd = open((path+filename).c_str(), mode);
   RPCAttr *rpcAttr; 
   std::cerr << "here1 \n";
   if (fd != -1) {
    //Call get Attribute 
    struct stat file_info;
    std::cerr << "here1 \n";
    if (fstat(fd, &file_info) == -1) {
      reply->set_status(-1);
      return Status::OK;
    }
    std::cerr << "here2 \n";
    int sz = lseek(fd, 0L, SEEK_END);
    lseek(fd, 0L, SEEK_SET);
    char *buffer = new char[sz];
    std::cerr << "here3 \n";
    int err = read(fd, buffer, sz);
    std::string obuffer = buffer;
    reply->set_data(obuffer);
    std::cerr << "here4 \n";
    //Add attributes from stat
    //rpcAttr->set_filesize(sz);
    //reply->set_allocated_rpcattr(rpcAttr);
    close(fd);
    std::cerr << "here5 \n";
    reply->set_status(1);

   }
   else{
    reply->set_status(-1);
   }
   
   return Status::OK;
 }

 Status CloseFile(ServerContext* context, const RPCRequest* request,
                 RPCResponse* reply) override {
    std::string path = request->path();
    std::string newContent = request->data();
    std::string fileName = request->data();
    // Check cache to see if the client exists - if so open the file and write the entire content and close it and update the cache, if not ignore the write
    int fileDescriptor = open((path + fileName).c_str(), O_RDWR);
    if (fileDescriptor != -1) {
      //TEMP FILE
      ssize_t writeResult = write(fileDescriptor, newContent.c_str(), newContent.size());
      if (writeResult == -1) {
        reply->set_status(-1);
        close(fileDescriptor);
        return Status::OK;
      }
      int closeResult = close(fileDescriptor);
      if(closeResult == -1){
        reply->set_status(-1);
        return Status::OK;
      }
      reply->set_status(1);
    }
    else{
      reply->set_status(-1);
    }
    return Status::OK;
  }
    /* Generally CreateFile would create a file if it doesn't exisit or overtie the file into a new file, here we will reply on first implementation only for now.*/
  Status CreateFile(ServerContext* context, const RPCRequest* request,
                 RPCResponse* reply) override {
      std::string path = request->path();
      std::string new_content = request->data();
      std::string fileName = request->data();
      std::string flag = request->data();
      int fileDescriptor = open((path + fileName).c_str(), O_CREAT, flag);
      if (fileDescriptor != -1) {
        reply->set_status(-1);
        return Status::OK;
      }

      reply->set_status(1);
      return Status::OK;
  }

  //Handle cache

  Status DeleteFile(ServerContext* context, const RPCRequest* request,
      RPCResponse* reply) override {
    std::string path = request->path();
    std::string fileName = request->data();
    int unlinkResult = unlink((path + fileName).c_str());
    if (unlinkResult != -1) {
        reply->set_status(-1);
        return Status::OK;
    }  
    reply->set_status(1);
    return Status::OK;
  }

  Status RemoveDir(ServerContext* context, const RPCRequest* request,
      RPCResponse* reply) override {
      std::string path = request->path();
      std::string dirName = request->data();
      int result = rmdir((path + dirName).c_str());
      if (result == -1) {
        reply->set_status(-1);
        return Status::OK;
      }
      reply->set_status(1);
      return Status::OK;

  }

Status CreateDir(ServerContext* context, const RPCRequest* request,
      RPCResponse* reply) override {
      std::string path = request->path();
      std::string dirName = request->data();
      int mode = request->mode();
      int result = mkdir((path + dirName).c_str(), mode);
      if (result == -1) {
        reply->set_status(-1);
        return Status::OK;
      }
      reply->set_status(1);
      return Status::OK;
      }
    
  

Status GetAttr(ServerContext* context, const RPCRequest* request,
      RPCResponse* reply) override {
    std::string path = request->path();
    std::string filename = request->data();
    RPCAttr * rpcAttr;// =  reply->mutable_data();
    int file_descriptor = open((path + filename).c_str(), O_RDONLY);
    if (file_descriptor == -1) {
      //The file could not be opened
      reply->set_status(-1);
      return Status::OK;
    }

    struct stat file_info;
    if (fstat(file_descriptor, &file_info) == -1) {
      //The file information could not be retrieved.
      reply->set_status(-1);
      return Status::OK;
    }
    rpcAttr->set_filesize(file_info.st_size);
    close(file_descriptor);
    reply->set_status(1);
    return Status::OK;
  }

};



void RunServer() {
 std::string server_address("10.10.1.2:50051");
 wiscAFSImpl service;

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
  RunServer();

  return 0;
}

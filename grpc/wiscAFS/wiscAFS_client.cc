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
#include <fcntl.h>
#include <unistd.h>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/wiscAFS.grpc.pb.h"
#else
#include "wiscAFS.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCResponse;
using wiscAFS::RPCRequest;
using wiscAFS::RPCAttr;

class wiscAFSClient {
 public:
  wiscAFSClient(std::shared_ptr<Channel> channel)
      : stub_(AFSController::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  RPCResponse OpenFile(const std::string& filename, const std::string& path, const int flags) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->OpenFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse CloseFile(const std::string& filename, const std::string& path, const string& data) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);
    request.set_data(data);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->CloseFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse CreateFile(const std::string& filename, const std::string& path, const int mode) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);
    request.set_mode(mode);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->CreateFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse DeleteFile(const std::string& filename, const std::string& path) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->DeleteFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse CreateDir(const std::string& dirname, const std::string& path, const int mode) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);
    request.set_path(mode);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->CreateFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse OpenDir(const std::string& dirname, const std::string& path, const int mode) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(dirname);
    request.set_path(path);
    request.set_path(mode);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->OpenFile(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse RemoveDir(const std::string& dirname, const std::string& path) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(dirname);
    request.set_path(path);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->RemoveDir(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }

  RPCResponse GetAttr(const std::string& filename, const std::string& path) {
    // Data we are sending to the server.
    RPCRequest request;
    request.set_filename(filename);
    request.set_path(path);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->GetAttr(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      reply.set_status(-1);
      return reply;
    }
  }


 private:
  std::unique_ptr<AFSController::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).

  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target="
                  << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "10.10.1.2:50051";
  }
  wiscAFSClient afsClient (
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  std::string filename("a.txt");
  std::string dirname("dir1");
  std::string path("/users/vramadas/grpc/examples/cpp/wiscAFS/cmake/build/");
  std::string data("asdafgdf\nssfdlkjgdgklfg\n");
  std::string data("asdafgdf\nssfdlkjgdgklfg\n");
  int flags = O_RDONLY;
  int mode = S_IRWXU;
  std::cout << "Sending CreateFile\n" ;
  RPCResponse reply = afsClient.CreateFile(filename, path, mode);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  std::cout << "Sending OpenFile\n" ;
  reply = afsClient.OpenFile(filename, path, flags);
  std::cout << "Data recieved : " << reply.data() << std::endl;

  std::cout << "Sending GetAttr\n" ;
  reply = afsClient.GetAttr(filename, path);
  std::cout << "Data recieved : " << reply.RPCAttr().filesize() << std::endl;

  std::cout << "Sending CloseFile\n" ;
  reply = afsClient.CloseFile(filename, path, data);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  std::cout << "Sending DeleteFile\n" ;
  reply = afsClient.DeleteFile(filename, path);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  std::cout << "Sending CreateDir\n" ;
  reply = afsClient.CreateDir(dirname, path);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  std::cout << "Sending OpenDir\n" ;
  reply = afsClient.OpenDir(dirname, path);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  std::cout << "Sending RemoveDir\n" ;
  reply = afsClient.RemoveDir(dirname, path);
  std::cout << "Response recieved : " << reply.status() << std::endl;

  return 0;
}

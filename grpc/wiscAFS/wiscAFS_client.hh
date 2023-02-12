#ifndef WISC_AFS_CLIENT
#define WISC_AFS_CLIENT

#include <iostream>
#include <memory>
#include <string>
#include <fcntl.h>
#include <unistd.h>

#include <grpcpp/grpcpp.h>
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

        RPCResponse CloseFile(const std::string& filename, const std::string& path, const std::string& data) {
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
            request.set_filename(dirname);
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

        RPCResponse OpenDir(const std::string& dirname, const std::string& path, const int mode) {
            // Data we are sending to the server.
            RPCRequest request;
            request.set_filename(dirname);
            request.set_path(path);
            request.set_mode(mode);

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

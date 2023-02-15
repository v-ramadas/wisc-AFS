#ifndef WISC_AFS_CLIENT
#define WISC_AFS_CLIENT

#include <iostream>
#include <memory>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "cache/ClientCache.h"

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

    DiskCache diskCache;
    std::string client_path = '/temp/afs/';
    public:
        wiscAFSClient(std::shared_ptr<Channel> channel)
            : stub_(AFSController::NewStub(channel)) {
                diskCache.loadCache();
            }

        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        int OpenFile(const std::string& filename, const int flags) {

            ClientCacheValue *ccv1 = diskcache.getCacheValue(filename);
            if(ccv1 == nullptr){
                // Data we are sending to the server. ##ASSUMING FILENAMES include path
               
                RPCRequest request;
                request.set_filename(filename);

                // Container for the data we expect from the server.
                RPCResponse reply;

                // Context for the client. It could be used to convey extra information to
                // the server and/or tweak certain RPC behaviors.
                ClientContext context;

                // The actual RPC.
                Status status = stub_->OpenFile(&context, request, &reply);
                // Act upon its status.
                if (status.ok()) {
                    std::local_path = (client_path + reply->inode() + ".tmp").c_str();
                    int fd = open(local_path,  O_WRONLY | O_CREAT | O_EXCL, 0644);
                    if (fileDescriptor != -1) {
                        ssize_t writeResult = write(fileDescriptor, reply->data(), reply->data().size());
                        //SUCCESS
                        FileAttrs fileatts(reply->filesize,reply->atime,reply->mtime);
                        ClientCacheValue ccv(fileatts, local_path, reply->inode, false, fd);
                        diskcache.addCacheValue(filename, ccv);
                    }
                    return fd;
                } else {
                    return -status.error_code();
                }
            }
            else{
                //ALREADY IN CACHE
                int fd = ccv1->fileDiscriptor;
                return fd;
               

            }
        }

        int CloseFile(const std::string& filename) {
            // Data we are sending to the server.
            ClientCacheValue *ccv1 = diskcache.getCacheValue(filename);
            if(ccv1 == nullptr){
                errno=ENOENT;
            }
            else if(!ccv1->isDirty){
                deleteCacheValue(filename)
                return 0;
            }
            else{
                RPCRequest request;
                request.set_filename(filename);
                request.set_data(data);
                // Container for the data we expect from the server.
                RPCResponse reply;

                // Context for the client. It could be used to convey extra information to
                // the server and/or tweak certain RPC behaviors.
                ClientContext context;

                // The actual RPC.
                Status status = stub_->CloseFile(&context, request, &reply);

                if (status.ok()) {
                    deleteCacheValue(filename);
                    return 0;
                }
                else{
                    return -status.error_code();
                }
            }
        }
        
        //Returning either fD or error
        int ReadFile(const std::string& filename){
            ClientCacheValue *ccv1 = diskcache.getCacheValue(filename);
            if(ccv1 == nullptr){
                errno=ENOENT;
            }
            else{
                return ccv1->fileDiscriptor;
            }
        }

        //Returning either fD or error
        int WriteFile(const std::string& filename){
            ClientCacheValue *ccv1 = diskcache.getCacheValue(filename);
            if(ccv1 == nullptr){
                errno=ENOENT;
            }
            else{
                if(!ccv1->isDirty()){
                    ccv1->isDirty = true;
                    diskcache.updateCacheValue(filename, ccv1);
                }
                return ccv1->fileDiscriptor;
            }
        }


        RPCResponse DeleteFile(const std::string& filename, const std::string& path) {
            // Data we are sending to the server.
            ClientCacheValue *ccv1 = diskcache.getCacheValue(filename);
            if(ccv1 == nullptr){
                errno=ENOENT;
            }
            else{
                RPCRequest request;
                request.set_filename(filename);
                // Container for the data we expect from the server.
                RPCResponse reply;

                // Context for the client. It could be used to convey extra information to
                // the server and/or tweak certain RPC behaviors.
                ClientContext context;

                // The actual RPC.
                Status status = stub_->DeleteFile(&context, request, &reply);

                if (status.ok()) {
                    deleteCacheValue(filename);
                    return 0;
                }
                else{
                    return -status.error_code();
                }
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

        RPCResponse GetAttr(const std::string& filename) {
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
                reply.set_status(-status.error_code());
                return reply;
            }
        }


    private:
        std::unique_ptr<AFSController::Stub> stub_;
};

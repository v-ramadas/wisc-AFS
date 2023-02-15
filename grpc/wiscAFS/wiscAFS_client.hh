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
    std::string client_path = "/temp/afs/";
    public:
        wiscAFSClient(std::shared_ptr<Channel> channel)
            : stub_(AFSController::NewStub(channel)) {
                diskCache.loadCache();
            }

        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        int OpenFile(const std::string& filename, const int flags);

        int CloseFile(const std::string& filename);

        
        //Returning either fD or error
        int ReadFile(const std::string& filename);

        int WriteFile(const std::string& filename);

        // Data we are sending to the server.
        RPCResponse DeleteFile(const std::string& filename, const std::string& path);

        RPCResponse CreateDir(const std::string& dirname, const std::string& path, const int mode);

        RPCResponse OpenDir(const std::string& dirname, const std::string& path, const int mode);

        RPCResponse RemoveDir(const std::string& dirname, const std::string& path);

        RPCResponse GetAttr(const std::string& filename);

    private:
        std::unique_ptr<AFSController::Stub> stub_;
};

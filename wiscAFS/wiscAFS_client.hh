#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "cache/ClientCache.h"
#include <grpcpp/grpcpp.h>
#include "wiscAFS.grpc.pb.h"
#include<fuse.h>
#ifdef __cplusplus
extern "C" {
#endif
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCResponse;
using wiscAFS::RPCRequest;
using wiscAFS::FileInfo;
using wiscAFS::RPCDirReply;

class wiscAFSClient {

    DiskCache diskCache;
    std::string client_path = "/tmp/afs/";
    public:
        wiscAFSClient(std::shared_ptr<Channel> channel)
            : stub_(AFSController::NewStub(channel)) {
                diskCache.loadCache();
            }

        void setFileInfo(FileInfo*, struct stat);
        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        int OpenFile(const std::string& filename, const int flags);

        int CloseFile(const std::string& filename);

        
        //Returning either fD or error
        int ReadFile(const std::string& filename);

        int WriteFile(const std::string& filename);

        int CreateFile(const std::string& filename, const int flags, const int mode);

        int ReadDir(const std::string& p, void *buf, fuse_fill_dir_t filler);

        // Data we are sending to the server.
        RPCResponse DeleteFile(const std::string& filename);

        RPCResponse CreateDir(const std::string& dirname, const int mode);

        RPCResponse OpenDir(const std::string& dirname, const int mode);

        RPCResponse RemoveDir(const std::string& dirname);

        RPCResponse GetAttr(const std::string& filename);

        RPCResponse GetXAttr(const std::string& filename, const std::string& name);

        RPCResponse Statfs(const std::string& filename);

    private:
        std::unique_ptr<AFSController::Stub> stub_;
};

#ifdef __cplusplus
}
#endif

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
using wiscAFS::FileLock;

class wiscAFSClient {

    DiskCache diskCache;
    std::string client_path = "/tmp/afs/";
    sem_t clientSem;
    public:
        wiscAFSClient(std::shared_ptr<Channel> channel)
            : stub_(AFSController::NewStub(channel)) {
                diskCache.loadCache();
                sem_init(&clientSem, 0, 1);
            }

        void setFileInfo(FileInfo*, struct stat);
        // Assembles the client's payload, sends it and presents the response back
        // from the server.
        RPCResponse OpenFile(const std::string& filename, const int flags);

        RPCResponse CloseFile(const std::string& filename, bool release);

        RPCResponse RenameFile(const std::string& oldname, const std::string& newname);

        
        //Returning either fD or error
        int ReadFile(const std::string& filename);

        int WriteFile(const std::string& filename);

        RPCResponse CreateFile(const std::string& filename, const int flags, const int mode);

        RPCDirReply ReadDir(const std::string& p, void *buf, fuse_fill_dir_t filler);

        // Data we are sending to the server.
        RPCResponse DeleteFile(const std::string& filename);

        RPCResponse CreateDir(const std::string& dirname, const int mode);

        RPCResponse OpenDir(const std::string& dirname, const int mode);

        RPCResponse RemoveDir(const std::string& dirname);

        RPCResponse GetAttr(const std::string& filename);

        RPCResponse GetXAttr(const std::string& filename, const std::string& name, char* value, size_t len);

        RPCResponse Statfs(const std::string& filename);

        RPCResponse AccessFile(const std::string& filename, const int mode);

        RPCResponse TruncateFile(const std::string& filename, const int length);

        RPCResponse Fcntl(const std::string& path, struct fuse_file_info* fi, int cmd, struct flock *fl);

        RPCResponse Chmod(const std::string& path, int flags);

        RPCResponse Chown(const std::string& path, uid_t, gid_t );

    private:
        std::unique_ptr<AFSController::Stub> stub_;
};

#ifdef __cplusplus
}
#endif

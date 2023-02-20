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
#include <sys/vfs.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCRequest;
using wiscAFS::RPCResponse;
using wiscAFS::FileInfo;
using wiscAFS::Statfs;


// Logic and data behind the server's behavior.
class wiscAFSImpl final : public AFSController::Service {

    void setFileInfo(FileInfo* fileInfo, struct stat info) {
        fileInfo->set_st_dev(info.st_dev);
        fileInfo->set_st_ino(info.st_ino);
        fileInfo->set_st_mode(info.st_mode);
        fileInfo->set_st_nlink(info.st_nlink);
        fileInfo->set_st_uid(info.st_uid);
        fileInfo->set_st_gid(info.st_gid);
        fileInfo->set_st_rdev(info.st_rdev);
        fileInfo->set_st_size(info.st_size);
        fileInfo->set_st_blksize(info.st_blksize);
        fileInfo->set_st_blocks(info.st_blocks);
        fileInfo->set_st_atim(info.st_atim.tv_nsec);
        fileInfo->set_st_mtim(info.st_mtim.tv_nsec);
        fileInfo->set_st_ctim(info.st_ctim.tv_nsec);
    }

    void setStatFS(Statfs* statFS, struct statfs info) {
        statFS->set_f_type(info.f_type);
        statFS->set_f_bsize(info.f_bsize);
        statFS->set_f_blocks(info.f_blocks);
        statFS->set_f_bfree(info.f_bfree);
        statFS->set_f_bavail(info.f_bavail);
        statFS->set_f_files(info.f_files);
        statFS->set_f_ffree(info.f_ffree);
        statFS->set_f_fsid((info.f_fsid.__val[1] << 32) | (info.f_fsid.__val[0]));
        statFS->set_f_namelen(info.f_namelen);
        statFS->set_f_frsize(info.f_frsize);
        statFS->set_f_flags(info.f_flags);
    }

    Status OpenFile(ServerContext* context, const RPCRequest* request,RPCResponse* reply) override {
        // Error handle the path and filename
        std::string filename = request->filename();
        int flags = request->flags();
        std::ifstream f(filename);
        struct stat file_info;

        int fd = open((filename).c_str(), flags);
        std::cout << "Printing filename,  fd, and flags " << filename << " " << fd << " " << flags << std::endl;
        if (fd < 0)
            std::cout << "Cannot open file " << filename << std::endl;
        FileInfo *fileInfo = new FileInfo;
        if (fd != -1) {
            //Call get Attribute 
            if (fstat(fd, &file_info) == -1) {
                reply->set_status(-1);
                return Status::OK;
            }

            //Read whole file
            int sz = lseek(fd, 0L, SEEK_END);
            lseek(fd, 0L, SEEK_SET);
            char *buffer = new char[sz];
            std::cout << "size = " << sz << std::endl;
            if(sz == 0){
                reply->set_data("");
            }
            else{
                int err = read(fd, buffer, sz);
                reply->set_data(buffer);
            }
            //std::cout << sz << "HSdjsdbj" << std::endl;
           // std::string obuffer = buffer;

            //Set all attrs
            setFileInfo(fileInfo, file_info);
            //Populate reply
            //reply->set_data(obuffer);
            reply->set_allocated_fileinfo(fileInfo);
            reply->set_status(1);

            close(fd);

        }
        else{
            reply->set_status(-1);
        }

        return Status::OK;
    }

    Status CloseFile(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
        std::string newContent = request->data();
        std::string fileName = request->filename();
        std::string curfileName = (fileName).c_str();
        std::string tmp_filename = (fileName + ".tmp").c_str();
        // Check cache to see if the client exists - if so open the file and write the entire content and close it and update the cache, if not ignore the write
        std::cout << " Inside CloseFile!" << std::endl;
        //int fileDescriptor = open(tmp_filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        int fileDescriptor = request->filedescriptor();
        close(fileDescriptor);
        if (fileDescriptor != -1) {
            //TEMP FILE
            ssize_t writeResult = write(fileDescriptor, newContent.c_str(), newContent.size());
            if (writeResult == -1) {
                reply->set_status(-1);
                close(fileDescriptor);
                unlink(tmp_filename.c_str());
                return Status::OK;
            }
            int closeResult = close(fileDescriptor);
            if(closeResult == -1){
                reply->set_status(-1);
                return Status::OK;
            }
            if (rename(tmp_filename.c_str(), fileName.c_str()) == -1) {
                unlink(tmp_filename.c_str());
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
        std::string filename = request->filename();
        int flags = request->flags();
        std::ifstream f(filename);
        struct stat file_info;

        int fd = open((filename).c_str(), flags);
        std::cout << "Printing filename,  fd, and flags " << filename << " " << fd << " " << flags << std::endl;
        if (fd < 0)
            std::cout << "Cannot create file " << filename << std::endl;
        if (fd != -1) {
            FileInfo* fileInfo = new FileInfo;
            //Call get Attribute 
            if (fstat(fd, &file_info) == -1) {
                reply->set_status(-1);
                return Status::OK;
            }

            //Read whole file
            int sz = lseek(fd, 0L, SEEK_END);
            lseek(fd, 0L, SEEK_SET);
            char *buffer = new char[sz];
            std::cout << "size = " << sz << std::endl;
            if(sz == 0){
                reply->set_data("");
            }
            else{
                int err = read(fd, buffer, sz);
                reply->set_data(buffer);
            }

            setFileInfo(fileInfo, file_info);
            //std::cout << sz << "HSdjsdbj" << std::endl;
           // std::string obuffer = buffer;

            reply->set_allocated_fileinfo(fileInfo);
            close(fd);

        }
        else{
            reply->set_status(-1);
        }

        return Status::OK;
     }

    //Handle cache

    Status DeleteFile(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "Inside Delete File!\n";
        std::string fileName = request->filename();
        int unlinkResult = unlink((fileName).c_str());
        if (unlinkResult != -1) {
            std::cout << "Delete was done\n";
            reply->set_status(-1);
            return Status::OK;
        } else {
            std::cout << " Delete Failed with error " << errno << "\n";
        }
        reply->set_status(1);
        return Status::OK;
    }

    Status RemoveDir(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::string dirName = request->data();
        int result = rmdir((dirName).c_str());
        if (result == -1) {
            reply->set_status(-1);
            return Status::OK;
        }
        reply->set_status(1);
        return Status::OK;

    }

    Status CreateDir(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << " Inside CreateDir" << std::endl;
        std::string dirName = request->data();
        int mode = request->mode();
        int result = mkdir((dirName).c_str(), mode);
        if (result == -1) {
            reply->set_status(-1);
            return Status::OK;
        }
        reply->set_status(1);
        return Status::OK;
    }

    Status GetAttr(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "Inside GetAttr\n";
        std::string filename = request->filename();
        FileInfo* fileInfo = new FileInfo();// =  reply->mutable_data();
        int file_descriptor = open((filename).c_str(), O_RDONLY);
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
        setFileInfo(fileInfo, file_info);
        close(file_descriptor);
        reply->set_allocated_fileinfo(fileInfo);
        reply->set_status(1);
        return Status::OK;
    }

    Status StatFS(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
        std::cout << "Inside StatFS\n";
        std::string filename = request->filename();
        Statfs* statfs_obj = new Statfs();// =  reply->mutable_data();

        struct statfs file_info;
        if (statfs(filename.c_str(), &file_info) == -1) {
            //The file information could not be retrieved.
            reply->set_status(-1);
            return Status::OK;
        }
        setStatFS(statfs_obj, file_info);
        reply->set_allocated_statfs(statfs_obj);
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

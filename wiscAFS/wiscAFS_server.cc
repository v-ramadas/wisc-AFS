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
#include <sys/statvfs.h>
#include <sys/xattr.h>
#include <dirent.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerWriter;
using wiscAFS::RPCDirReply;
using grpc::Status;
using wiscAFS::AFSController;
using wiscAFS::RPCRequest;
using wiscAFS::RPCResponse;
using wiscAFS::FileInfo;
using wiscAFS::Statfs;
using grpc::ServerReader;
using grpc::ServerWriter;


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

    void setStatFS(Statfs* statFS, struct statvfs info) {
        statFS->set_f_bsize(info.f_bsize);
        statFS->set_f_frsize(info.f_frsize);
        statFS->set_f_blocks(info.f_blocks);
        statFS->set_f_bfree(info.f_bfree);
        statFS->set_f_bavail(info.f_bavail);
        statFS->set_f_favail(info.f_favail);
        statFS->set_f_files(info.f_files);
        statFS->set_f_ffree(info.f_ffree);
        statFS->set_f_fsid(info.f_fsid);
        statFS->set_f_namemax(info.f_namemax);
        statFS->set_f_flag(info.f_flag);
    }

   /* Status OpenFile(ServerContext* context, const RPCRequest* request,RPCResponse* reply) override {
        // Error handle the path and filename
        std::string filename = request->filename();
        int flags = request->flags();
        std::ifstream f(filename);
        struct stat file_info;

        std::cout << "WiscServer: Entering OpenFile\n";

        int fd = open((filename).c_str(), flags);
        std::cout << "Printing filename,  fd, and flags " << filename << " " << fd << " " << flags << std::endl;
        if (fd < 0)
            std::cout << "Cannot open file " << filename << std::endl;
        FileInfo *fileInfo = new FileInfo;
        if (fd != -1) {
            //Call get Attribute 
            if (fstat(fd, &file_info) == -1) {
                reply->set_status(-1);
                reply->set_error(errno);
                std::cout << "WiscServer: Exiting OpenFile\n";
                return Status::OK;
            }

            //Read whole file
            int sz = lseek(fd, 0L, SEEK_END);
            lseek(fd, 0L, SEEK_SET);
            char *buffer = new char[sz];
            std::cout << "wiscServer:OpenFile: size = " << sz << std::endl;
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

        reply->set_error(errno);
        std::cout << "WiscServer: Exiting OpenFile\n";

        return Status::OK;
    }*/

    Status OpenFile(ServerContext* context, const RPCRequest* request,ServerWriter<RPCResponse>* writer) override {
        // Error handle the path and filename
        std::string filename = request->filename();
        int flags = request->flags();
        std::ifstream f(filename);
        struct stat file_info;
        RPCResponse reply;

        std::cout << "WiscServer: Entering OpenFile\n";

        int fd = open((filename).c_str(), flags);
        std::cout << "Printing filename,  fd, and flags " << filename << " " << fd << " " << flags << std::endl;
        if (fd < 0){
            reply.set_status(-1);
            reply.set_error(errno);
            std::cout << "wiscServer: OpenFile: Cannot open file " << filename << std::endl;
            return Status::OK;
        }
        
        FileInfo *fileInfo = new FileInfo;
        char *buf = new char[1024];
        if (fstat(fd, &file_info) == -1) {
                reply.set_status(-1);
                reply.set_error(errno);
                std::cout << "WiscServer: OpenFile: Cannot fstat exiting OpenFile\n";
                return Status::OK;
        }
        setFileInfo(fileInfo, file_info);
        reply.set_allocated_fileinfo(fileInfo);

        int sz = lseek(fd, 0L, SEEK_END);
        lseek(fd, 0L, SEEK_SET);
        std::cout << "wiscServer:OpenFile: total size = " << sz << std::endl;

        while(1){
            int bytesRead;
            if (sz == 0){
                reply.set_data("");
                writer->Write(reply);
                break;
            }
            if (sz < 1024){
                bytesRead = read(fd, buf, sz);
            }
            else{
                bytesRead = read(fd, buf, 1024);
            }
            std::cout << "wiscServer: OpenFile: Sending bytesRead, data " << bytesRead << ", " << buf << std::endl;
            if(bytesRead == 0){
                break;
            }

            if(bytesRead == -1){
                std::cout << "ERROR: wiscServer: OpenFile:Cannot read file " << filename << std::endl;
                reply.set_status(-1);
                reply.set_error(errno);
                return Status::OK;
            }

            reply.set_data(buf);
            reply.set_filesize(bytesRead);
            writer->Write(reply);
        }
     
        close(fd);
        free(buf);
        std::cout << "WiscServer: Exiting OpenFile\n";
        reply.set_status(1);

        return Status::OK;
    }

    // Status CloseFile(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
    //     //void *newContent; 
    //     //newContent = (void*)malloc(1025);
    //     //memcpy(newContent, (void*)request->data(), 1024);
    //     std::string newContent = request->data();
    //     std::string fileName = request->filename();
    //     std::string curFileName = (fileName);
    //     std::string tmpFileName = (fileName + ".tmp");
    //     std::cout << " Inside CloseFile!" << std::endl;
    //     std::cout << " newContent = " << newContent << std::endl;
    //     std::cout << "curFileName =  " <<  curFileName << std::endl;
    //     std::cout << "tmpFileName =  " << tmpFileName << std::endl;
    //     // Check cache to see if the client exists - if so open the file and write the entire content and close it and update the cache, if not ignore the write
    //     int tfd = open(tmpFileName.c_str(), O_CREAT|O_RDWR|O_TRUNC, 0777);
    //     if (tfd == -1) {
    //         std::cout << "wiscServer:CloseFIle: Failed to open tmp file\n";
    //     }
    //     unsigned long int fileSize = request->filesize();
    //     std::cout << "wiscSerer: CloseFile: FileSize = " << fileSize << std::endl;
    //     int sz = write(tfd, newContent.c_str(), fileSize);
    //     //std::cout << "wiscServer:CloseFIle: newContent.strlen() == " << strlen(newContent.c_str());
    //     if(sz < 0) {
    //         std::cout << "wiscServer:CloseFile: Writing to tmp file failed\n";
    //     }
    //     close(tfd);
    //     int ret = rename (tmpFileName.c_str(), curFileName.c_str());
    //     if (ret < 0) {
    //         std::cout << "wiscServer:CloseFIle: Rename failed, returning bad status\n";
    //         reply->set_status(-1);
    //         return Status::OK;
    //     }
    //     else{
    //         std::cout << "wiscServer:CloseFIle: Rename success, returning status\n";
    //         unlink(tmpFileName.c_str());
    //         reply->set_status(0);
    //         return Status::OK;
    //     }


    //     //int fileDescriptor = open(tmp_filename.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    //     /*int fileDescriptor = request->filedescriptor();
    //     close(fileDescriptor);
    //     if (fileDescriptor != -1) {
    //         //TEMP FILE
    //         ssize_t writeResult = write(fileDescriptor, newContent.c_str(), newContent.size());
    //         if (writeResult == -1) {
    //             reply->set_status(-1);
    //             reply->set_error(errno);
    //             close(fileDescriptor);
    //             unlink(tmp_filename.c_str());
    //             std::cout << "wiscServer: Exiting CloseFile\n";
    //             return Status::OK;
    //         }
    //         int closeResult = close(fileDescriptor);
    //         if(closeResult == -1){
    //             reply->set_status(-1);
    //             reply->set_error(errno);
    //             std::cout << "wiscServer: Exiting CloseFile\n";
    //             return Status::OK;
    //         }
    //         if (rename(tmp_filename.c_str(), fileName.c_str()) == -1) {
    //             unlink(tmp_filename.c_str());
    //             std::cout << "wiscServer: Exiting CloseFile\n";
    //             return Status::OK;
    //         }
    //         reply->set_status(1);
    //     }
    //     else{
    //         reply->set_status(-1);
    //     }
    //     reply->set_error(errno);
    //     std::cout << "wiscServer: Exiting CloseFile\n";
    //     return Status::OK;*/
    // }


    Status CloseFile(ServerContext* context, ServerReader<RPCRequest>* reader, RPCResponse* reply) override {
        int fd, res;
        bool firstReq = true;
        std::string path, tempPath;
        RPCRequest request;
        while (reader->Read(&request)) {
            if(firstReq){
                path = request.filename();
                tempPath = (path + ".tmp");
                fd = open(tempPath.c_str(), O_CREAT|O_RDWR, 0644);
                if(fd == -1){
                    std::cout << "wiscServer:CloseFIle: Failed to open tmp file\n";
                    return grpc::Status(grpc::StatusCode::NOT_FOUND, "custom error msg");
                }
                firstReq = false;
            }

            res = write(fd, request.data().c_str(), request.filesize());
            if(res == -1){
                std::cout << "wiscServer:CloseFile: Writing to tmp file failed\n";
                close(fd);
                return grpc::Status(grpc::StatusCode::NOT_FOUND, "custom error msg");
            }
        }

        fsync(fd);
        close(fd);

        int ret = rename (tempPath.c_str(), path.c_str());
        if (ret < 0) {
            std::cout << "wiscServer:CloseFIle: Rename failed, returning bad status\n";
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        }
        else{
            std::cout << "wiscServer:CloseFIle: Rename success, returning status\n";
            unlink(tempPath.c_str());
            reply->set_status(0);
            return Status::OK;
        }
    }

    Status RenameFile(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
        std::string oldname = request->filename();
        std::string newname = request->newfilename();
        int ret = rename(oldname.c_str(), newname.c_str());
        if (ret == -1) {
            reply->set_status(-1);
            reply->set_error(errno);
            std::cout << "WiscServer: RenameFile Failed\n";
            return Status::OK;
        }
        reply->set_status(1);
        return Status::OK;
    }

    /* Generally CreateFile would create a file if it doesn't exisit or overtie the file into a new file, here we will reply on first implementation only for now.*/
     Status CreateFile(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
        // Error handle the path and filename
        std::string filename = request->filename();
        int flags = request->flags();
        int mode = request->mode();
        std::ifstream f(filename);
        struct stat file_info;

        std::cout << "WiscServer: Entering CreateFile\n";

        int fd = open((filename).c_str(), flags, mode);
        std::cout << "wiscServer:CreateFilePrinting filename,  fd, flags, mode " << filename << ", " << fd << ", " << flags << ", " <<  mode << std::endl;
        if (fd < 0){
            std::cout << "ERROR: Cannot open file " << filename << std::endl;
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        }
        FileInfo *fileInfo = new FileInfo;
        if (fd != -1) {
            //Call get Attribute 
            if (fstat(fd, &file_info) == -1) {
                reply->set_status(-1);
                reply->set_error(errno);
                std::cout << "WiscServer:CreateFile fstat failed\n";
                return Status::OK;
            }

            //Read whole file
            int sz = lseek(fd, 0L, SEEK_END);
            lseek(fd, 0L, SEEK_SET);
            char *buffer = new char[sz];
            std::cout << "wiscServer:CreateFile: size = " << sz << std::endl;
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

        reply->set_error(errno);
        std::cout << "WiscServer:CreateFile: Exiting OpenFile\n";

        return Status::OK;
    }

    Status RemoveDir(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "wiscServer: Entering RemoveDir\n";
        std::string dirName = request->filename();
        int result = rmdir((dirName).c_str());
        if (result == -1) {
            reply->set_status(-1);
        } else {
            reply->set_status(1);
        }
        reply->set_error(errno);
        std::cout << "wiscServer: Exiting RemoveDir\n";
        return Status::OK;

    }

    Status CreateDir(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "wiscServer:CreateDir: Inside CreateDir" << std::endl;
        std::string dirName = request->filename();
        int mode = request->mode();
        std::cout << "mkdir with dirname = " << dirName << "mode = " << mode << std::endl;
        int result = mkdir((dirName).c_str(), mode);
        if (result == -1) {
            std::cout << "wiscServer:CreateDir: mkdir failed" << std::endl;
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        } else {
            reply->set_status(1);
        }
        reply->set_error(errno);
        std::cout << "wiscServer:CreateDir: Exiting RemoveDir\n";
        return Status::OK;
    }

    Status DeleteFile(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "wiscServer:DeleteFile: Inside DeleteFile" << std::endl;
        std::string filename = request->filename();
        std::cout << "file with filename = " << filename << std::endl;
        int result = unlink((filename).c_str());
        if (result == -1) {
            std::cout << "wiscServer:DeleteFile: DeleteFile failed" << std::endl;
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        } else {
            reply->set_status(1);
        }
        std::cout << "wiscServer:DeleteFile: Exiting DeleteFile\n";
        return Status::OK;
    }

    // Status OpenDir(ServerContext* context, const RPCRequest* request,
    //         RPCResponse* reply) override {
        
    //     std::string dirName = request->filename();
    //     reply->set_data("Hello + " + dirName);
    //     DIR* result = opendir((dirName).c_str());
    //     if (result == nullptr) {
    //         reply->set_status(errno);
    //         return Status::OK;
    //     }
    //     reply->set_status(1);
    //     reply->set_pointer(result);
    //     return Status::OK;
    // }

    Status AccessFile(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
     
        std::string filename = request->filename();
        int ret = access(filename.c_str(), request->mode());
        if (ret == -1){
            std::cout <<"wiscServer:AccessFile: Access issue with the file, retuning errno and -1\n";
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        }
        reply->set_status(1);
        return Status::OK;
    }

    Status ReadDir(ServerContext* context, const RPCRequest* request,
		  ServerWriter<RPCDirReply>* writer) override {

		DIR *dp;
		struct dirent *de;
		RPCDirReply reply;

		dp = opendir((request->filename()).c_str());
		if (dp == NULL){
			std::cout<<"opendir dp null"<<std::endl;
            perror(strerror(errno));
			reply.set_error(errno);
            reply.set_status(-1);
		    writer->Write(reply);
            return grpc::Status::OK;
		}

		while((de = readdir(dp)) != NULL){
            std::cout<< "Directory names seen from the server " << de->d_name;
		    reply.set_dino(de->d_ino);
		    reply.set_dname(de->d_name);
		    reply.set_dtype(de->d_type);
            reply.set_status(1);
		    writer->Write(reply);
		}
		reply.set_error(0);
        closedir(dp);
        reply.set_status(1);
		return Status::OK;
  }



    Status GetAttr(ServerContext* context, const RPCRequest* request,
            RPCResponse* reply) override {
        std::cout << "wiscServer: Inside GetAttr\n";
        std::string filename = request->filename();
        FileInfo* fileInfo = new FileInfo();// =  reply->mutable_data();
        //int file_descriptor = open((filename).c_str(), O_RDONLY);
        //if (file_descriptor == -1) {
        //    //The file could not be opened
        //    reply->set_status(-1);
        //    return Status::OK;
        //}

        struct stat file_info;
        if (lstat(filename.c_str(), &file_info) == -1) {
            //The file information could not be retrieved.
            std::cout << "wiscServer: lstat failed for " << filename << " returning -1\n";
            reply->set_status(-1);
            reply->set_error(errno);
            return Status::OK;
        } else {
            setFileInfo(fileInfo, file_info);
            //close(file_descriptor);
            reply->set_allocated_fileinfo(fileInfo);
            reply->set_status(1);
        }
        reply->set_error(errno);
        std::cout << "wiscServer: Exiting GetAttr\n";
        return Status::OK;
    }

    Status GetXAttr(ServerContext* context, const RPCRequest* request,
             RPCResponse* reply) override {
         std::cout << "wiscServer: Inside GetXAttr\n";
         std::string filename = request->filename();
         std::string xattr_name = request->xattr_name();
         std::string xattr_value = request->xattr_value();
         size_t xattr_size = request->xattr_size();
         char* value = new char[1024];
         int size = getxattr(filename.c_str(), xattr_name.c_str(), value, xattr_size);
         std::string s_value = value;
         reply->set_xattr_value(s_value);
         reply->set_xattr_size(size);
         if (size < 0) {
           std::cout << "wiscServer: Exiting GetXAttr\n";
           reply->set_status(-1);
           reply->set_error(errno);
           return Status::OK;
         }
         reply->set_status(1);
         reply->set_error(errno);
         std::cout << "wiscServer: Exiting GetXAttr\n";
         return Status::OK;
     }


    Status StatFS(ServerContext* context, const RPCRequest* request, RPCResponse* reply) override {
        std::cout << "wiscServer: Inside StatFS\n";
        std::string filename = request->filename();
        Statfs* statfs_obj = new Statfs();// =  reply->mutable_data();

        struct statvfs file_info;
        if (statvfs(filename.c_str(), &file_info) == -1) {
            //The file information could not be retrieved.
            reply->set_status(-1);
        } else {
            setStatFS(statfs_obj, file_info);
            reply->set_allocated_statfs(statfs_obj);
            reply->set_status(1);
        }
        std::cout << "wiscServer: Exiting StatFS\n";
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

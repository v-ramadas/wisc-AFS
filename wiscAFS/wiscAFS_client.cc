#include "wiscAFS_client.hh"
#include "cache/ClientCache.h"
#include <dirent.h>
#include <fuse.h>
#include <errno.h>
#include <sys/xattr.h>

using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientWriter;


#ifdef __cplusplus
extern "C" {
#endif

void wiscAFSClient::setFileInfo(FileInfo* fileInfo, struct stat info) {
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


RPCResponse wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
      
   int semr = sem_wait(&clientSem);
   if (semr == -1){
       std::cout << "wiscClient:OpenFile: sem_wait failed\n";
   }
   std::cout << "wiscClient:OpenFile: Start filename = " << filename << ", flags = " << flags << "\n";
   RPCRequest request;
   request.set_filename(filename);
   request.set_flags(flags);

   // Container for the data we expect from the server.
   RPCResponse reply;
   CacheFileInfo fileatts;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;
   // The actual RPC.
   std::unique_ptr<ClientReader<RPCResponse>> reader (stub_->OpenFile(&context, request));

   int count = 0;
   int fileDescriptor;
   std::string local_path;
   while(reader->Read(&reply)){
       if(reply.error() != 0){
           reply.set_status(-1);
           sem_post(&clientSem);
           return reply;
       }

       if (count == 0){
           local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
           std::cout << "wiscClient:OpenFile: created temp file = " << local_path << " filename = " << filename << ", flags = " << flags << "\n";
           fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
           if (fileDescriptor < 0){
               std::cout << "CERROR: wiscClient:OpenFile: Cannot open temp file= " << local_path << " filename = " << filename << "\n";
               reply.set_status(-1);
               reply.set_error(errno);
               sem_post(&clientSem);
               return reply;
           }
           fileatts.setFileInfo(&reply.fileinfo());
           //TODO Now checking whether cache entry exists instead of assuming deleted
           /*ClientCacheValue ccv(fileatts, false, fileDescriptor);
           ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
           diskCache.addCacheValue(filename, ccv);*/
           ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
           if (ccv1 == nullptr){
               ClientCacheValue ccv(fileatts, false, fileDescriptor);
               ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
               diskCache.addCacheValue(filename, ccv);
           }
           else{
               ccv1->fileInfo = fileatts;
               ccv1->isDirty = false;
               ccv1->fileDescriptor = fileDescriptor;
               diskCache.updateCacheValue(filename, *ccv1);
           }
       }
       int res = write(fileDescriptor, reply.data().c_str(), reply.filesize());
       if (res == -1) {
           std::cout << "CERROR: wiscClient:OpenFile: Cannot write to temp file= " << local_path << " filename = " << filename << "\n";
           reply.set_status(-1);
           reply.set_error(errno);
           sem_post(&clientSem);
           return reply;
       }
       count++;

   }
   Status status = reader->Finish();
   //TODO Deliberately removed this because we want full permission
   /*close(fileDescriptor);
   //Now that the temp cache file is written change the mode to the original file
   //int chmod_ret = chmod(local_path.c_str(), reply.fileinfo().st_mode());
   fileDescriptor = open(local_path.c_str(),  flags);
   if (fileDescriptor < 0){
        std::cout << "CERROR: wiscClient:OpenFile: could not reopen with correct flags temp file= " << local_path << " filename = " << filename << "\n";
        reply.set_status(-1);
        reply.set_error(errno);
        sem_post(&clientSem);
        return reply;
   }*/
   reply.set_file_descriptor(fileDescriptor);
   std::cout << "wiscClient:OpenFile: End local_path = " << local_path << " file = " << filename << "  fd =  " << fileDescriptor << std::endl;

   // TODO: check if the read from server finished or not. 
   // TODO: If not, then retry the whole operation

   sem_post(&clientSem);
   return reply;
}

RPCResponse wiscAFSClient::CloseFile(const std::string& filename, bool release) {
    // Data we are sending to the server.
    // DELTE LOCAL FILES POST THIS
    int semr = sem_wait(&clientSem);
    if (semr == -1){
        std::cout << "wiscClient:closeFile: sem_wait failed\n";
    }
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    std::cout << "wiscClient:CloseFile: Start filename = " << filename << ", release = " << release << "\n";
    RPCRequest request;
    RPCResponse reply;
    unsigned long int sz;
    if(ccv1 == nullptr){
        errno=ENOENT;
        reply.set_status(-1);
        reply.set_error(errno);
        std::cout << "CERROR: wiscClient:CloseFile: no cache entry filename = " << filename << ", release = " << release << "\n";
        sem_post(&clientSem);
        return reply;
    }
    else if(!ccv1->isDirty){
        if (release) {
            std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
            diskCache.deleteCacheValue(filename);
            std::cout << "wiscClient: CloseFile: Deleting non dirty cache entry filename = " << filename << " delete temp file path = " << local_path << std::endl;
            unlink(local_path.c_str());
        }
        reply.set_status(1);
        sem_post(&clientSem);
        return reply;
    }
    else{
        std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
        std::cout << "wiscClient:CloseFile: Opening local file = " << local_path << " filename = " << filename << ", release = " << release << "\n";
        int fd = open(local_path.c_str(),  O_RDONLY);
        if (fd == -1){
            std::cout << "CERROR: wiscClient:CloseFile: couldn't open local file = " << local_path << " filename = " << filename << ", release = " << release << "\n";
            reply.set_status(-1);
            reply.set_error(errno);
            sem_post(&clientSem);
            return reply;
        }
        sz = lseek(fd, 0L, SEEK_END);
        lseek(fd, 0L, SEEK_SET);
        request.set_filename(filename);
        request.set_filedescriptor(ccv1->fileDescriptor);
        ClientContext context;
        //std::cout<< "wiscClient:CloseFile Calling stub->closefile " <<  std::endl;
        std::unique_ptr<ClientWriter<RPCRequest>> writer(stub_->CloseFile(&context, &reply));
        std::cout << "wiscClient:CloseFile: printing bufs for local file = " << local_path << " filename = " << filename << ", release = " << release << "\n";
        while(1){
            char *buf = new char[4096];
            int bytesRead;
            if (sz < 1024){
                bytesRead = read(fd, buf, sz);
                //buf[bytesRead] = '\0';
            }
            else{
                bytesRead = read(fd, buf, 1024);
                //buf[bytesRead] = '\0';
            }
            if (bytesRead == 0) {
                break;
            }
            if (bytesRead == -1) {
                std::cout << "CERROR: wiscClient:CloseFile: read error in local file = " << local_path << " filename = " << filename << ", release = " << release << "\n";
                reply.set_status(-1);
                reply.set_error(errno);
                sem_post(&clientSem);
                return reply;
            }
            //for (int l = 0; l < bytesRead; l++){
            //    std::cout << buf[l];
            //}
            //std::cout << "\nRe-reading request object datan\n";
            request.set_data(std::string(buf, bytesRead));
            request.set_filesize(bytesRead);
            //const char* obj_data = request.data().c_str();
            //for (int l = 0; l < bytesRead; l++){
            //    std::cout << obj_data[l];
            //}
            //std::cout << "\n";

            if (!writer->Write(request)) {
                std::cout << "CERROR: wiscClient: CloseFile: stream broke:" << errno << std::endl;
                reply.set_status(-1);
                reply.set_error(errno);
                sem_post(&clientSem);
                return reply;
            }
            free(buf);
        }

        writer->WritesDone();
        Status status = writer->Finish();
        close(fd);
        if (release) {
            diskCache.deleteCacheValue(filename);
            std::cout << "wiscClient: CloseFile: Deleting cache entry filename = " << filename << " delete temp file path = " << local_path << std::endl;
            unlink(local_path.c_str());
        }
        reply.set_status(1);
        std::cout << "wiscClient: CloseFile: End file " << filename << " temp file path = " << local_path << " sz = " << sz << std::endl;
      
        sem_post(&clientSem);
        return reply;
    }
}

RPCResponse wiscAFSClient::RenameFile(const std::string& oldname, const std::string& newname) {
    sem_wait(&clientSem);
    std::cout << "wiscClient: Entering RenameFile\n";
    ClientCacheValue *ccv = diskCache.getCacheValue(oldname);

    RPCResponse reply;
    if (ccv != nullptr) {
        diskCache.renameCacheValue(oldname, newname);
        ccv = diskCache.getCacheValue(newname);
        if (ccv == nullptr) {
            std::cout <<"wiscCient: Rename Failed on cache copy\n";
            reply.set_status(-1);
            reply.set_error(EACCES);
            sem_post(&clientSem);
            return reply;
        }
    }

    ClientContext context;

    RPCRequest request;
    request.set_filename(oldname);
    request.set_newfilename(newname);
    Status status = stub_->RenameFile(&context, request, &reply);
    if (status.ok()) {
       std::cout << "wiscClient:RenameFile Reply status " << reply.status() << std::endl;
       std::cout << "wiscClient:Exiting RenameFile\n";
    }
    else {
       std::cout << "wiscClient:RenameFile Failed! Exiting RenameFile\n";
       errno = reply.error();
    }

    sem_post(&clientSem);
    return reply;

}


int wiscAFSClient::ReadFile(const std::string& filename) {
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        errno=ENOENT;
    }
    else{
        return ccv1->fileDescriptor;
    }
}

//Returning either fD or error
int wiscAFSClient::WriteFile(const std::string& filename){
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        std::cout<<"wiscClient: WriteFile: cache value not found\n";
        errno=ENOENT;
    }
    else{
        if(!ccv1->isDirty){
            ccv1->isDirty = true;
            diskCache.updateCacheValue(filename, *ccv1);
            std::cout<<"wiscClient: WriteFile: cache value set dirty\n";
        }
        else{
            std::cout<<"wiscClient: WriteFile: cache value already dirty\n";
        }
        return ccv1->fileDescriptor;
    }
}

RPCResponse wiscAFSClient::CreateFile(const std::string& filename, const int flags, const int mode) {
   int semr = sem_wait(&clientSem);
   if (semr == -1){
       std::cout << "wiscClient:createFile: sem_wait failed\n";
   }
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   //std::cout << "wiscClient:CreateFile: Entering CreateFile\n";
   RPCRequest request;
   request.set_filename(filename);
   request.set_flags(flags);
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
       //std::cout << "wiscClient:CreateFile Reply status " << reply.status() << std::endl;
       std::string local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
       int fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
       if (fileDescriptor < 0) {
           std::cout << "MYERROR: wiscClient:CreateFile: Cannot open temp file " << local_path << std::endl;
           reply.set_status(-1);
           reply.set_error(errno);
           sem_post(&clientSem);
           return reply;
       }

       if (fileDescriptor != -1) {
           std::cout << "wiscClient: CreateFile: Reply data received at client = " << reply.data() << std::endl;
           ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
           printf("wiscClient:CreateFile: writeResult = %ld\n", writeResult);
           //SUCCESS
           printf("wiscClient:CreateFile: Printing fileatts = %ld, %ld, %ld\n", reply.fileinfo().st_size(),reply.fileinfo().st_atim(),reply.fileinfo().st_mtim());
           CacheFileInfo fileatts;
           fileatts.setFileInfo(&reply.fileinfo());
           ClientCacheValue ccv(fileatts, false, fileDescriptor);
           ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
           std::cout << "wiscClient:CreateFile: filename = " << filename << ", local_path = " << local_path << " Set reply.inode() = " << reply.fileinfo().st_ino() << " ccv.inode = " << ccv.fileInfo.st_ino << std::endl;
           diskCache.addCacheValue(filename, ccv);
           reply.set_status(1);
           reply.set_file_descriptor(fileDescriptor);
           sem_post(&clientSem);
           return reply;
       } 
       else {
           std::cout << "wiscClient:CreateFile Exiting CreateFile\n";
           reply.set_status(-1);
           reply.set_error(errno);
           sem_post(&clientSem);
           return reply;
        }
   }
   else {
       std::cout << "wiscClient:CreateFile Exiting CreateFile\n";
       reply.set_status(-1);
       reply.set_error(status.error_code());
       sem_post(&clientSem);
       return reply;
   }

}

RPCResponse wiscAFSClient::DeleteFile(const std::string& filename) {
   sem_wait(&clientSem);
   std::cout << "wiscClient: Entering DeleteFile\n";
   RPCRequest request;
   request.set_filename(filename);
   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   // The actual RPC.
   Status status = stub_->DeleteFile(&context, request, &reply);

   ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   if (!status.ok()){
       reply.set_status(-1);
       reply.set_error(status.error_code());
   }
   if(ccv1 == nullptr){
       std::cout << "wiscClient:DeleteFile: Cache entry not found\n";
   }
   else {
       diskCache.deleteCacheValue(filename);
   }

   std::cout << "wiscClient: Exiting DeleteFile\n";
   errno = reply.error();
   sem_post(&clientSem);
   return reply;

}

RPCResponse wiscAFSClient::CreateDir(const std::string& dirname, const int mode) {
   // Data we are sending to the server.
   std::cout << "wiscClient: Entering CreateDir\n";
   RPCRequest request;
   request.set_filename(dirname);
   std::cout<<mode;
   request.set_mode(mode);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;
   // The actual RPC.
   Status status = stub_->CreateDir(&context, request, &reply);
   // Act upon its status.
   if (status.ok()) {
       std::cout << "wiscClient: CreateDir: Received status ok\n";
   } else {
       std::cout << status.error_code() << ": " << status.error_message() << std::endl;
       reply.set_status(-1);
       reply.set_error(status.error_code());
   }
   std::cout << "wiscClient: Exiting CreateDir\n";
   errno = reply.error();
   return reply;
}

RPCResponse wiscAFSClient::OpenDir(const std::string& dirname, const int mode) {

    std::cout << "wiscClient: Entering OpenDir\n";
    RPCRequest request;
    request.set_filename(dirname);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->OpenDir(&context, request, &reply);

    // Act upon its status.
    if (!status.ok()) {
        std::cout << status.error_code() << ": " << status.error_message()
            << std::endl;
        reply.set_status(-1);
    }
    errno = reply.error();
    std::cout << "wiscClient: Exiting OpenDir\n";
    return reply;
}

RPCResponse wiscAFSClient::TruncateFile(const std::string& filename, const int length) {

    std::cout << "wiscClient: Entering TruncateFile\n";
    RPCResponse reply;
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if (ccv1 == nullptr){
        std::cout << "CERROR: wiscClient:TruncateFile: Cache entry does not exist filename = " << filename << std::endl;
        reply.set_error(errno);
        reply.set_status(-1);
        return reply;
    }
    else{
        std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
        int trunc_ret = truncate(local_path.c_str(), length);
        if (trunc_ret == -1){
            std::cout << "CERROR: wiscClient:TruncateFile: Truncate failed filename = " << filename << std::endl;
            reply.set_error(errno);
            reply.set_status(-1);
            return reply;
        }
    }

    // Container for the data we expect from the server.

    reply.set_status(1);
    std::cout << "wiscClient: Exiting TruncateFile\n";
    return reply;
}

RPCResponse wiscAFSClient::AccessFile(const std::string& filename, const int mode) {

    std::cout << "wiscClient: Entering AccessFile\n";
    RPCRequest request;
    request.set_filename(filename);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->AccessFile(&context, request, &reply);

    // Act upon its status.
    if (!status.ok()) {
        std::cout << "wiscClient: AccessFile: error code" << status.error_code() << ": " << status.error_message() << std::endl;
        reply.set_status(-1);
        reply.set_error(status.error_code());
    }
    errno = reply.error();
    reply.set_status(1);
    std::cout << "wiscClient: Exiting AccessFile\n";
    return reply;
}

RPCDirReply wiscAFSClient::ReadDir(const std::string& p, void *buf, fuse_fill_dir_t filler) {
    RPCRequest request;
    std::cout << "wiscClient: Entering ReadDir" << std::endl;
    std::cout << p << std::endl;
    request.set_filename(p);
    RPCDirReply reply;
    dirent de;
    reply.set_error(-1);
    ClientContext context;
    std::unique_ptr<ClientReader<RPCDirReply>> reader(stub_->ReadDir(&context, request));
    while(reader->Read(&reply)){
        if (reply.status() == -1){
            return reply;
        }
        struct stat st;
        memset(&st, 0, sizeof(st));

        de.d_ino = reply.dino();
        strcpy(de.d_name, reply.dname().c_str());
        de.d_type = reply.dtype();

        st.st_ino = de.d_ino;
        st.st_mode = de.d_type << 12;
        std::cout << de.d_ino << " d_tpye: " << de.d_type << " st_ino: " << st.st_ino << std::endl;
        if (filler(buf, de.d_name, &st, 0))
            break;
    }
    Status status = reader->Finish();
    std::cout << "wiscClient: Exiting ReadDir\n";
    errno = reply.error();
    return reply;
}

RPCResponse wiscAFSClient::RemoveDir (const std::string& dirname) {
    // Data we are sending to the server.
    std::cout << "wiscClient: Entering RemoveDir\n";
    RPCRequest request;
    request.set_filename(dirname);
    request.set_path(dirname);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->RemoveDir(&context, request, &reply);

    // Act upon its status.
    if (!status.ok()) {
        std::cout << "wiscClient:RemoveDir: Failed"<< status.error_code() << ": " << status.error_message() << std::endl;
        reply.set_status(-1);
        reply.set_error(status.error_code());
    }

    std::cout << "wiscClient: Exiting RemoveDir\n";
    errno = reply.error();
    return reply;
}

RPCResponse wiscAFSClient::GetAttr(const std::string& filename) {
   // Data we are sending to the server.
   sem_wait(&clientSem);
   std::cout << "wiscClient:GetAttr: Entering GetAttr\n";
   ClientCacheValue *ccv = diskCache.getCacheValue(filename);
   RPCRequest request;
   request.set_filename(filename);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   if (ccv == nullptr) {
       // The actual RPC.
        Status status = stub_->GetAttr(&context, request, &reply);
        // Act upon its status.
        if (!status.ok()) {
            reply.set_status(-1);
            reply.set_error(status.error_code());
        }
    } else {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        struct stat buf;
        lstat(local_path.c_str(), &buf);
        FileInfo* fileInfo = new FileInfo();
        setFileInfo(fileInfo, buf);
        reply.set_allocated_fileinfo(fileInfo);
        reply.set_status(0);
    }


   std::cout << "wiscClient:GetAttr: Exiting GetAttr\n";
   errno = reply.error();
   sem_post(&clientSem);
   return reply;
 
}

RPCResponse wiscAFSClient::GetXAttr(const std::string& filename, const std::string& name, char* value, size_t size) {
   // Data we are sending to the server.
   std::cout << "wiscClient:GetXAttr: Entering GetXAttr\n";
   ClientCacheValue *ccv = diskCache.getCacheValue(filename);
   RPCRequest request;
   request.set_filename(filename);
   request.set_xattr_name(name);
   request.set_xattr_size(size);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   if (ccv == nullptr) {
        std::cout << "wiscClient:GetXAttr : Making stub call\n";
       // The actual RPC.
        Status status = stub_->GetXAttr(&context, request, &reply);
        // Act upon its status.
        if (!status.ok()) {
            reply.set_status(-1);
            reply.set_error(status.error_code());
        }
    } else {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        std::cout << "wiscClient:GetXAttr : Making local call to " << local_path <<"\n";
        int attrSize = getxattr(local_path.c_str(), name.c_str(), value, size);
        std::string s_value(value);
        reply.set_xattr_value(s_value);
        reply.set_xattr_size(attrSize);
        std::cout << " Size: " << attrSize << "Value : " << s_value << std::endl; 
        reply.set_status(0);
        reply.set_error(errno);
    }

   std::cout << "wiscClient:GetXAttr: Exiting GetXAttr\n";
   errno = reply.error();
   return reply;
 
}

RPCResponse wiscAFSClient::Statfs(const std::string& filename) {

   std::cout << "wiscClient: Entering StatFS\n";
   RPCRequest request;
   request.set_filename(filename);

   RPCResponse reply;

   ClientContext context;

   Status status = stub_->StatFS(&context, request, &reply);

   if (!status.ok()) {
      reply.set_status(-status.error_code());
   }
   errno = reply.error();
   return reply;
   std::cout << "wiscClient: Exiting StatFS\n";
}

RPCResponse wiscAFSClient::Fcntl(const std::string& path, struct fuse_file_info* fi, int cmd, struct flock* fl)
{
    std::cout << "wiscClient: Entering Fcntl\n";

    ClientCacheValue *ccv = diskCache.getCacheValue(path);
    RPCResponse reply;
    if (ccv!=nullptr) {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        std::cout << "wiscClient:CloseFile: Trying to do fcntl on local file = " << local_path << std::endl;
        int ret = fcntl((int) fi->fh, cmd, fl);
        if (ret == -1) {
            reply.set_status(-1);
            reply.set_error(errno);
        } else {
            reply.set_status(0);
        }
    }

    ClientContext context;
    RPCRequest request;

    FileLock *fileLock = new FileLock;
    fileLock->set_l_type(fl->l_type);
    fileLock->set_l_whence(fl->l_whence);
    fileLock->set_l_start(fl->l_start);
    fileLock->set_l_pid(fl->l_pid);

    request.set_allocated_filelock(fileLock);
    request.set_filename(path);
    Status status = stub_->Fcntl(&context, request, &reply);

    if (!status.ok()) {
        reply.set_status(status.error_code());
    }
    std::cout << "wiscClient: Exiting Fcntl\n";
    return reply;
}

RPCResponse wiscAFSClient::Chmod(const std::string& filename, int mode) {
    std::cout << "wiscClient: Entering chmod\n";
    ClientCacheValue *ccv = diskCache.getCacheValue(filename);
    RPCResponse reply;
    ClientContext context;

    if (ccv != nullptr) {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        std::cout << "wiscClient:CloseFile: Trying to do fcntl on local file = " << local_path << std::endl;
        int ret = chmod(filename.c_str(), mode);
        if (ret == -1) {
            reply.set_status(-1);
            reply.set_error(errno);
        } else {
            reply.set_status(0);
        }
    }

    RPCRequest request;
    request.set_filename(filename);
    request.set_mode(mode);
    Status status = stub_->Chmod(&context, request, &reply);

    if (!status.ok()) {
        reply.set_status(status.error_code());
    }
    std::cout << "wiscClient: Exiting Chmod\n";
    return reply;
}

RPCResponse wiscAFSClient::Chown(const std::string& filename, uid_t uid, gid_t gid) {

    std::cout << "wiscClient: Entering Chown\n";
    RPCResponse reply;
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if (ccv1 == nullptr){
        std::cout << "CERROR: wiscClient:Chown: Cache entry does not exist filename = " << filename << std::endl;
        reply.set_error(errno);
        reply.set_status(-1);
        return reply;
    }
    else{
        std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
        int trunc_ret = chown(local_path.c_str(), uid, gid);
        if (trunc_ret == -1){
            std::cout << "CERROR: wiscClient:Chown: Chown failed filename = " << filename << std::endl;
            reply.set_error(errno);
            reply.set_status(-1);
            return reply;
        }
    }

    // Container for the data we expect from the server.

    reply.set_status(1);
    std::cout << "wiscClient: Exiting Chown\n";
    return reply;
}



#ifdef __cplusplus
}
#endif

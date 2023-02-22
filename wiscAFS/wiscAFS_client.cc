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


int wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   std::cout << "wiscClient:OpenFile: Entering OpenFile\n";
   RPCRequest request;
   request.set_filename(filename);
   request.set_flags(flags);

   // Container for the data we expect from the server.
   RPCResponse reply;
   CacheFileInfo fileatts;
   //ClientCacheValue ccv;

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
           return -1;
       }

       if (count == 0){
           std::cout << "wiscClient:OpenFile: Opening temp file and adding entry to cache" << std::endl;
           local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
           fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
           if (fileDescriptor < 0){
               std::cout << "wiscClient:OpenFile: ERROR: Cannot open temp file " << local_path << std::endl;
               return -1;
           }
           fileatts.setFileInfo(&reply.fileinfo());
           ClientCacheValue ccv(fileatts, false, fileDescriptor);
           ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
           diskCache.addCacheValue(filename, ccv);
       }
       int res = write(fileDescriptor, reply.data().c_str(), strlen(reply.data().c_str()));
       if (res == -1) {
           std::cout << "ERROR: Cannot write to temp file " << local_path << std::endl;
       }
       count++;

   }
   Status status = reader->Finish();
   std::cout << "wiscClient:OpenFile: Finished openFile with count = " << count << std::endl;
   // TODO: check if the read from server finished or not. 
   // TODO: If not, then retry the whole operation

   return fileDescriptor;


   // Act upon its status.
//    if (status.ok()) {
//        std::cout << "wiscClient:OpenFile Reply status " << reply.status() << std::endl;
//        std::string local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
//        int fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
//        if (fileDescriptor < 0) {
//            std::cout << "ERROR: Cannot open temp file " << local_path << std::endl;
//        }

//        if (fileDescriptor != -1) {
//            std::cout << "wiscClient: OpenFile: Reply data received at client = " << reply.data() << std::endl;
//            ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
//            printf("wiscClient:OpenFile: writeResult = %ld\n", writeResult);
//            //SUCCESS
//            printf("wiscClient:OpenFile: Printing fileatts = %ld, %ld, %ld\n", reply.fileinfo().st_size(),reply.fileinfo().st_atim(),reply.fileinfo().st_mtim());
//            CacheFileInfo fileatts;
//            fileatts.setFileInfo(&reply.fileinfo());
//            ClientCacheValue ccv(fileatts, false, fileDescriptor);
//            ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
//            std::cout << "wiscClient:OpenFile: Set reply.inode() = " << reply.fileinfo().st_ino() << "ccv.inode = " << ccv.fileInfo.st_ino << std::endl;
//            diskCache.addCacheValue(filename, ccv);
//            errno = reply.error();
//            return fileDescriptor;
//            } else {
//                std::cout << "wiscClient:OpenFile Exiting OpenFile\n";
//                errno = reply.error();
//                return fileDescriptor;
//             }
//      }
//    else {
//        std::cout << "wiscClient:OpenFile Exiting OpenFile\n";
//        errno = reply.error();
//        return -1;
//    }
}
/*int wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   std::cout << "wiscClient:OpenFile: Entering OpenFile\n";
   RPCRequest request;
   request.set_filename(filename);
   request.set_flags(flags);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   // The actual RPC.
   Status status = stub_->OpenFile(&context, request, &reply);
   // Act upon its status.
   if (status.ok()) {
       std::cout << "wiscClient:OpenFile Reply status " << reply.status() << std::endl;
       std::string local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
       int fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
       if (fileDescriptor < 0) {
           std::cout << "ERROR: Cannot open temp file " << local_path << std::endl;
       }

       if (fileDescriptor != -1) {
           std::cout << "wiscClient: OpenFile: Reply data received at client = " << reply.data() << std::endl;
           ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
           printf("wiscClient:OpenFile: writeResult = %ld\n", writeResult);
           //SUCCESS
           printf("wiscClient:OpenFile: Printing fileatts = %ld, %ld, %ld\n", reply.fileinfo().st_size(),reply.fileinfo().st_atim(),reply.fileinfo().st_mtim());
           CacheFileInfo fileatts;
           fileatts.setFileInfo(&reply.fileinfo());
           ClientCacheValue ccv(fileatts, false, fileDescriptor);
           ccv.fileInfo.st_ino = reply.fileinfo().st_ino();
           std::cout << "wiscClient:OpenFile: Set reply.inode() = " << reply.fileinfo().st_ino() << "ccv.inode = " << ccv.fileInfo.st_ino << std::endl;
           diskCache.addCacheValue(filename, ccv);
           errno = reply.error();
           return fileDescriptor;
           } else {
               std::cout << "wiscClient:OpenFile Exiting OpenFile\n";
               errno = reply.error();
               return fileDescriptor;
            }
     }
   else {
       std::cout << "wiscClient:OpenFile Exiting OpenFile\n";
       errno = reply.error();
       return -1;
   }
}*/

// int wiscAFSClient::CloseFile(const std::string& filename) {
//     // Data we are sending to the server.
//     // DELTE LOCAL FILES POST THIS
//     ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
//     RPCRequest request;
//     if(ccv1 == nullptr){
//         errno=ENOENT;
//     }
//     else if(!ccv1->isDirty){
//         diskCache.deleteCacheValue(filename);
//         return 0;
//     }
//     else{
//         std::cout << "wiscClient:CloseFile found cache entry\n";
//         std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
//         std::cout << "wiscClient:CloseFile: Trying to open local file = " << local_path << std::endl;
//         int fd = open(local_path.c_str(),  O_RDONLY);
//         unsigned long int sz;
//         char* buffer;
//         if (fd == -1){
//             std::cout << "wiscClient:CloseFile: couldn't open temporary file\n";
//         }
//         else {
//             sz = lseek(fd, 0L, SEEK_END);
//             lseek(fd, 0L, SEEK_SET);
//             buffer = new char[sz];
//             buffer[sz] = '\0';
//             if (sz == 0){
//                 std::cout<< "wiscClient:CloseFile: size of temp file is 0\n";
//                 request.set_data("");
//             }
//             else{
//                 int err = read(fd, buffer, sz);
//                 std::string s_buffer(buffer);
//                 std::cout<< "wiscClient:CloseFile: sz = " << sz << ", sending data = " << buffer[0] << ",sz = " << sz << ", bytes read = " << err << std::endl;
//                 request.set_data(buffer);
//             }
//             request.set_filesize(sz);
//         }

//         request.set_filename(filename);
//         request.set_filedescriptor(ccv1->fileDescriptor);

//         // Container for the data we expect from the server.
//         RPCResponse reply;

//         // Context for the client. It could be used to convey extra information to
//         // the server and/or tweak certain RPC behaviors.
//         ClientContext context;

//         // The actual RPC.
//         std::cout<< "wiscClient:CloseFile Calling stub->closefile " <<  std::endl;
//         Status status = stub_->CloseFile(&context, request, &reply);

//         if (status.ok()) {
//             std::cout<< "wiscClient:CloseFile Deleting cacheValue, returning 0" <<  std::endl;
//             diskCache.deleteCacheValue(filename);
//             std::cout<< "wiscClient:CloseFile Deleted cacheValue, returning 0" <<  std::endl;
//             return 0;
//         }
//         else{
//             return -status.error_code();
//         }
//     }
// }


int wiscAFSClient::CloseFile(const std::string& filename) {
    // Data we are sending to the server.
    // DELTE LOCAL FILES POST THIS
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    RPCRequest request;
    if(ccv1 == nullptr){
        errno=ENOENT;
    }
    else if(!ccv1->isDirty){
        diskCache.deleteCacheValue(filename);
        return 0;
    }
    else{
        std::cout << "wiscClient:CloseFile found cache entry\n";
        std::string local_path = (client_path + std::to_string(ccv1->fileInfo.st_ino) + ".tmp").c_str();
        std::cout << "wiscClient:CloseFile: Trying to open local file = " << local_path << std::endl;
        int fd = open(local_path.c_str(),  O_RDONLY);
        if (fd == -1){
            std::cout << "wiscClient:CloseFile: couldn't open temporary file\n";
            return -1;
        }
        unsigned long int sz;
        sz = lseek(fd, 0L, SEEK_END);
        lseek(fd, 0L, SEEK_SET);
        request.set_filename(filename);
        request.set_filedescriptor(ccv1->fileDescriptor);
        RPCResponse reply;
        ClientContext context;
        std::cout<< "wiscClient:CloseFile Calling stub->closefile " <<  std::endl;

        std::unique_ptr<ClientWriter<RPCRequest>> writer(stub_->CloseFile(&context, &reply));
        char *buf = new char[4096];
        while(1){
            int bytesRead;
            if (sz < 1024){
                bytesRead = read(fd, buf, sz);
            }
            else{
                bytesRead = read(fd, buf, 1024);
            }
            if (bytesRead == 0) {
                break;
            }
            if (bytesRead == -1) {
                std::cerr << "client read error while reading op - err:" << errno << std::endl;
                return -1;
            }
            request.set_data(buf);
            request.set_filesize(bytesRead);

            if (!writer->Write(request)) {
                std::cerr << "stream broke:" << errno << std::endl;
                return -1;
            }
        }

        writer->WritesDone();
        Status status = writer->Finish();
        close(fd);
        diskCache.deleteCacheValue(filename);
        unlink(local_path.c_str());
        free(buf);
        // Container for the data we expect from the server.

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.

        // The actual RPC.
      
        return 0;
    }
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

int wiscAFSClient::CreateFile(const std::string& filename, const int flags, const int mode) {
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   std::cout << "wiscClient:CreateFile: Entering CreateFile\n";
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
       std::cout << "wiscClient:CreateFile Reply status " << reply.status() << std::endl;
       std::string local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
       int fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
       if (fileDescriptor < 0) {
           std::cout << "ERROR: Cannot open temp file " << local_path << std::endl;
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
           std::cout << "wiscClient:CreateFile: Set reply.inode() = " << reply.fileinfo().st_ino() << "ccv.inode = " << ccv.fileInfo.st_ino << std::endl;
           diskCache.addCacheValue(filename, ccv);
           errno = reply.error();
           return fileDescriptor;
           } else {
               std::cout << "wiscClient:CreateFile Exiting CreateFile\n";
               errno = reply.error();
               return fileDescriptor;
            }
     }
   else {
       std::cout << "wiscClient:CreateFile Exiting CreateFile\n";
       errno = reply.error();
       return -1;
   }

}

RPCResponse wiscAFSClient::DeleteFile(const std::string& filename) {
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
   if(ccv1 == nullptr){
       std::cout << "Cache entry not found\n";
       errno=ENOENT;
       errno = reply.error();
   }
   else if (status.ok()) {
       diskCache.deleteCacheValue(filename);
   }

   std::cout << "wiscClient: Exiting DeleteFile\n";
   errno = reply.error();
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
       reply.set_status(-status.error_code());
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
    Status status = stub_->OpenDir(&context, request, &reply);

    // Act upon its status.
    if (!status.ok()) {
        std::cout << status.error_code() << ": " << status.error_message()
            << std::endl;
        reply.set_status(-1);
    }
    errno = reply.error();
    std::cout << "wiscClient: Exiting AccessFile\n";
    return reply;
}

int wiscAFSClient::ReadDir(const std::string& p, void *buf, fuse_fill_dir_t filler) {
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
        struct stat st;
        memset(&st, 0, sizeof(st));

        de.d_ino = reply.dino();
        strcpy(de.d_name, reply.dname().c_str());
        de.d_type = reply.dtype();

        st.st_ino = de.d_ino;
        st.st_mode = de.d_type << 12;
        std::cout << de.d_ino << " ANBC " << de.d_type << " bhadbj " << st.st_ino << std::endl;
        if (filler(buf, de.d_name, &st, 0))
            break;
    }
    Status status = reader->Finish();
    std::cout << "wiscClient: Exiting ReadDir\n";
    errno = reply.error();
    return -status.error_code();
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
        std::cout << status.error_code() << ": " << status.error_message()
            << std::endl;
        reply.set_status(status.error_code());
    }

    std::cout << "wiscClient: Exiting RemoveDir\n";
    errno = reply.error();
    return reply;
}

RPCResponse wiscAFSClient::GetAttr(const std::string& filename) {
   // Data we are sending to the server.
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
            reply.set_status(-status.error_code());
        }
    } else {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        struct stat buf;
        lstat(local_path.c_str(), &buf);
        FileInfo* fileInfo = new FileInfo();
        setFileInfo(fileInfo, buf);
        reply.set_allocated_fileinfo(fileInfo);
    }


   std::cout << "wiscClient:GetAttr: Exiting GetAttr\n";
   errno = reply.error();
   return reply;
 
}

RPCResponse wiscAFSClient::GetXAttr(const std::string& filename, const std::string& name, char* value, size_t size) {
   // Data we are sending to the server.
   std::cout << "wiscClient:GetXAttr: Entering GetXAttr\n";
   ClientCacheValue *ccv = diskCache.getCacheValue(filename);
   RPCRequest request;
   request.set_filename(filename);
   request.set_xattr(name);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   if (ccv == nullptr) {
       // The actual RPC.
        Status status = stub_->GetXAttr(&context, request, &reply);
        // Act upon its status.
        if (!status.ok()) {
            reply.set_status(-status.error_code());
        }
    } else {
        std::string local_path = (client_path + std::to_string(ccv->fileInfo.st_ino) + ".tmp").c_str();
        int size = getxattr(local_path.c_str(), name.c_str(), value, size);
        reply.set_xattr(value);
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


#ifdef __cplusplus
}
#endif

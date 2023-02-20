#include "wiscAFS_client.hh"
#include "cache/ClientCache.h"
#include <dirent.h>
#include <fuse.h>
#include <errno.h>

using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using grpc::ClientWriter;



#ifdef __cplusplus
extern "C" {
#endif
int wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   std::cout << "wiscClient: Entering OpenFile\n";
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
       std::cout << "wiscClient: Reply status " << reply.status() << std::endl;
       std::string local_path = (client_path + std::to_string(reply.fileinfo().st_ino()) + ".tmp").c_str();
       int fileDescriptor = open(local_path.c_str(),  flags, 0644);
       if (fileDescriptor < 0) {
           std::cout << "Cannot open temp file " << local_path << std::endl;
       }

       if (fileDescriptor != -1) {
           std::cout << "wiscClient: OPEN: Reply data received at client = " << reply.data() << std::endl;
           ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
           printf("wiscClient: writeResult = %ld\n", writeResult);
           //SUCCESS
           printf("wiscClient: Printing fileatts = %ld, %ld, %ld\n", reply.fileinfo().st_size(),reply.fileinfo().st_atim(),reply.fileinfo().st_mtim());
           CacheFileInfo fileatts;
           fileatts.setFileInfo(&reply.fileinfo());
           ClientCacheValue ccv(fileatts, false, fileDescriptor);
           diskCache.addCacheValue(filename, ccv);
           } else {
               std::cout << "wiscClient: Exiting OpenFile\n";
               errno = reply.error();
               return fileDescriptor;
            }
     }
   else {
       std::cout << "wiscClient: Exiting OpenFile\n";
       errno = reply.error();
       return -1;
   }
}

int wiscAFSClient::CloseFile(const std::string& filename) {
    // Data we are sending to the server.
    std::cout << "wiscClient: Entering CloseFile\n";
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        std::cout << "Cache entry does not exist\n";
        errno=ENOENT;
    } else {
        RPCRequest request;
        request.set_filename(filename);
        request.set_filedescriptor(ccv1->fileDescriptor);
        //request.set_data(data);
        // Container for the data we expect from the server.
        RPCResponse reply;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        ClientContext context;

        // The actual RPC.
        Status status = stub_->CloseFile(&context, request, &reply);

        if (status.ok()) {
            diskCache.deleteCacheValue(filename);
            std::cout << " File Closed\n";
            std::cout << "wiscClient:Exiting CloseFile\n";
            errno = reply.error();
            return 0;
        }
        else{
            std::cout << "wiscClient:Exiting CloseFile\n";
            errno = reply.error();
            return -status.error_code();
        }
    }
}

int wiscAFSClient::ReadFile(const std::string& filename) {
    std::cout << "wiscClient: Entering ReadFile\n";
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        std::cout << "Cache entry not found\n";
        std::cout << "wiscClient: Exiting ReadFile\n";
        errno=ENOENT;
        return -1;
    }
    else{
        std::cout << "wiscClient: Exiting ReadFile\n";
        errno = 0;
        return ccv1->fileDescriptor;
    }
}

//Returning either fD or error
int wiscAFSClient::WriteFile(const std::string& filename){
    std::cout << "wiscClient: Entering WriteFile\n";
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        errno=ENOENT;
        std::cout << "Cache entry not found\n";
        std::cout << "wiscClient: Exiting WriteFile\n";
        return -1;
    } else {
        if(!ccv1->isDirty){
            ccv1->isDirty = true;
            diskCache.updateCacheValue(filename, *ccv1);
        }
        std::cout << "wiscClient: Exiting WriteFile\n";
        errno = 0;
        return ccv1->fileDescriptor;
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
       //errno=ENOENT;
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
    int fd = open("/users/vramadas/test.log", O_CREAT|O_RDWR|O_APPEND, 0777);
   // The actual RPC.
   Status status = stub_->CreateDir(&context, request, &reply);
   write(fd, "CreateDir Passed\n", strlen("CreateDir Passed\n"));
   std::cout << status.ok();
   std::cout<<"This is the start" << std::endl;
   // Act upon its status.
   if (status.ok()) {
       std::cout << " I am open" << reply.data(); 
   } else {

       std::cout << status.error_code() << ": " << status.error_message()
           << std::endl;
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
    return reply;
    std::cout << "wiscClient: Exiting OpenDir\n";
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
   std::cout << "wiscClient: Entering GetAttr\n";
   RPCRequest request;
   request.set_filename(filename);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;

   // The actual RPC.
   Status status = stub_->GetAttr(&context, request, &reply);

   // Act upon its status.
   if (!status.ok()) {
       reply.set_status(-status.error_code());
   }

   std::cout << "wiscClient: Exiting GetAttr\n";
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

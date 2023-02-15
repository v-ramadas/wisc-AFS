#include "wiscAFS_client.hh"
#include "cache/ClientCache.h"

int wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
   ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
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
           std::string local_path = (client_path + std::to_string(reply.inode()) + ".tmp").c_str();
           int fileDescriptor = open(local_path.c_str(),  O_WRONLY | O_CREAT | O_EXCL, 0644);
           if (fileDescriptor != -1) {
               ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
               //SUCCESS
               FileAttrs fileatts(reply.rpcattr().filesize(),reply.rpcattr().atime(),reply.rpcattr().mtime());
               ClientCacheValue ccv(fileatts, reply.inode(), false, fileDescriptor);
               diskCache.addCacheValue(filename, ccv);
           }
           return fileDescriptor;
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

int wiscAFSClient::CloseFile(const std::string& filename) {
    // Data we are sending to the server.
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        errno=ENOENT;
    }
    else if(!ccv1->isDirty){
        diskCache.deleteCacheValue(filename);
        return 0;
    }
    else{
        RPCRequest request;
        request.set_filename(filename);
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
            return 0;
        }
        else{
            return -status.error_code();
        }
    }
}

int wiscAFSClient::ReadFile(const std::string& filename) {
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        errno=ENOENT;
    }
    else{
        return ccv1->fileDiscriptor;
    }
}

//Returning either fD or error
int wiscAFSClient::WriteFile(const std::string& filename){
    ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
    if(ccv1 == nullptr){
        errno=ENOENT;
    }
    else{
        if(!ccv1->isDirty){
            ccv1->isDirty = true;
            diskCache.updateCacheValue(filename, *ccv1);
        }
        return ccv1->fileDiscriptor;
    }
}

RPCResponse wiscAFSClient::DeleteFile(const std::string& filename, const std::string& path) {
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
       errno=ENOENT;
   }
   else if (status.ok()) {
       diskCache.deleteCacheValue(filename);
   }
   return reply;

}

RPCResponse wiscAFSClient::CreateDir(const std::string& dirname, const std::string& path, const int mode) {
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

RPCResponse wiscAFSClient::RemoveDir (const std::string& dirname, const std::string& path) {
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

RPCResponse wiscAFSClient::GetAttr(const std::string& filename) {
   // Data we are sending to the server.
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
   if (status.ok()) {
       return reply;
   } else {
       reply.set_status(-status.error_code());
       return reply;
   }
 
}

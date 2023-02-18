#include "wiscAFS_client.hh"
#include "cache/ClientCache.h"

int wiscAFSClient::OpenFile(const std::string& filename, const int flags) {
   //ClientCacheValue *ccv1 = diskCache.getCacheValue(filename);
   //if(ccv1 == nullptr){
   // Data we are sending to the server. ##ASSUMING FILENAMES include path
      
   RPCRequest request;
   request.set_filename(filename);
   request.set_flags(flags);

   // Container for the data we expect from the server.
   RPCResponse reply;

   // Context for the client. It could be used to convey extra information to
   // the server and/or tweak certain RPC behaviors.
   ClientContext context;
   int flog = open("/users/vramadas/client.log", O_CREAT|O_RDWR|O_TRUNC, 0777);

   // The actual RPC.
   write(flog, "WiscAFSClint:: Calling OpenFile Stub\n", strlen("WiscAFSClint:: Calling OpenFile Stub\n"));
   Status status = stub_->OpenFile(&context, request, &reply);
   // Act upon its status.
   if (status.ok()) {
       write(flog, "Received status ok\n", strlen("Received status ok\n"));
       std::cout << "Client: Reply status " << reply.status() << std::endl;
       std::string local_path = (client_path + std::to_string(reply.inode()) + ".tmp").c_str();
       int fileDescriptor = open(local_path.c_str(),  O_CREAT|O_RDWR|O_TRUNC, 0777);
       if (fileDescriptor < 0) {
           std::cout << "Cannot open temp file " << local_path << std::endl;
       }
       if (fileDescriptor != -1) {
           std::cout << "Client: OPEN: Reply data received at client = " << reply.data() << std::endl;
           ssize_t writeResult = write(fileDescriptor, reply.data().c_str(), reply.data().size());
           printf("Client: writeResult = %ld\n", writeResult);
           //SUCCESS
           printf("Client: Printing fileatts = %d, %ld, %ld\n", reply.rpcattr().filesize(),reply.rpcattr().atime(),reply.rpcattr().mtime());
           FileAttrs fileatts(reply.rpcattr().filesize(),reply.rpcattr().atime(),reply.rpcattr().mtime());
           ClientCacheValue ccv(fileatts, reply.inode(), false, fileDescriptor);
           diskCache.addCacheValue(filename, ccv);
       }
       return fileDescriptor;
   } 
   else {
       write(flog, "Received status not ok\n", strlen("Received status ok\n"));
       return -status.error_code();
   }
   //}
   /*else{
       //ALREADY IN CACHE
       int fd = ccv1->fileDiscriptor;
       return fd;
   }*/
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

RPCResponse wiscAFSClient::DeleteFile(const std::string& filename) {
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

RPCResponse wiscAFSClient::CreateDir(const std::string& dirname, const int mode) {
   // Data we are sending to the server.
   RPCRequest request;
   request.set_filename(dirname);
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

RPCResponse wiscAFSClient::OpenDir(const std::string& dirname, const int mode) {
    RPCRequest request;
    request.set_filename(dirname);
    request.set_path(dirname);

    // Container for the data we expect from the server.
    RPCResponse reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->OpenDir(&context, request, &reply);

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

RPCResponse wiscAFSClient::RemoveDir (const std::string& dirname) {
    // Data we are sending to the server.
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

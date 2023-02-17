#pragma once

#include "wiscAFS_client.hh"

#ifdef __cplusplus
extern "C" {
#endif
//extern wiscAFSClient* afsClient;
static wiscAFSClient* afsClient;
//static wiscAFSClient afsClient2(grpc::CreateChannel(std::string("10.10.1.2:50051"), grpc::InsecureChannelCredentials()));
#ifdef __cplusplus
}
#endif

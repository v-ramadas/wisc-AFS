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


#include "wiscAFS_client.hh"
int main(int argc, char** argv) {
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).

    std::string target_str;
    std::string arg_str("--target");
    if (argc > 1) {
        std::string arg_val = argv[1];
        size_t start_pos = arg_val.find(arg_str);
        if (start_pos != std::string::npos) {
            start_pos += arg_str.size();
            if (arg_val[start_pos] == '=') {
                target_str = arg_val.substr(start_pos + 1);
            } else {
                std::cout << "The only correct argument syntax is --target="
                    << std::endl;
                return 0;
            }
        } else {
            std::cout << "The only acceptable argument is --target=" << std::endl;
            return 0;
        }
    } else {
        target_str = "10.10.1.2:50051";
    }
    wiscAFSClient afsClient (
            grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string filename("a.txt");
    std::string dirname("dir1");
    std::string path1("/users/vramadas/test.txt");
    std::string path2("/users/vramadas/delete.txt");
    std::string data("asdafgdf\nssfdlkjgdgklfg\n");
    int flags = O_RDONLY | O_APPEND;
    int mode = S_IRWXU;
    int dirmode = 700;
    std::cout << "Sending OpenFile\n" ;
    int reply = afsClient.OpenFile(path1, flags);
    std::cout << "Reply = " << reply << std::endl;

    std::cout << "Sending CreateFile\n" ;
      reply = afsClient.OpenFile(path2, O_CREAT);
      std::cout << "Response recieved : " << reply << std::endl;

      std::cout << "Sending GetAttr\n" ;
      RPCResponse response  = afsClient.GetAttr(path1);

    std::cout << " FileAttr : " << response.fileinfo().st_size() <<  " : " << response.fileinfo().st_ino() << " : " << response.fileinfo().st_atim() << "\n";
      std::cout << "Sending CloseFile\n" ;
      reply = afsClient.CloseFile(path1);
      std::cout << "Response recieved : " << reply << std::endl;

      std::cout << "Sending DeleteFile\n" ;
      afsClient.DeleteFile(path2);

      /*std::cout << "Sending CreateDir\n" ;
      reply = afsClient.CreateDir(dirname, path, dirmode);
      std::cout << "Response recieved : " << reply.status() << std::endl;

      std::cout << "Sending OpenDir\n" ;
      reply = afsClient.OpenDir(dirname, path, mode);
      std::cout << "Response recieved : " << reply.status() << std::endl;

      std::cout << "Sending RemoveDir\n" ;
      reply = afsClient.RemoveDir(dirname, path);
      std::cout << "Response recieved : " << reply.status() << std::endl;*/

    return 0;
}

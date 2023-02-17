# AFS
A basic AFS-like distributed file system

Steps to build wiscAFS GRPC Layer
1. export MY_INSTALL_DIR-~/.local
2. cd <repo>/wiscAFS/
3. mkdir -p cmake/build
4. cd cmake/build
5. cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
6. make
4. Binary is in <repo>/wiscAFS/cmake/build


Steps to build unreliablefs
1. cd <repo>/unreliablefs
2. cmake -S . -B build -DCMAKE_BUILD_TYPE=Debu
3. cmake --build build --parallel
4. Binary is in <repo>/unreliablefs/build/unreliablefs/

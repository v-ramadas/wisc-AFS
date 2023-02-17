# AFS
A basic AFS-like distributed file system

Steps to build wiscAFS GRPC Layer
1. export MY_INSTALL_DIR=~/.local
2. cd <repo>/wiscAFS/
3. mkdir -p cmake/build
4. cd cmake/build
5. cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ../..
6. make
4. Binary is in <repo>/wiscAFS/cmake/build


Steps to build unreliablefs
1. cd <repo>/unreliablefs
2. cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
3. cmake --build build --parallel
4. Binary is in <repo>/unreliablefs/build/unreliablefs/


How to run unreliablefs
1. cd <repo>/unreliable
2. build/unreliablefs/unreliablefs /tmp/fs -basedir=/tmp -seed=1618680646
3. To unmount, sudo umount /tmp/fs
4. In case you crash the session, open another session in the same node and:
5. ps -aux | grep vramadas
6. kill -9 <pid> where pid is the process Id for unreliablefs and/or crashing application
7. unmount

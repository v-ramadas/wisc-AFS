cmake_minimum_required(VERSION 3.10.2)

include(CheckFunctionExists)

set(UNRELIABLEFS_SRC conf.c
                     unreliablefs.c
                     unreliablefs_errinj.c
                     unreliablefs_ops.c
                     wiscAFS_ops.cc)

set(HOME_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../)
find_package(FUSE 2.9 REQUIRED)

include(../../wiscAFS/common.cmake)
find_package(gRPC CONFIG REQUIRED)
find_library(WISCAFS_LIB wiscAFS_grpc_proto HINTS ${HOME_PATH}/wiscAFS/cmake/build)
#find_library(_GRPC_GRPCPP grpc++ HINTS ~/.local/lib)
#find_library(_PROTOBUF_LIBPROTOBUF protobuf HINTS ~/.local/lib)

add_definitions(${FUSE_DEFINITIONS})
add_executable(${PROJECT_NAME} ${UNRELIABLEFS_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${FUSE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES} ${WISCAFS_LIB} gRPC::grpc++)
target_include_directories(${PROJECT_NAME} PRIVATE ../../wiscAFS)
target_include_directories(${PROJECT_NAME} PRIVATE ../../wiscAFS/cmake/build)
#target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH})
#target_include_directories(${PROJECT_NAME} PRIVATE ~/.local/include)
#target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/wiscAFS)
#target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/wiscAFS/cmake/build)
#target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES} ${WISCAFS_LIB} ${_REFLECTION} ${_PROTOBUF_LIBPROTOBUF} ${_GRPC_GRPCPP})
#target_link_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/wiscAFC/cmake/build)

target_compile_options(${PROJECT_NAME} PUBLIC -D_FILE_OFFSET_BITS=64 -Wall -Wextra -Wno-unused-parameter)
target_compile_options(${PROJECT_NAME} PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++14 -D_FILE_OFFSET_BITS=64>)

check_function_exists(fallocate HAVE_FALLOCATE)
check_function_exists(fallocate HAVE_FLOCK)
check_function_exists(utimensat HAVE_UTIMENSAT)
check_function_exists(setxattr HAVE_XATTR)
if (${HAVE_FALLOCATE})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_FALLOCATE)
endif ()
if (${HAVE_FLOCK})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_FLOCK)
endif ()
if (${HAVE_UTIMENSAT})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_UTIMENSAT)
endif ()
if (${HAVE_XATTR})
    target_compile_definitions(${PROJECT_NAME} PUBLIC HAVE_XATTR)
endif ()

add_subdirectory(tests)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(FILES "unreliablefs.1" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
install(FILES "unreliablefs.conf.5" DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man5)

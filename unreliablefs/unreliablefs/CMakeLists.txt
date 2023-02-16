cmake_minimum_required(VERSION 3.10.2)

include(CheckFunctionExists)

set(UNRELIABLEFS_SRC conf.c
                     unreliablefs.c
                     unreliablefs_errinj.c
                     unreliablefs_ops.c
                     wiscAFS_ops.cc)

find_package(FUSE 2.9 REQUIRED)

set(HOME_PATH /users/vramadas)

add_definitions(${FUSE_DEFINITIONS})
add_executable(${PROJECT_NAME} ${UNRELIABLEFS_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE ${FUSE_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH})
target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/grpc/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/grpc/bin/include)
target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/grpc/examples/cpp/wiscAFC)
target_include_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/grpc/examples/cpp/wiscAFC/cmake/build)
target_link_libraries(${PROJECT_NAME} ${FUSE_LIBRARIES} wiscAFS_grpc_proto)
target_link_directories(${PROJECT_NAME} PRIVATE ${HOME_PATH}/grpc/examples/cpp/wiscAFC/cmake/build)

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

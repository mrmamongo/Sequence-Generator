cmake_minimum_required(VERSION 3.16)
project(seq_generator)

find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 17)

add_library(utils INTERFACE)

target_include_directories(utils INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        )

add_library(seq_storage SHARED
        source/seq_storage/seq_storage.cxx
        source/seq_storage/sequence.cxx
        )

target_link_libraries(seq_storage PUBLIC utils)
target_include_directories(seq_storage
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/source"
        )

add_library(server
        source/server/server.cxx
        source/server/client.cxx
        )

target_link_libraries(server PUBLIC seq_storage)
target_include_directories(server
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/source"
        )

add_executable(${PROJECT_NAME}
        main.cpp
        )

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads server)

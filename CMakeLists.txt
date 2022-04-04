cmake_minimum_required(VERSION 3.0)

project(translator)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Werror=return-type")

include_directories(include)

add_executable(translator
        main.cpp
        src/SCGI_Connection.cpp
        src/Websocket_Connection.cpp
        )

target_link_libraries(translator
        uSockets
        z
        ssl
        crypto
        pthread
        )

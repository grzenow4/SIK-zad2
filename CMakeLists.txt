cmake_minimum_required(VERSION 3.22)
project(SIK-zad2)

set(CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Werror -O2 -pthread")

SET(Boost_USE_STATIC_LIBS FALSE)
FIND_PACKAGE(Boost REQUIRED COMPONENTS system program_options)
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF (Boost_FOUND)

SET(USED_LIBS ${Boost_SYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(robots-client robots-client.cpp Client.cpp)
add_executable(robots-server robots-server.cpp Server.cpp GameRoom.cpp GameSession.cpp)

TARGET_LINK_LIBRARIES(robots-client ${USED_LIBS})
TARGET_LINK_LIBRARIES(robots-server ${USED_LIBS})
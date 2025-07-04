cmake_minimum_required(VERSION 3.10)
project(chat_client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(client
    src/main.cpp
    src/validation.cpp
    src/connection.cpp
    src/commands.cpp
)

target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic -Werror)

target_include_directories(client PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

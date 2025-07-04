cmake_minimum_required(VERSION 3.10)
project(chat_client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(client
    main.cpp
)

target_compile_options(client PRIVATE -Wall -Wextra -Wpedantic -Werror)

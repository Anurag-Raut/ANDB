cmake_minimum_required(VERSION 3.10)

# Define the main project
project(andb-projects)

# Define the andb-daemon executable
add_executable(andb-daemon daemon/main.cpp)

# Define the andb-cli executable
add_executable(andb-cli cli/main.cpp)
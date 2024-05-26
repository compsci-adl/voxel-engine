cmake_minimum_required(VERSION 3.15)
project(voxel-engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)

# Define the installation directory
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

# Add your libraries and include directories
add_subdirectory(include/raylib)

include_directories(
    include
    include/raylib/src
    src
)

file(GLOB_RECURSE SRCS src/*.cpp)

set(PROJECT_NAME_LIB ${PROJECT_NAME}_lib)
add_library(${PROJECT_NAME_LIB} STATIC ${SRCS})

add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME_LIB} PRIVATE
    include
    include/raylib/src
)

target_link_libraries(${PROJECT_NAME}
    ${PROJECT_NAME_LIB}
    raylib
    # -static-libstdc++  # Statically link libstdc++
)

# Install the executable and libraries
install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME_LIB}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
)

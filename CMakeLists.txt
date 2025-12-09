
cmake_minimum_required(VERSION 3.15)
project(IVC C)

# Find SDL3 package
find_package(SDL3 REQUIRED)

# Add executable
add_executable(ivc main.c)

# Include your local headers (stb_image.h)
target_include_directories(ivc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link SDL3 and the math library
target_link_libraries(ivc PRIVATE SDL3::SDL3 m)

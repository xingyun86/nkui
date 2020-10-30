# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("glui")

# Include sub-projects.
add_subdirectory ("glfw-3.3.2")
add_subdirectory ("glui")

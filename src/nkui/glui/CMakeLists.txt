# CMakeList.txt : CMake project for glui, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(project_name "glui")
project(${project_name})

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/vendor
	${CMAKE_SOURCE_DIR}/glfw-3.3.2/include
	${CMAKE_BINARY_DIR}/glfw-3.3.2/src
	${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps
)
link_directories(
	${CMAKE_BINARY_DIR}/glfw-3.3.2/src
)
if (MSVC)
set(gl_ext_libs glfw3)
ELSE(MSVC)
set(gl_ext_libs glfw3 dl pthread X11)
ENDIF(MSVC)
if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

set(GLFW_DEPS_DIR ${CMAKE_SOURCE_DIR}/glfw-3.3.2/deps)
set(GLAD_GL "${GLFW_DEPS_DIR}/glad/gl.h"
            "${GLFW_DEPS_DIR}/glad_gl.c")
set(GETOPT "${GLFW_DEPS_DIR}/getopt.h"
           "${GLFW_DEPS_DIR}/getopt.c")
set(TINYCTHREAD "${GLFW_DEPS_DIR}/tinycthread.h"
                "${GLFW_DEPS_DIR}/tinycthread.c")

if (${CMAKE_VERSION} VERSION_EQUAL "3.1.0" OR
    ${CMAKE_VERSION} VERSION_GREATER "3.1.0")
    set(CMAKE_C_STANDARD 99)
else()
    # Remove this fallback when removing support for CMake version less than 3.1
    add_compile_options("$<$<C_COMPILER_ID:AppleClang>:-std=c99>"
                        "$<$<C_COMPILER_ID:Clang>:-std=c99>"
                        "$<$<C_COMPILER_ID:GNU>:-std=c99>")
endif()

file(GLOB SRCS "*.cpp")
# Add source to this project's executable.
add_executable (${project_name} ${SRCS} ${GETOPT} ${GLAD_GL})

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif(MSVC)
target_compile_definitions(${project_name} PRIVATE -D_CRT_SECURE_NO_WARNINGS)
target_link_libraries(${project_name} ${gl_ext_libs})

# TODO: Add tests and install targets if needed.

cmake_minimum_required(VERSION 3.29)

if (DEFINED ENV{USE_VCPKG_TOOLCHAIN} AND "$ENV{USE_VCPKG_TOOLCHAIN}" STREQUAL "1")
    set(CMAKE_TOOLCHAIN_FILE "E:/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

project(async VERSION 1.0.0)

#find_package(Boost REQUIRED COMPONENTS program_options)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(async 
main.cpp
)

set_target_properties(async PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(async ${Boost_LIBRARIES})

if (MSVC)
    target_compile_options(async PRIVATE /W4)
else ()
    target_compile_options(async PRIVATE -Wall -Wextra -pedantic) 
endif()

#install(TARGETS async RUNTIME DESTINATION bin)
#set(CPACK_GENERATOR DEB)
#set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
#set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
#set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
#set(CPACK_PACKAGE_CONTACT example@example.com)
#include(CPack)





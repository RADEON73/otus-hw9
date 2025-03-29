cmake_minimum_required(VERSION 3.29)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

project(async VERSION 1.0.0)

add_executable(main 
main.cpp
)

add_library(async SHARED
async.cpp async.h
BulkProcessor.cpp BulkProcessor.h
MultiThreadOutputter.cpp MultiThreadOutputter.h
Commands.h
CommandFactory.h
ThreadSafeQueue.h
)

set_target_properties(main async PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(async PRIVATE 
	"${CMAKE_BINARY_DIR}"
)

target_link_libraries(main PRIVATE
    async
)

if (MSVC)
    target_compile_options(main PRIVATE /W4)
	target_compile_options(async PRIVATE /W4)
else ()
    target_compile_options(main PRIVATE -Wall -Wextra -pedantic)
    target_compile_options(async PRIVATE -Wall -Wextra -pedantic) 
endif()

install(TARGETS async
	LIBRARY DESTINATION lib
    PUBLIC_HEADER DESTINATION async
)

install(TARGETS async RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
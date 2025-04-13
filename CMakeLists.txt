cmake_minimum_required(VERSION 3.29)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

project(async VERSION 1.1.0)

add_executable(main 
main.cpp
ProcessorCommands.h
ProcessorManager.h
)

add_library(async SHARED
async.cpp async.h
BulkProcessor.cpp BulkProcessor.h
MultiThreadOutputter.cpp MultiThreadOutputter.h
BulkCommands.h
BulkCommandFactory.h
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

if (DEFINED ENV{USE_VCPKG_TOOLCHAIN}) # Небольшая автоматизация для себя в итоговый код не попадает
add_custom_command(
    TARGET async POST_BUILD  
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/test_data.txt
        ${CMAKE_BINARY_DIR}/test_data.txt
    COMMENT "Копирование test_data.txt в каталог программы"
)
endif()

install(TARGETS async RUNTIME DESTINATION bin)
set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_CONTACT example@example.com)
include(CPack)
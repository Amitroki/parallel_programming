cmake_minimum_required(VERSION 3.11)

project(matrix_multiply CXX)

set(CMAKE_CXX_STANDARD 23)

find_package(MPI REQUIRED)

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_compile_options(/W4 /O2 /fp:fast)
endif()


include_directories(include)


add_executable(multiplier "multiplier.cpp" "korolev.cpp")

target_link_libraries(multiplier PRIVATE 
    MPI::MPI_CXX
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
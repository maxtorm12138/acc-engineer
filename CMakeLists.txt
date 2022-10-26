cmake_minimum_required(VERSION 3.14)

project(acc-engineer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Protobuf_USE_STATIC_LIBS ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${PROJECT_SOURCE_DIR}/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Boost 1.80.0 COMPONENTS program_options REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts OpenGL)

function(generate_proto PROTO_ROOT PROTO_FILE GENERATE_PATH OUT_FILES)
    get_filename_component(PROTO_FILE ${PROTO_FILE} ABSOLUTE)
    get_filename_component(PROTO_ROOT ${PROTO_ROOT} ABSOLUTE)
    get_filename_component(PROTO_FILE_NAME ${PROTO_FILE} NAME)

    string(REPLACE ".proto" ".pb.cc" PROTO_CPP_SRC "${GENERATE_PATH}/${PROTO_FILE_NAME}")
    string(REPLACE ".proto" ".pb.h" PROTO_CPP_HDR "${GENERATE_PATH}/${PROTO_FILE_NAME}")

    file(TO_NATIVE_PATH ${PROTO_FILE} NATIVE_PROTO_FILE)
    file(TO_NATIVE_PATH ${GENERATE_PATH} NATIVE_GENERATE_PATH)
    file(TO_NATIVE_PATH ${PROTO_ROOT} NATIVE_PROTO_ROOT)

    file(MAKE_DIRECTORY ${NATIVE_GENERATE_PATH})

    execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=${NATIVE_PROTO_ROOT} --cpp_out=${NATIVE_GENERATE_PATH} ${NATIVE_PROTO_FILE} RESULT_VARIABLE RV)

    if (${RV})
        message(FATAL_ERROR "Generation of data model returned ${RV} for proto ${NATIVE_PROTO_FILE}")
    endif ()
    set(${OUT_FILES} ${PROTO_CPP_SRC} ${PROTO_CPP_HDR} PARENT_SCOPE)
endfunction()

 set(ACC_ENGINEER_SHARED_MEMORY_SRCS
    src/shared_memory/defines.h
    src/shared_memory/shared_memory.h
    src/shared_memory/shared_memory.cpp)

add_library(acc-engineer-shared-memory ${ACC_ENGINEER_SHARED_MEMORY_SRCS})
target_link_libraries(acc-engineer-shared-memory Boost::boost)
target_include_directories(acc-engineer-shared-memory PRIVATE src/)
target_link_libraries(acc-engineer-shared-memory spdlog::spdlog)

set(ACC_ENGINEER_UI_SRCS
    src/ui/driver_input.cpp
    src/ui/driver_input.h
    src/ui/shared_memory_emitter.cpp
    src/ui/shared_memory_emitter.h
    src/ui/main_window.cpp
    src/ui/main_window.h)


set(ACC_ENGINEER_SRCS
    ${ACC_ENGINEER_UI_SRCS}
    )


message("${CMAKE_BUILD_TYPE}")
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_executable(acc-engineer src/main.cpp ${ACC_ENGINEER_SRCS})
    target_compile_definitions(acc-engineer PRIVATE SPDLOG_ACTIVE_LEVEL=0)
    target_link_directories(acc-engineer PRIVATE "E:/qwt-6.2.0/lib")
    target_link_libraries(acc-engineer qwtd.dll)
else()
    add_executable(acc-engineer WIN32 src/main.cpp ${ACC_ENGINEER_SRCS})
    target_link_directories(acc-engineer PRIVATE "E:/qwt-6.2.0/lib")
    target_link_libraries(acc-engineer qwt.dll)
endif ()

target_link_libraries(acc-engineer acc-engineer-shared-memory)
target_link_libraries(acc-engineer Boost::boost)
target_link_libraries(acc-engineer protobuf::libprotobuf)
target_link_libraries(acc-engineer spdlog::spdlog)
target_link_libraries(acc-engineer Qt6::Core Qt6::Widgets Qt6::Charts)
target_include_directories(acc-engineer PRIVATE src/)
target_include_directories(acc-engineer PRIVATE "E:/Qwt-6.2.0/include")
target_compile_definitions(acc-engineer PRIVATE BOOST_ASIO_NO_DEPRECATED)


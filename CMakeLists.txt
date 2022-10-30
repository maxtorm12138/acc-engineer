cmake_minimum_required(VERSION 3.14)

project(acc-engineer LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Protobuf_USE_STATIC_LIBS ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${PROJECT_SOURCE_DIR}/client/ui ${PROJECT_SOURCE_DIR}/server/ui)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts OpenGL REQUIRED)
find_package(Boost 1.80.0 COMPONENTS program_options REQUIRED)
find_package(Protobuf REQUIRED)
find_package(spdlog REQUIRED)

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

generate_proto(proto/ proto/rpc.proto ${CMAKE_CURRENT_BINARY_DIR}/proto ACC_ENGINEER_RPC_PROTO_FILES)

set(ACC_ENGINEER_LOG_LEVEL CACHE STRING 3)

set(ACC_ENGINEER_RPC_SRCS
        src/rpc/rpc/detail/stub.h
        src/rpc/rpc/detail/error_code.h
        src/rpc/rpc/detail/packet_handler.h
        src/rpc/rpc/detail/method.h
        src/rpc/rpc/detail/error_code.h
        src/rpc/rpc/detail/batch_task.cpp
        src/rpc/rpc/detail/batch_task.h
        src/rpc/rpc/batch_task.h
        src/rpc/rpc/error_code.h
        src/rpc/rpc/method.h
        src/rpc/rpc/stub.h
        ${ACC_ENGINEER_RPC_PROTO_FILES})

add_library(acc-engineer-rpc ${ACC_ENGINEER_RPC_SRCS})
target_link_libraries(acc-engineer-rpc PRIVATE Boost::boost)
target_link_libraries(acc-engineer-rpc PRIVATE protobuf::libprotobuf)
target_link_libraries(acc-engineer-rpc PRIVATE spdlog::spdlog)

target_include_directories(acc-engineer-rpc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(acc-engineer-rpc PRIVATE BOOST_ASIO_NO_DEPRECATED)

target_include_directories(acc-engineer-rpc PUBLIC src/rpc)
target_compile_definitions(acc-engineer-rpc PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL})

if (WIN32)
    target_compile_definitions(acc-engineer-rpc PRIVATE _WIN32_WINNT=0x0A00)
endif ()


set(ACC_ENGINEER_SHARED_MEMORY_SRCS
   src/shared_memory/shared_memory/defines.h
   src/shared_memory/shared_memory/reader.h
   src/shared_memory/shared_memory/tools.h
   src/shared_memory/shared_memory/car_spec.h
   src/shared_memory/shared_memory/emitter.h
   src/shared_memory/shared_memory/reader.cpp)

add_library(acc-engineer-shared-memory ${ACC_ENGINEER_SHARED_MEMORY_SRCS})
target_link_libraries(acc-engineer-shared-memory PRIVATE Boost::boost)
target_link_libraries(acc-engineer-shared-memory PRIVATE spdlog::spdlog)
target_link_libraries(acc-engineer-shared-memory PRIVATE Qt6::Core)
target_include_directories(acc-engineer-shared-memory PUBLIC src/shared_memory)
target_compile_definitions(acc-engineer-shared-memory PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL})

if (ACC_ENGINEER_BUILD_CLIENT)
    set(ACC_ENGINEER_CLIENT_SRCS
        client/src/ui/connect.cpp
        client/src/ui/connect.h
        client/src/ui/driver_input.cpp
        client/src/ui/driver_input.h
        client/src/ui/main_window.cpp
        client/src/ui/main_window.h
        client/src/logic.cpp
        client/src/logic.h
        client/src/app.cpp
        client/src/app.h
    )

    add_executable(acc-engineer client/src/main.cpp ${ACC_ENGINEER_CLIENT_SRCS})
    target_compile_definitions(acc-engineer PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL} BOOST_ASIO_NO_DEPRECATED)
    target_link_directories(acc-engineer PRIVATE ${QWT_LIBRARY_DIR})
    target_include_directories(acc-engineer PRIVATE ${CMAKE_BINARY_DIR} client/src)

    target_link_libraries(acc-engineer PUBLIC acc-engineer-shared-memory acc-engineer-rpc)
    target_link_libraries(acc-engineer PUBLIC Boost::boost)
    target_link_libraries(acc-engineer PUBLIC protobuf::libprotobuf)
    target_link_libraries(acc-engineer PUBLIC spdlog::spdlog)
    target_link_libraries(acc-engineer PUBLIC Qt6::Core Qt6::Widgets Qt6::OpenGL)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_link_libraries(acc-engineer PUBLIC qwtd)
    else()
        target_link_libraries(acc-engineer PUBLIC qwt)
    endif()

    if (WIN32)
        target_compile_definitions(acc-engineer PRIVATE _WIN32_WINNT=0x0A00)
    endif ()
endif()
# 
# 
# target_link_libraries(acc-engineer acc-engineer-shared-memory)
# target_link_libraries(acc-engineer Boost::boost)
# target_link_libraries(acc-engineer protobuf::libprotobuf)
# target_link_libraries(acc-engineer spdlog::spdlog)
# target_link_libraries(acc-engineer Qt6::Core Qt6::Widgets Qt6::Charts)
# target_include_directories(acc-engineer PRIVATE src/)
# target_include_directories(acc-engineer PRIVATE "E:/Qwt-6.2.0/include")
# target_compile_definitions(acc-engineer PRIVATE BOOST_ASIO_NO_DEPRECATED)
# 
# add_subdirectory(server)

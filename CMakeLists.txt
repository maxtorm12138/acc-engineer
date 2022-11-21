cmake_minimum_required(VERSION 3.14)
project(acc-engineer LANGUAGES CXX)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${PROJECT_SOURCE_DIR}/client/ui ${PROJECT_SOURCE_DIR}/server/ui)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGL REQUIRED)

FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG        v1.11.0
)

FetchContent_Declare(
    msft_proxy
    GIT_REPOSITORY https://github.com/microsoft/proxy.git
    GIT_TAG        1.1.1
)

FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG        boost-1.81.0.beta1
)

FetchContent_Declare(
    mrpc
    GIT_REPOSITORY https://github.com/maxtorm12138/mrpc.git
    GIT_TAG develop
)

FetchContent_MakeAvailable(spdlog msft_proxy Boost mrpc)

message("${MRPC_PROTO_IMPORT_PATH}")

set(Protobuf_USE_STATIC_LIBS ON)
find_package(Protobuf REQUIRED)

function(generate_proto)
    cmake_parse_arguments(GENERATE_PROTO "" "GENERATE_PATH;FILE;OUTPUT_FILE" "IMPORT_PATH" ${ARGN})

    get_filename_component(GENERATE_PROTO_GENERATE_PATH ${GENERATE_PROTO_GENERATE_PATH} ABSOLUTE)
    file(TO_NATIVE_PATH ${GENERATE_PROTO_GENERATE_PATH} GENERATE_PROTO_GENERATE_PATH)

    get_filename_component(GENERATE_PROTO_FILE ${GENERATE_PROTO_FILE} ABSOLUTE)
    file(TO_NATIVE_PATH ${GENERATE_PROTO_FILE} GENERATE_PROTO_FILE)

    get_filename_component(GENERATE_PROTO_FILE_NAME ${GENERATE_PROTO_FILE} NAME)
    string(REPLACE ".proto" ".pb.cc" GENERATE_PROTO_CPP_SRC "${GENERATE_PROTO_GENERATE_PATH}/${GENERATE_PROTO_FILE_NAME}")
    string(REPLACE ".proto" ".pb.h" GENERATE_PROTO_CPP_HDR "${GENERATE_PROTO_GENERATE_PATH}/${GENERATE_PROTO_FILE_NAME}")

    file(MAKE_DIRECTORY ${GENERATE_PROTO_GENERATE_PATH})
    

    foreach(IMPORT_PATH ${GENERATE_PROTO_IMPORT_PATH})
        get_filename_component(IMPORT_PATH ${IMPORT_PATH} ABSOLUTE)
        file(TO_NATIVE_PATH ${IMPORT_PATH} IMPORT_PATH)
        string(CONCAT IMPORT_ARG "--proto_path=" "${IMPORT_PATH}")
        list(APPEND PROTOC_ARGS ${IMPORT_ARG})
    endforeach()
    list(APPEND PROTOC_ARGS "--cpp_out=${GENERATE_PROTO_GENERATE_PATH}" "${GENERATE_PROTO_FILE}")

    message("PROTOC_ARGS: ${PROTOC_ARGS}")

    execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTOC_ARGS} RESULT_VARIABLE RV)

    if (${RV})
        message(FATAL_ERROR "Generation of data model returned ${RV} for proto ${NATIVE_PROTO_FILE}")
    endif ()
    set(${GENERATE_PROTO_OUTPUT_FILE} ${GENERATE_PROTO_CPP_SRC} ${GENERATE_PROTO_CPP_HDR} PARENT_SCOPE)
    
endfunction()

generate_proto(
    FILE proto/struct.proto
    GENERATE_PATH ${CMAKE_CURRENT_BINARY_DIR}/proto
    OUTPUT_FILE ACC_ENGINEER_STRUCT_PROTO_FILES
    IMPORT_PATH proto/ ${MRPC_PROTO_IMPORT_PATH}
)

generate_proto(
    FILE proto/service.proto
    GENERATE_PATH ${CMAKE_CURRENT_BINARY_DIR}/proto
    OUTPUT_FILE ACC_ENGINEER_SERVICE_PROTO_FILES
    IMPORT_PATH proto/ ${MRPC_PROTO_IMPORT_PATH}
)

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
target_link_libraries(acc-engineer-rpc PRIVATE Boost::asio Boost::scope_exit)
target_link_libraries(acc-engineer-rpc PRIVATE protobuf::libprotobuf)
target_link_libraries(acc-engineer-rpc PRIVATE spdlog::spdlog)
target_link_libraries(acc-engineer-rpc PRIVATE msft_proxy)

target_include_directories(acc-engineer-rpc PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(acc-engineer-rpc PRIVATE BOOST_ASIO_NO_DEPRECATED)

target_include_directories(acc-engineer-rpc PUBLIC src/rpc)
target_compile_definitions(acc-engineer-rpc PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL})

if (WIN32)
    target_compile_definitions(acc-engineer-rpc PRIVATE _WIN32_WINNT=0x0A00)
endif ()

if (WIN32)
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
    target_link_libraries(acc-engineer-shared-memory PUBLIC Winmm)
    target_include_directories(acc-engineer-shared-memory PUBLIC src/shared_memory)
    target_compile_definitions(acc-engineer-shared-memory PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL})

    set(ACC_ENGINEER_STRATEGY_SETTER_SRCS
        ${ACC_ENGINEER_STRUCT_PROTO_FILES}
        src/strategy_setter/strategy_setter/strategy_setter.cpp
        src/strategy_setter/strategy_setter/strategy_setter.h)

    add_library(acc-engineer-strategy-setter ${ACC_ENGINEER_STRATEGY_SETTER_SRCS})
    target_link_libraries(acc-engineer-strategy-setter PRIVATE Boost::boost)
    target_link_libraries(acc-engineer-strategy-setter PRIVATE spdlog::spdlog)
    target_link_libraries(acc-engineer-strategy-setter PRIVATE protobuf::libprotobuf)
    target_link_libraries(acc-engineer-strategy-setter PRIVATE acc-engineer-shared-memory)
    target_include_directories(acc-engineer-strategy-setter PUBLIC src/strategy_setter)
    target_compile_definitions(acc-engineer-strategy-setter PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL})
    target_compile_definitions(acc-engineer-strategy-setter PRIVATE _WIN32_WINNT=0x0A00)
endif()

if (WIN32 AND ACC_ENGINEER_BUILD_CLIENT)
    set(ACC_ENGINEER_CLIENT_SRCS
        ${ACC_ENGINEER_SERVICE_PROTO_FILES}
        ${ACC_ENGINEER_STRUCT_PROTO_FILES}
        client/src/ui/connect.cpp
        client/src/ui/connect.h
        client/src/ui/driver_input.cpp
        client/src/ui/driver_input.h
        client/src/ui/main_window.cpp
        client/src/ui/main_window.h
        client/src/ui/strategy.cpp
        client/src/ui/strategy.h
        client/src/ui/telemetry.cpp
        client/src/ui/telemetry.h
        client/src/ui/telemetry_tyre.cpp
        client/src/ui/telemetry_tyre.h
        "client/src/ui/component/setter_button.cpp"
        "client/src/ui/component/setter_button.h"
        "client/src/ui/component/setter_spin_box.cpp"
        "client/src/ui/component/setter_spin_box.h"
        client/src/logic/logic.cpp
        client/src/logic/logic.h
        client/src/app.cpp
        client/src/app.h
      )

    add_executable(acc-engineer client/src/main.cpp ${ACC_ENGINEER_CLIENT_SRCS})
    target_link_directories(acc-engineer PRIVATE ${QWT_LIBRARY_DIR})
    target_link_libraries(acc-engineer PRIVATE acc-engineer-shared-memory acc-engineer-rpc acc-engineer-strategy-setter)
    target_link_libraries(acc-engineer PRIVATE Boost::boost)
    target_link_libraries(acc-engineer PRIVATE protobuf::libprotobuf)
    target_link_libraries(acc-engineer PRIVATE spdlog::spdlog)
    target_link_libraries(acc-engineer PRIVATE Qt6::Core Qt6::Widgets Qt6::OpenGL)
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_link_libraries(acc-engineer PRIVATE qwtd)
    else()
        target_link_libraries(acc-engineer PRIVATE qwt)
    endif()

    target_include_directories(acc-engineer PRIVATE ${CMAKE_CURRENT_BINARY_DIR} client/src)
    target_compile_definitions(acc-engineer PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL} BOOST_ASIO_NO_DEPRECATED)
    if (WIN32)
        target_compile_definitions(acc-engineer PRIVATE _WIN32_WINNT=0x0A00)
    endif ()
endif()

if (ACC_ENGINEER_BUILD_SERVER)
    set(ACC_ENGINEER_SERVER_SRCS
            ${ACC_ENGINEER_SERVICE_PROTO_FILES}
            ${ACC_ENGINEER_STRUCT_PROTO_FILES}
            server/src/service/config.cpp
            server/src/service/config.h
            server/src/service/service.cpp
            server/src/service/service.h)
    add_executable(acc-engineer-server ${ACC_ENGINEER_SERVER_SRCS} server/src/main.cpp)
    target_link_libraries(acc-engineer-server PRIVATE acc-engineer-rpc)
    target_link_libraries(acc-engineer-server PRIVATE Boost::asio Boost::program_options)
    target_link_libraries(acc-engineer-server PRIVATE protobuf::libprotobuf)
    target_link_libraries(acc-engineer-server PRIVATE spdlog::spdlog)
    target_include_directories(acc-engineer-server PRIVATE ${CMAKE_CURRENT_BINARY_DIR} server/src/)
    target_compile_definitions(acc-engineer-server PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL} BOOST_ASIO_NO_DEPRECATED)
    if (WIN32)
        target_compile_definitions(acc-engineer-server PRIVATE _WIN32_WINNT=0x0A00)
    endif ()
endif()

function(add_acc_engineer_test)
    cmake_parse_arguments(ACC_ENGINEER_TEST "" "NAME" "FILES" ${ARGN})
    add_executable(${ACC_ENGINEER_TEST_NAME} ${ACC_ENGINEER_TEST_FILES})
    target_include_directories(${ACC_ENGINEER_TEST_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
    target_compile_definitions(${ACC_ENGINEER_TEST_NAME} PRIVATE SPDLOG_ACTIVE_LEVEL=${ACC_ENGINEER_LOG_LEVEL} BOOST_ASIO_NO_DEPRECATED)
    target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE acc-engineer-rpc)
    target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE Boost::asio Boost::program_options)
    target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE protobuf::libprotobuf)
    target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE spdlog::spdlog)

    if (MSVC)
        target_compile_definitions(${ACC_ENGINEER_TEST_NAME} PRIVATE _WIN32_WINNT=0x0A00)
    endif ()

    if (WIN32)
        target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE acc-engineer-shared-memory)
        target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE acc-engineer-strategy-setter)
        target_link_libraries(${ACC_ENGINEER_TEST_NAME} PRIVATE Winmm)
        target_compile_definitions(${ACC_ENGINEER_TEST_NAME} PRIVATE _WIN32_WINNT=0x0A00)
    endif()
endfunction()

if (ACC_ENGINEER_BUILD_TESTS)
    add_acc_engineer_test(NAME test_batch_task FILES test/test_batch_task.cpp)
    add_acc_engineer_test(NAME test_client FILES test/test_client.cpp ${ACC_ENGINEER_SERVICE_PROTO_FILES})
    add_acc_engineer_test(NAME test_direct_input FILES test/test_direct_input.cpp ${ACC_ENGINEER_STRUCT_PROTO_FILES})
    add_acc_engineer_test(NAME test_high_precision_sleep FILES test/test_high_precision_sleep.cpp)
endif()

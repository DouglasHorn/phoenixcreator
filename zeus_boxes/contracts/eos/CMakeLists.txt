project(eoscontract NONE)
cmake_minimum_required(VERSION 3.5)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(ExternalProject)
# if no cdt root is given use default path
if(EOSIO_CDT_ROOT STREQUAL "" OR NOT EOSIO_CDT_ROOT)
   find_package(eosio.cdt)
endif()
ExternalProject_Add(
               dappservices
               SOURCE_DIR dappservices
               BINARY_DIR dappservices
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               allservices
               SOURCE_DIR allservices
               BINARY_DIR allservices
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               logconsumer
               SOURCE_DIR logconsumer
               BINARY_DIR logconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               ipfsconsumer
               SOURCE_DIR ipfsconsumer
               BINARY_DIR ipfsconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               oldipfscons
               SOURCE_DIR oldipfscons
               BINARY_DIR oldipfscons
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               cronconsumer
               SOURCE_DIR cronconsumer
               BINARY_DIR cronconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vaccountsconsumer
               SOURCE_DIR vaccountsconsumer
               BINARY_DIR vaccountsconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vaccountsremote
               SOURCE_DIR vaccountsremote
               BINARY_DIR vaccountsremote
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               oracleconsumer
               SOURCE_DIR oracleconsumer
               BINARY_DIR oracleconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               authenticator
               SOURCE_DIR authenticator
               BINARY_DIR authenticator
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               storageconsumer
               SOURCE_DIR storageconsumer
               BINARY_DIR storageconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               readfnconsumer
               SOURCE_DIR readfnconsumer
               BINARY_DIR readfnconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               dnsconsumer
               SOURCE_DIR dnsconsumer
               BINARY_DIR dnsconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vcpuconsumer
               SOURCE_DIR vcpuconsumer
               BINARY_DIR vcpuconsumer
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               dappservicex
               SOURCE_DIR dappservicex
               BINARY_DIR dappservicex
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               liquidx
               SOURCE_DIR liquidx
               BINARY_DIR liquidx
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               ipfsxtest
               SOURCE_DIR ipfsxtest
               BINARY_DIR ipfsxtest
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               orcxtest
               SOURCE_DIR orcxtest
               BINARY_DIR orcxtest
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               readfnxtest
               SOURCE_DIR readfnxtest
               BINARY_DIR readfnxtest
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vaccountsx
               SOURCE_DIR vaccountsx
               BINARY_DIR vaccountsx
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vaccntxrem
               SOURCE_DIR vaccntxrem
               BINARY_DIR vaccntxrem
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               vaccntxremx
               SOURCE_DIR vaccntxremx
               BINARY_DIR vaccntxremx
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               cronxtest
               SOURCE_DIR cronxtest
               BINARY_DIR cronxtest
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               storagextest
               SOURCE_DIR storagextest
               BINARY_DIR storagextest
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               phoenix
               SOURCE_DIR phoenix
               BINARY_DIR phoenix
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

ExternalProject_Add(
               phoenixtoken
               SOURCE_DIR phoenixtoken
               BINARY_DIR phoenixtoken
               CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${EOSIO_CDT_ROOT}/lib/cmake/eosio.cdt/EosioWasmToolchain.cmake
               UPDATE_COMMAND ""
               PATCH_COMMAND ""
               TEST_COMMAND ""
               INSTALL_COMMAND ""
               BUILD_ALWAYS 1
            )

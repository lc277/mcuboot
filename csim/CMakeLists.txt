# CMake build for the mcuboot C simulation.
#
cmake_minimum_required(VERSION 3.16)

project(mcuboot_c_sim C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)              # use -std=gnu99 like the Makefile

set(MCUBOOT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")

# Recursive source collection, equivalent to the Makefile rwildcard rules.
# CONFIGURE_DEPENDS re-runs the globs whenever the tree changes.
file(GLOB LOCAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE MCUBOOT_SYS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/mcuboot-sys/*.c")
file(GLOB_RECURSE TINYCRYPT_SOURCES CONFIGURE_DEPENDS
    "${MCUBOOT_ROOT}/ext/tinycrypt/lib/source/*.c")
file(GLOB_RECURSE TINYCRYPT_SHA512_SOURCES CONFIGURE_DEPENDS
    "${MCUBOOT_ROOT}/ext/tinycrypt-sha512/lib/source/*.c")

# Mbed TLS is only needed as a library here: no test suites, no programs.
# Cache variables with FORCE are required: mbedtls' CMakeLists uses
# cmake_minimum_required(VERSION 3.5.1), which resets CMP0077 to the old
# behavior where option() clears plain (non-cache) variables.
set(ENABLE_TESTING OFF CACHE BOOL "Build Mbed TLS tests." FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "Build Mbed TLS programs." FORCE)

add_subdirectory("${MCUBOOT_ROOT}/ext/mbedtls-3.6.0" mbedtls)

# Reuse the bootutil CMake library from the mcuboot tree.
# It brings its own include dirs (boot/bootutil/include PUBLIC,
# boot/bootutil/src PRIVATE) and selects the FIH delay RNG via
# CONFIG_BOOT_USE_PSA_CRYPTO (off by default -> mbedtls variant).
add_subdirectory("${MCUBOOT_ROOT}/boot/bootutil" bootutil)

# bootutil expects the platform configuration headers
# (mcuboot_config.h, flash_map_backend.h, sysflash.h, ...) and the
# TinyCrypt headers from this simulation; the subproject itself does
# not know about them.
target_include_directories(bootutil PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/mcuboot-sys"
    "${MCUBOOT_ROOT}/ext/mbedtls-3.6.0/include"
    "${MCUBOOT_ROOT}/ext/tinycrypt/lib/include"
    "${MCUBOOT_ROOT}/ext/tinycrypt-sha512/lib/include"
)

add_executable(exe
    ${LOCAL_SOURCES}
    ${MCUBOOT_SYS_SOURCES}
    ${TINYCRYPT_SOURCES}
    ${TINYCRYPT_SHA512_SOURCES}
)

# Defines from the Makefile (C_DEFS).
target_compile_definitions(exe PRIVATE MAKEFILE)

# Include paths from the Makefile (C_INCLUDES).
target_include_directories(exe PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/mcuboot-sys"
    "${MCUBOOT_ROOT}/ext/mbedtls-3.6.0/include"
    "${MCUBOOT_ROOT}/ext/tinycrypt/lib/include"
    "${MCUBOOT_ROOT}/ext/tinycrypt-sha512/lib/include"
)

# Compiler flags from the Makefile (CFLAGS), minus the debug/optimization
# level which is handled below.
target_compile_options(exe PRIVATE
    -Wall
    -fmessage-length=0
)

# The Makefile defaults to a debug build (-Og -g3); a Release build uses -O2.
# Applied at directory scope so the bootutil subproject gets the same flags.
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2)
else()
    add_compile_options(-Og -g3)
endif()

# Libraries from the Makefile (LIBS). The bootutil target also propagates
# its public include directories (boot/bootutil/include) to the executable.
target_link_libraries(exe PRIVATE mbedtls bootutil m rt)

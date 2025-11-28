#******************************************************************************
# Toolchain.cmake : tool chain configuration
# Must be included BEFORE project() for cross-compilation
#******************************************************************************
if(DEFINED AMBIQ_TOOLCHAIN_LOADED)
    return()
endif()
set(AMBIQ_TOOLCHAIN_LOADED TRUE CACHE INTERNAL "Ambiq toolchain configured")

# Set toolchain prefix (allow override via cache)
if(NOT DEFINED TOOLCHAIN_PREFIX)
    set(TOOLCHAIN_PREFIX "arm-none-eabi-" CACHE STRING "Toolchain prefix")
endif()

# Set system variables (must be set before project() for cross-compilation)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# Set compiler variables (must be set before project() for cross-compilation)
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}gcc")

# Set tool variables
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_PREFIX}objdump")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}size")

# Disable compiler tests for cross-compilation
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Cache values for later use
set(CMAKE_SYSTEM_NAME ${CMAKE_SYSTEM_NAME} CACHE STRING "Ambiq target system" FORCE)
set(CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "Ambiq target processor" FORCE)
set(CMAKE_C_COMPILER ${CMAKE_C_COMPILER} CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER ${CMAKE_CXX_COMPILER} CACHE FILEPATH "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER ${CMAKE_ASM_COMPILER} CACHE FILEPATH "ASM compiler" FORCE)
set(CMAKE_OBJCOPY ${CMAKE_OBJCOPY} CACHE FILEPATH "Objcopy tool" FORCE)
set(CMAKE_OBJDUMP ${CMAKE_OBJDUMP} CACHE FILEPATH "Objdump tool" FORCE)
set(CMAKE_SIZE ${CMAKE_SIZE} CACHE FILEPATH "Size tool" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE ${CMAKE_TRY_COMPILE_TARGET_TYPE} CACHE STRING "Try compile as static library" FORCE)

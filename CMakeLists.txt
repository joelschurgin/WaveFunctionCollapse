cmake_minimum_required(VERSION 3.10)
project(WaveFunctionCollapse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Enable multi-config generators (Visual Studio, Xcode)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directories for different configurations
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin/RelWithDebInfo)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin/MinSizeRel)

# Set library output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/lib/RelWithDebInfo)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/lib/MinSizeRel)

# Set archive output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib/Release)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/lib/RelWithDebInfo)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/lib/MinSizeRel)

# Compiler-specific settings
if(MSVC)
    # Visual Studio specific settings
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_RELEASE "/MD /O2 /Ob2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MD /Zi /O2 /Ob1 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "/MD /O1 /Ob1 /DNDEBUG")
    
    # Warning suppression for stb_image_write
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4996 /wd4244 /wd4267 /wd4996")
    
    # Set architecture-specific flags
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN64")
        message(STATUS "Building for 64-bit (x64)")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32")
        message(STATUS "Building for 32-bit (x86)")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang specific settings
    set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")
    
    # Warning suppression for stb_image_write
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-parameter -Wno-unused-variable")
    
    # Set architecture-specific flags
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
        message(STATUS "Building for 64-bit (x64)")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        message(STATUS "Building for 32-bit (x86)")
    endif()
endif()

# Define source files
set(COMMON_SOURCES
    main_seq_Q.cpp
    main_omp.cpp
    main_pattern_caching.cpp
    main_pattern_weight.cpp
)

# Create executables for each source file
foreach(source_file ${COMMON_SOURCES})
    get_filename_component(executable_name ${source_file} NAME_WE)
    add_executable(${executable_name} ${source_file})
    
    # Set target properties for better organization
    set_target_properties(${executable_name} PROPERTIES
        FOLDER "Executables"
        DEBUG_POSTFIX "_d"
        RELEASE_POSTFIX ""
    )
    
    # Add warning suppression specifically for stb_image_write
    if(MSVC)
        target_compile_options(${executable_name} PRIVATE 
            /wd4996  # 'function': was declared deprecated
            /wd4244  # 'conversion' conversion from 'type1' to 'type2', possible loss of data
            /wd4267  # 'var' : conversion from 'size_t' to 'type', possible loss of data
            /wd4996  # The POSIX name for this item is deprecated
        )
    else()
        target_compile_options(${executable_name} PRIVATE 
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-sign-conversion
            -Wno-conversion
        )
    endif()
endforeach()

# Optional: Add compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(main_omp PRIVATE -fopenmp)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        target_link_libraries(main_omp OpenMP::OpenMP_CXX)
    endif()
endif()

# Print build configuration info
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Architecture: 64-bit")
else()
    message(STATUS "Architecture: 32-bit")
endif()
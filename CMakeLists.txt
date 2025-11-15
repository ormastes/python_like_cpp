cmake_minimum_required(VERSION 3.20)
project(PyLike VERSION 1.0.0 LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clangd/LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prefer Clang if available
if(NOT CMAKE_CXX_COMPILER)
    find_program(CLANG_CXX_COMPILER NAMES clang++-20 clang++-19 clang++-18 clang++-17 clang++-16 clang++-15 clang++)
    if(CLANG_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER ${CLANG_CXX_COMPILER})
    endif()
endif()

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
    )
endif()

# Coverage support
option(ENABLE_COVERAGE "Enable code coverage" OFF)
if(ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        # LLVM/Clang coverage
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate)
        message(STATUS "Code coverage enabled (LLVM)")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        # GCC coverage
        add_compile_options(--coverage)
        add_link_options(--coverage)
        message(STATUS "Code coverage enabled (GCC)")
    else()
        message(WARNING "Code coverage requested but not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
    endif()
endif()

# PyLike library (pyl namespace)
# pyl_ranges.h, pyl_strong_num.h, pyl_basic_types.h, and pyl_object_interface.h are header-only
# pyl_text has both .h and .cpp
add_library(pyl pyl_text.cpp)
target_include_directories(pyl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(pyl PUBLIC cxx_std_20)

# Testing
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()

    # Fetch Catch2
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    # Add Catch2 CMake module path
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

    # Test executable
    add_executable(pyl_tests
        tests/test_pyl_ranges.cpp
        tests/test_pyl_text.cpp
        tests/test_pyl_child_ptr.cpp
        tests/test_pyl_strong_num.cpp
        tests/test_pyl_basic_types.cpp
    )
    target_link_libraries(pyl_tests PRIVATE pyl Catch2::Catch2WithMain)

    # Auto-discover tests
    include(CTest)
    include(Catch)
    catch_discover_tests(pyl_tests)

    # Coverage report targets
    if(ENABLE_COVERAGE)
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            # LLVM coverage report targets
            # Extract Clang version to find matching llvm-profdata and llvm-cov
            execute_process(
                COMMAND ${CMAKE_CXX_COMPILER} --version
                OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
            )
            string(REGEX MATCH "clang version ([0-9]+)" _ "${CLANG_VERSION_OUTPUT}")
            set(CLANG_MAJOR_VERSION "${CMAKE_MATCH_1}")

            # Search for version-specific tools first, then generic
            find_program(LLVM_PROFDATA NAMES llvm-profdata-${CLANG_MAJOR_VERSION} llvm-profdata)
            find_program(LLVM_COV NAMES llvm-cov-${CLANG_MAJOR_VERSION} llvm-cov)

            if(LLVM_PROFDATA AND LLVM_COV)
                add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} -E env LLVM_PROFILE_FILE=pyl_tests.profraw $<TARGET_FILE:pyl_tests>
                    COMMAND ${LLVM_PROFDATA} merge -sparse pyl_tests.profraw -o pyl_tests.profdata
                    COMMAND ${LLVM_COV} show $<TARGET_FILE:pyl_tests> -instr-profile=pyl_tests.profdata -format=html -output-dir=coverage_html -show-line-counts-or-regions -show-instantiations=false
                    COMMAND ${LLVM_COV} report $<TARGET_FILE:pyl_tests> -instr-profile=pyl_tests.profdata
                    DEPENDS pyl_tests
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating LLVM coverage report in coverage_html/"
                )
                message(STATUS "Using ${LLVM_PROFDATA} and ${LLVM_COV} for coverage")
            else()
                message(WARNING "llvm-profdata or llvm-cov not found - coverage target not available")
            endif()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # GCC coverage report target
            find_program(GCOVR gcovr)
            if(GCOVR)
                add_custom_target(coverage
                    COMMAND ${CMAKE_COMMAND} -E make_directory coverage_html
                    COMMAND $<TARGET_FILE:pyl_tests>
                    COMMAND ${GCOVR} --html-details coverage_html/index.html --root ${CMAKE_SOURCE_DIR}
                    DEPENDS pyl_tests
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating GCC coverage report in coverage_html/"
                )
            else()
                message(STATUS "gcovr not found - coverage target not available")
            endif()
        endif()
    endif()
endif()

# Installation (optional)
include(GNUInstallDirs)
install(FILES
    pyl_ranges.h
    pyl_text.h
    pyl_child_ptr.h
    pyl_strong_num.h
    pyl_basic_types.h
    pyl_object_interface.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(TARGETS pyl
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

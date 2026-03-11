cmake_minimum_required(VERSION 3.14.7)
project(everest-tutorial VERSION 0.1
    DESCRIPTION "EVerest tutorial modules"
    LANGUAGES CXX C)

find_package(everest-cmake REQUIRED
    COMPONENTS bundling
)

# options
option(BUILD_TESTING "Run unit tests" OFF)
option(CMAKE_RUN_CLANG_TIDY "Run clang-tidy" OFF)

if(NOT EVEREST_SCHEMA_DIR)
    if(DEFINED CMAKE_SYSROOT AND EXISTS "${CMAKE_SYSROOT}/usr/share/everest/schemas")
        set(EVEREST_SCHEMA_DIR "${CMAKE_SYSROOT}/usr/share/everest/schemas")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/interfaces")
        set(EVEREST_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/interfaces")
    endif()
endif()

# dependencies
if (NOT DISABLE_EDM)
    evc_setup_edm()
else()
    find_package(everest-log REQUIRED)
    find_package(everest-sqlite REQUIRED)
    #find_package(everest-framework REQUIRED)
    find_package(everest-core REQUIRED)
    # InfyPowerACDC uses pal-sigslot
    find_package(PalSigslot REQUIRED)
    #include(ev-project-bootstrap)
endif()

ev_add_project()

include(GNUInstallDirs)

# Install config file to /etc/everest
install(
    FILES config-tutorial-module.yaml
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/everest
)

# config (not needed if you do not need a run script for your configuration)
# add_subdirectory(config)

# configure clang-tidy if requested
if(CMAKE_RUN_CLANG_TIDY)
    message("Enabling clang-tidy")
    set(CMAKE_CXX_CLANG_TIDY clang-tidy)
endif()
# testing
if(BUILD_TESTING)
    include(CTest)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    evc_include(CodeCoverage)
    append_coverage_compiler_flags()
    add_subdirectory(tests)
    setup_target_for_coverage_gcovr_html(
        NAME gcovr_coverage
        EXECUTABLE test_config
        DEPENDENCIES test_config everest
    )
    setup_target_for_coverage_lcov(
        NAME lcov_coverage
        EXECUTABLE test_config
        DEPENDENCIES test_config everest
    )
endif()

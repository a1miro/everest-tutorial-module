evc_add_module(
    MODULE_NAME WhiteBeetSlac
    SOURCES
        WhiteBeetSlac.cpp
        main/slacImpl.cpp
)

# Find Python3 development libraries
find_package(Python3 COMPONENTS Development REQUIRED)

target_include_directories(WhiteBeetSlac PRIVATE
    ${Python3_INCLUDE_DIRS}
)

target_link_libraries(WhiteBeetSlac PRIVATE
    ${Python3_LIBRARIES}
)

# Add FreeV2G path to runtime search path
set_target_properties(WhiteBeetSlac PROPERTIES
    BUILD_RPATH "/opt/FreeV2G"
    INSTALL_RPATH "/opt/FreeV2G"
)

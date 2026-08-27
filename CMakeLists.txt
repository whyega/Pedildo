cmake_minimum_required(VERSION 3.15)

project(Pedildo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
endif()

include(FetchContent)


FetchContent_Declare(
    plugintmplt
    GIT_REPOSITORY https://github.com/whyega/plugintmplt.git
    GIT_TAG v1.0.2
)

FetchContent_Declare(
    xbyak
    GIT_REPOSITORY https://github.com/herumi/xbyak.git
    GIT_TAG ce083a0dcc306c1717685a81f577a4e050193919
)

FetchContent_Declare(
    ktsignal
    GIT_REPOSITORY https://github.com/kin4stat/ktsignal.git
    GIT_TAG f08a19d9a00ad5284871bf44178d835da7b26ebf
)

FetchContent_Declare(
    kthook
    GIT_REPOSITORY https://github.com/kin4stat/kthook.git
    GIT_TAG 21ca70481273d1efcc98ca49063178e46277009b
)

FetchContent_Declare(
    sampapi
    GIT_REPOSITORY https://github.com/BlastHackNet/SAMP-API.git
    GIT_TAG 6d4db99ab41f19d1a6a7c6cd48f5878bd1e14b62
)

FetchContent_MakeAvailable(
    plugintmplt
    xbyak
    ktsignal
    kthook
    sampapi
)

if(TARGET simpleplugin)
    set_target_properties(
        simpleplugin
        PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            EXCLUDE_FROM_DEFAULT_BUILD TRUE
    )
endif()

find_package(plugin-sdk REQUIRED)

set(PEDILDO_SOURCES
    src/main.cpp
    src/Plugin.cpp
    src/Commands.cpp
    src/Remote.cpp
    src/Skeleton.cpp
)

add_library(
    ${PROJECT_NAME}
    SHARED
    ${PEDILDO_SOURCES}
)

if(MSVC)
    set_property(
        TARGET ${PROJECT_NAME}
        PROPERTY MSVC_RUNTIME_LIBRARY
        "MultiThreadedDLL"
    )

    foreach(target_name IN ITEMS sampapi ${PROJECT_NAME})
        target_compile_definitions(
            ${target_name}
            PRIVATE
                $<$<CONFIG:Debug>:_ITERATOR_DEBUG_LEVEL=0>
        )
        target_compile_options(
            ${target_name}
            PRIVATE
                $<$<CONFIG:Debug>:/U_DEBUG>
        )
    endforeach()
endif()

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        pluginsdk::gtasa
        plugintmplt
        kthook
        BlastHack::sampapi
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES
        PREFIX ""
        SUFFIX ".asi"
)

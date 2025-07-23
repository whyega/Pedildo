cmake_minimum_required(VERSION 3.15)
project(Pedildo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDART_REQUIED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

list(APPEND PEDMASKRENDER_SRC
  src/main.cpp
  src/Plugin.cpp
)

add_library(${PROJECT_NAME} SHARED ${PEDMASKRENDER_SRC})

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

FetchContent_MakeAvailable(plugintmplt xbyak ktsignal kthook)

find_package(plugin-sdk REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
  pluginsdk::gtasa
  plugintmplt
  kthook
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  SUFFIX .asi
)
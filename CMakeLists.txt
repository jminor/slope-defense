cmake_minimum_required(VERSION 3.24)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
  cmake_policy(SET CMP0048 NEW)
endif(COMMAND cmake_policy)

set(CMAKE_OSX_ARCHITECTURES arm64;x86_64)

project(slope_defense VERSION 0.1.0)

if(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif(WIN32)

include(FetchContent) 
FetchContent_Declare(mahi-gui GIT_REPOSITORY https://github.com/mahilab/mahi-gui.git) 
FetchContent_MakeAvailable(mahi-gui)

FetchContent_Declare(exprtk GIT_REPOSITORY https://github.com/ArashPartow/exprtk.git)
FetchContent_MakeAvailable(exprtk)

add_executable(slope_defense "app.cpp")
target_link_libraries(slope_defense mahi::gui)
target_include_directories(slope_defense PRIVATE ${exprtk_SOURCE_DIR})



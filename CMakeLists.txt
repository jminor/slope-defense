cmake_minimum_required(VERSION 3.0)
project(slope_defense)

option(USE_FILE_SYSTEM "Enable using the filesystem" ON)

set(PROJ_SRCS
    main.cpp
    app.cpp
    )

set(IMGUI_DIR libs/imgui)

set(IMGUI_SRCS
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

set(IMPLOT_DIR libs/implot)

set(IMPLOT_SRCS
    ${IMPLOT_DIR}/implot.cpp
    ${IMPLOT_DIR}/implot_demo.cpp
    ${IMPLOT_DIR}/implot_items.cpp
    )

set(INCS
    ${CMAKE_CURRENT_LIST_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMPLOT_DIR}
    libs/exprtk
    )

set(LIBS ${CMAKE_DL_LIBS} SDL2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2 -s WASM=1 -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=0 -s ASSERTIONS=1")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_PTHREADS=1 -s ALLOW_MEMORY_GROWTH=1 -Wl,--shared-memory,--no-check-features --shell-file ${CMAKE_CURRENT_LIST_DIR}/shell_minimal.html")

if(USE_FILE_SYSTEM)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --no-heap-copy --preload-file ${CMAKE_CURRENT_LIST_DIR}/fonts@/fonts")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_FILESYSTEM=1 -DIMGUI_DISABLE_FILE_FUNCTIONS")
endif()

add_executable(${PROJECT_NAME}
    ${PROJ_SRCS}
    ${IMGUI_SRCS}
    ${IMPLOT_SRCS}
    )

set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX .html)

target_include_directories(${PROJECT_NAME} PUBLIC ${INCS})

target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})


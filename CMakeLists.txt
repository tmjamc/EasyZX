cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Whole Program Optimization
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG")

# Enable Linker Optimizations
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /OPT:REF /OPT:ICF")

# Disable Incremental Linking (conflicts with LTCG)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /INCREMENTAL:NO")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /INCREMENTAL:NO")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("EasyZX")

add_definitions(-D__WINDOWS_WASAPI__)

add_executable(EasyZX
WIN32
	src/win_app.cpp
	src/settings.cpp
	src/display.cpp
	src/shader.cpp
	src/audio.cpp
	src/ui/realtime_monitor.cpp
	src/spectrum/main.cpp
	src/spectrum/components/memory.cpp
	src/spectrum/components/z80.cpp
	src/spectrum/components/ula.cpp
	src/spectrum/components/io.cpp
	src/spectrum/components/beeper.cpp
	src/spectrum/devices/tape.cpp
	src/spectrum/devices/beta_disk.cpp
	src/spectrum/devices/wd_1793.cpp
	src/spectrum/devices/ay_3_8912.cpp
	src/resources/resources.rc
	src/libs/glad/src/gl.c
	src/libs/glad/src/wgl.c
	src/libs/mINI/ini.h
	src/libs/RtAudio/RtAudio.cpp
	src/libs/audio_filters/dc_adjustment_filter.cpp
	src/libs/ImGui/imgui.cpp
	src/libs/ImGui/imgui_draw.cpp
	src/libs/ImGui/imgui_tables.cpp
	src/libs/ImGui/imgui_widgets.cpp
	src/libs/ImGui/backends/imgui_impl_opengl3.cpp
	src/libs/ImGui/backends/imgui_impl_win32.cpp
)

target_include_directories(EasyZX
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/ui
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum/components
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum/devices
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/mINI
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/RtAudio
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/audio_filters
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/ImGui
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/ImGui/backends
)

target_link_libraries(EasyZX
PRIVATE
	opengl32
)
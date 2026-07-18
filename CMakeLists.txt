cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
	src/spectrum/main.cpp
	src/spectrum/components/memory.cpp
	src/spectrum/components/z80.cpp
	src/spectrum/components/ula.cpp
	src/spectrum/components/io.cpp
	src/spectrum/components/beeper.cpp
	src/spectrum/devices/tape.cpp
	src/spectrum/devices/beta_disk.cpp
	src/spectrum/devices/wd_1793.cpp
	src/resources/resources.rc
	src/libs/glad/src/gl.c
	src/libs/glad/src/wgl.c
	src/libs/mINI/ini.h
	src/libs/RtAudio/RtAudio.cpp
	src/libs/audio_filters/dc_adjustment_filter.cpp
)

target_include_directories(EasyZX
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum/components
	${CMAKE_CURRENT_SOURCE_DIR}/src/spectrum/devices
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/mINI
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/glad/include
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/RtAudio
	${CMAKE_CURRENT_SOURCE_DIR}/src/libs/audio_filters
)

target_link_libraries(EasyZX
PRIVATE
	opengl32
)
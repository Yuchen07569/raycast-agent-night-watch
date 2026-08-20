cmake_minimum_required(VERSION 3.24)

project(agent-night-watch-windows VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(agent-night-watch-core STATIC
  src/power_session.cpp
  src/session.cpp
)
target_include_directories(agent-night-watch-core PUBLIC include)

if(MSVC)
  target_compile_options(agent-night-watch-core PRIVATE /W4 /WX /permissive- /EHsc /utf-8 /analyze)
  set_property(TARGET agent-night-watch-core PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  target_compile_options(agent-night-watch-core PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(agent-night-watch-tests tests/test_main.cpp)
  target_link_libraries(agent-night-watch-tests PRIVATE agent-night-watch-core)
  if(MSVC)
    target_compile_options(agent-night-watch-tests PRIVATE /W4 /WX /permissive- /EHsc /utf-8)
    set_property(TARGET agent-night-watch-tests PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  else()
    target_compile_options(agent-night-watch-tests PRIVATE -Wall -Wextra -Wpedantic -Werror)
  endif()
  add_test(NAME agent-night-watch-tests COMMAND agent-night-watch-tests)
endif()

if(WIN32)
  add_executable(agent-night-watch WIN32
    resources/version.rc
    src/main.cpp
    src/windows_platform.cpp
  )
  target_include_directories(agent-night-watch PRIVATE include src resources)
  target_compile_definitions(agent-night-watch PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
  target_link_libraries(agent-night-watch PRIVATE
    agent-night-watch-core
    PowrProf
    Shell32
    User32
    Ole32
  )
  if(MSVC)
    target_compile_options(agent-night-watch PRIVATE /W4 /WX /permissive- /EHsc /utf-8 /analyze)
    # version.rc already embeds the explicit asInvoker application manifest.
    # Disable the linker's generated manifest to avoid a duplicate resource.
    target_link_options(agent-night-watch PRIVATE /SUBSYSTEM:WINDOWS /MANIFEST:NO)
    set_property(TARGET agent-night-watch PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()
  set_target_properties(agent-night-watch PROPERTIES OUTPUT_NAME "AgentNightWatch")
endif()

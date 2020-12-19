cmake_minimum_required(VERSION 3.0.0)
project(mled VERSION 0.1.0)

include(CTest)
enable_testing()

include_directories(. input)
add_executable(mled main.c audio.c wave.c lights.c input/pulse.c)
target_link_libraries(mled fftw3f pthread pulse-simple pulse ws2811 m)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

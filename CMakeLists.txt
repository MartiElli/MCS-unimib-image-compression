cmake_minimum_required(VERSION 3.14)

project(MCS-image-compression VERSION 1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Qt MOC code x slot/signals
set(CMAKE_AUTOMOC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_library(FFTW3_LIB fftw3 REQUIRED)

# eseguibile 1: benchmark DCT2 custom vs FFTW
add_executable(DCTComparison dct2compare.cpp)
target_link_libraries(DCTComparison PRIVATE Eigen3::Eigen ${FFTW3_LIB})

# eseguibile 2: compressione immagini con GUI
add_executable(ImageCompression
    src/main.cpp
    src/mainwindow.cpp
    include/mainwindow.h
)
# ese 2 punta a include x mainwindow.h
target_include_directories(ImageCompression PRIVATE include)
target_link_libraries(ImageCompression PRIVATE Qt6::Widgets Eigen3::Eigen ${FFTW3_LIB})

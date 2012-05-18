cmake_minimum_required(VERSION 2.6)

PROJECT(InteractivePatchComparison)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

include_directories(/media/portable/src/Eigen)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Submodules
add_subdirectory(EigenHelpers)
add_subdirectory(Helpers)
add_subdirectory(ITKHelpers)
add_subdirectory(VTKHelpers)
add_subdirectory(ITKVTKHelpers)
add_subdirectory(Mask)
add_subdirectory(QtHelpers)
add_subdirectory(ITKQtHelpers)

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(InteractivePatchComparisonWidgetUISrcs
InteractivePatchComparisonWidget.ui PatchInfoWidget.ui)
QT4_WRAP_CPP(InteractivePatchComparisonWidgetMOCSrcs
InteractivePatchComparisonWidget.h
PatchInfoWidget.h)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

ADD_EXECUTABLE(InteractivePatchComparison
DiffusionDistance.cpp
InteractivePatchComparisonWidget.cxx
Interactive.cpp 
SwitchBetweenStyle.cxx
CustomImageStyle.cxx
CustomTrackballStyle.cxx
VarianceScore.hpp
PatchInfoWidget.cxx
${InteractivePatchComparisonWidgetUISrcs} ${InteractivePatchComparisonWidgetMOCSrcs})
TARGET_LINK_LIBRARIES(InteractivePatchComparison EigenHelpers libQtHelpers libHelpers libVTKHelpers libITKHelpers libITKVTKHelpers libMask ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS InteractivePatchComparison RUNTIME DESTINATION ${INSTALL_DIR} )

# 
# ADD_EXECUTABLE(ExamplePatchDifference ExamplePatchDifference.cpp)
# TARGET_LINK_LIBRARIES(ExamplePatchDifference PatchComparison ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})

# ADD_EXECUTABLE(DiffusionDistanceTest DiffusionDistanceTest.cpp DiffusionDistance.cpp Helpers.cpp)
# TARGET_LINK_LIBRARIES(DiffusionDistanceTest ${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
# these libraries are required because Helpers has many other things, even though we don't use them in this executable

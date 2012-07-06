cmake_minimum_required(VERSION 2.6)

PROJECT(InteractivePatchComparison)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

include_directories(/media/portable/src/Eigen)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Submodules
if(NOT TARGET VTKHelpers)
  add_subdirectory(VTKHelpers)
endif()

if(NOT TARGET ITKVTKHelpers)
  add_subdirectory(ITKVTKHelpers)
endif()

if(NOT TARGET Mask)
  add_subdirectory(Mask)
endif()

if(NOT TARGET QtHelpers)
  add_subdirectory(QtHelpers)
endif()

if(NOT TARGET ITKQtHelpers)
  add_subdirectory(ITKQtHelpers)
endif()

if(NOT TARGET PatchComparison)
  add_subdirectory(PatchComparison)
endif()

if(NOT TARGET ITKVTKCamera)
  add_subdirectory(ITKVTKCamera)
endif()

if(NOT TARGET PatchClustering)
  add_subdirectory(PatchClustering)
endif()

if(NOT TARGET PatchProjection)
  add_subdirectory(PatchProjection)
endif()

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

if(${VTK_MAJOR_VERSION} LESS 6)
  message(FATAL_ERROR "Your VTK version (${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}) is too old! At least 6.0 is required.")
endif()

FIND_PACKAGE(ITK 4 REQUIRED)
INCLUDE(${ITK_USE_FILE})

QT4_WRAP_UI(InteractivePatchComparisonWidgetUISrcs
InteractivePatchComparisonWidget.ui
PatchInfoWidget.ui
TopPatchesWidget.ui)
QT4_WRAP_CPP(InteractivePatchComparisonWidgetMOCSrcs
InteractivePatchComparisonWidget.h
PatchInfoWidget.h
TopPatchesWidget.h
PixmapDelegate.h)

ADD_EXECUTABLE(InteractivePatchComparison
InteractivePatchComparisonWidget.cpp
Interactive.cpp 
SwitchBetweenStyle.cxx
CustomImageStyle.cxx
CustomTrackballStyle.cxx
PatchInfoWidget.cpp
TopPatchesWidget.cpp
SelfPatchCompare.cpp
PixmapDelegate.cpp
TableModelTopPatches.cpp
Layer.cpp
${InteractivePatchComparisonWidgetUISrcs} ${InteractivePatchComparisonWidgetMOCSrcs})
TARGET_LINK_LIBRARIES(InteractivePatchComparison 
EigenHelpers QtHelpers Helpers VTKHelpers ITKHelpers ITKVTKHelpers
Mask ITKVTKCamera
PatchComparison
PatchClustering
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS InteractivePatchComparison RUNTIME DESTINATION ${INSTALL_DIR} )

get_directory_property(output INCLUDE_DIRECTORIES)
message(${output})

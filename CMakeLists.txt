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
  include_directories(${VTKHelpers_includes})
endif()

if(NOT TARGET QtHelpers)
  add_subdirectory(QtHelpers)
  include_directories(${QtHelpers_includes})
endif()

if(NOT TARGET ITKQtHelpers)
  add_subdirectory(ITKQtHelpers)
  include_directories(${ITKQtHelpers_includes})
endif()

if(NOT TARGET PatchComparison)
  add_subdirectory(PatchComparison)
  include_directories(${PatchComparison_includes})
endif()

if(NOT TARGET ITKVTKCamera)
  add_subdirectory(ITKVTKCamera)
  include_directories(${ITKVTKCamera_includes})
endif()

if(NOT TARGET PatchClustering)
  add_subdirectory(PatchClustering)
  include_directories(${PatchClustering_includes})
endif()

if(NOT TARGET Layer)
  add_subdirectory(Layer)
  include_directories(${Layer_includes})
endif()

FIND_PACKAGE(Boost COMPONENTS regex) # need the libboost-regex1.48.0 and libboost-regex1.48.0-dev packages installed (Ubuntu). Without the -dev package, CMake says "Could NOT find Boost."
IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

if(${VTK_MAJOR_VERSION} LESS 6)
  message(FATAL_ERROR "Your VTK version (${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION})
          is too old! At least 6.0 is required.")
endif()

FIND_PACKAGE(ITK 4 REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKDistanceMap ITKTestKernel
             ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology)
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
OddValidator.cpp
PixmapDelegate.cpp
TableModelTopPatches.cpp
${InteractivePatchComparisonWidgetUISrcs} ${InteractivePatchComparisonWidgetMOCSrcs})
TARGET_LINK_LIBRARIES(InteractivePatchComparison 
EigenHelpers QtHelpers Helpers VTKHelpers ITKHelpers ITKVTKHelpers
Mask ITKVTKCamera
Layer
PatchComparison
PatchClustering
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS InteractivePatchComparison RUNTIME DESTINATION ${INSTALL_DIR} )

get_directory_property(output INCLUDE_DIRECTORIES)
message(${output})

#####################

QT4_WRAP_UI(ViewAllMatchesWidgetUISrcs
ViewAllMatchesWidget.ui)
QT4_WRAP_CPP(ViewAllMatchesMOCSrcs
ViewAllMatchesWidget.h
PixmapDelegate.h)

ADD_EXECUTABLE(ViewAllMatches
ViewAllMatches.cpp
PixmapDelegate.cpp
TableModelViewAllMatches.cpp
ViewAllMatchesWidget.cpp
${ViewAllMatchesWidgetUISrcs} ${ViewAllMatchesMOCSrcs})

TARGET_LINK_LIBRARIES(ViewAllMatches
EigenHelpers QtHelpers Helpers VTKHelpers ITKHelpers ITKVTKHelpers
PatchComparison
PatchClustering
${VTK_LIBRARIES} ${ITK_LIBRARIES} ${QT_LIBRARIES} boost_regex)
INSTALL( TARGETS InteractivePatchComparison RUNTIME DESTINATION ${INSTALL_DIR} )

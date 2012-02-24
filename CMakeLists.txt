cmake_minimum_required(VERSION 2.6)
PROJECT(InteractivePatchComparison)

# Where to copy executables when 'make install' is run
SET(INSTALL_DIR ${CMAKE_INSTALL_PREFIX} )
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) #fix the dynamic linking error (RPATH) that would occur without this

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_UI(UISrcs Form.ui)
QT4_WRAP_CPP(MOCSrcs Form.h)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${ITK_USE_FILE})

add_library(PatchComparison
Helpers.cpp
SelfPatchCompare.cpp
)

ADD_EXECUTABLE(InteractivePatchComparison
Form.cxx 
Interactive.cpp 
SwitchBetweenStyle.cxx
CustomImageStyle.cxx
CustomTrackballStyle.cxx
${UISrcs} ${MOCSrcs})
TARGET_LINK_LIBRARIES(InteractivePatchComparison PatchComparison ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})
INSTALL( TARGETS InteractivePatchComparison RUNTIME DESTINATION ${INSTALL_DIR} )


ADD_EXECUTABLE(ExamplePatchDifference ExamplePatchDifference.cpp)
TARGET_LINK_LIBRARIES(ExamplePatchDifference PatchComparison ${VTK_LIBRARIES} QVTK ${ITK_LIBRARIES} ${QT_LIBRARIES})

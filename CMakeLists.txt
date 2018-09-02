cmake_minimum_required(VERSION 3.5)

project(PathReconstruction)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://www.slicerigt.org")
set(EXTENSION_CATEGORY "IGT")
set(EXTENSION_CONTRIBUTORS "Thomas Vaughan (Queen's University)")
set(EXTENSION_DESCRIPTION "This extension contains a module for reconstructing catheter paths from spatial tracking information. Intended users should have real-time tracking hardware (e.g. electromagnetic field generator and sensors) connected to 3D Slicer through OpenIGTLink network." )
set(EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/2b/SlicerIGTLogo.png" )
set(EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/7/78/SlicerIGTScreenshot.png" )
set(EXTENSION_DEPENDS SlicerIGT SlicerRT MarkupsToModel ) # Specified as a space separated list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

find_package(SlicerIGT REQUIRED)
find_package(SlicerRT REQUIRED)
find_package(MarkupsToModel REQUIRED)

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(PathReconstruction)
add_subdirectory(PathVerification)

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})


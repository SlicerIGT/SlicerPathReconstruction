project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerMarkupsToModelModuleMRML_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerPathReconstructionTableWidget.h
  qSlicerPathReconstructionTableWidget.cxx
  )

set(${KIT}_MOC_SRCS
  qSlicerPathReconstructionTableWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerPathReconstructionTableWidget.ui
  )

set(${KIT}_RESOURCES)

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )

#-----------------------------------------------------------------------------
add_subdirectory(DesignerPlugins)

#-----------------------------------------------------------------------------

import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging

#
# PathVerification
#

class PathVerification(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "PathVerification"
    self.parent.categories = ["IGT"]
    self.parent.dependencies = []
    self.parent.contributors = ["Thomas Vaughan (Queen's University)"]
    self.parent.helpText = """Measure accuracy between two paths"""
    self.parent.helpText += self.getDefaultModuleDocumentationLink()
    self.parent.acknowledgementText = """This work was funded by an Ontario Graduate Scholarship."""

#
# PathVerificationWidget
#

class PathVerificationWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)
    
    # Instantiate and connect widgets ...

    preprocessingCollapsibleButton = ctk.ctkCollapsibleButton()
    preprocessingCollapsibleButton.text = "Preprocessing"
    self.layout.addWidget(preprocessingCollapsibleButton)
    preprocessingFormLayout = qt.QFormLayout(preprocessingCollapsibleButton)

    self.preprocessPathsComboBox = slicer.qMRMLNodeComboBox()
    self.preprocessPathsComboBox.nodeTypes = ["vtkMRMLPathReconstructionNode"]
    self.preprocessPathsComboBox.addEnabled = True
    self.preprocessPathsComboBox.renameEnabled = True
    self.preprocessPathsComboBox.removeEnabled = False
    self.preprocessPathsComboBox.noneEnabled = True
    self.preprocessPathsComboBox.showHidden = False
    self.preprocessPathsComboBox.showChildNodeTypes = False
    self.preprocessPathsComboBox.setMRMLScene( slicer.mrmlScene )
    preprocessingFormLayout.addRow("Paths to preprocess: ", self.preprocessPathsComboBox)

    self.segmentationComboBox = slicer.qMRMLNodeComboBox()
    self.segmentationComboBox.nodeTypes = ["vtkMRMLSegmentationNode"]
    self.segmentationComboBox.addEnabled = False
    self.segmentationComboBox.removeEnabled = False
    self.segmentationComboBox.noneEnabled = True
    self.segmentationComboBox.showHidden = False
    self.segmentationComboBox.showChildNodeTypes = False
    self.segmentationComboBox.setMRMLScene( slicer.mrmlScene )
    preprocessingFormLayout.addRow("Segmented Paths: ", self.segmentationComboBox)

    self.segmentExportButton = qt.QPushButton("Export Segments to Path")
    self.segmentExportButton.enabled = False
    preprocessingFormLayout.addRow(self.segmentExportButton)

    self.nearTrimAmountSpinBox = ctk.ctkDoubleSpinBox()
    self.nearTrimAmountSpinBox.setDecimals(1)
    self.nearTrimAmountSpinBox.setValue(0)
    self.nearTrimAmountSpinBox.minimum = 0
    self.nearTrimAmountSpinBox.maximum = 100
    self.nearTrimAmountSpinBox.singleStep = 1
    preprocessingFormLayout.addRow("Near Trim (mm): ", self.nearTrimAmountSpinBox)

    self.farTrimAmountSpinBox = ctk.ctkDoubleSpinBox()
    self.farTrimAmountSpinBox.setDecimals(1)
    self.farTrimAmountSpinBox.setValue(0)
    self.farTrimAmountSpinBox.minimum = 0
    self.farTrimAmountSpinBox.maximum = 100
    self.farTrimAmountSpinBox.singleStep = 1
    preprocessingFormLayout.addRow("Far Trim (mm): ", self.farTrimAmountSpinBox)

    self.trimButton = qt.QPushButton("Trim Points")
    self.trimButton.enabled = False
    preprocessingFormLayout.addRow(self.trimButton)

    self.refitButton = qt.QPushButton("Refit Paths")
    self.refitButton.enabled = False
    preprocessingFormLayout.addRow(self.refitButton)

    verificationCollapsibleButton = ctk.ctkCollapsibleButton()
    verificationCollapsibleButton.text = "Verification"
    self.layout.addWidget(verificationCollapsibleButton)
    verificationFormLayout = qt.QFormLayout(verificationCollapsibleButton)

    self.referencePathsComboBox = slicer.qMRMLNodeComboBox()
    self.referencePathsComboBox.nodeTypes = ["vtkMRMLPathReconstructionNode"]
    self.referencePathsComboBox.addEnabled = False
    self.referencePathsComboBox.renameEnabled = True
    self.referencePathsComboBox.removeEnabled = False
    self.referencePathsComboBox.noneEnabled = True
    self.referencePathsComboBox.showHidden = False
    self.referencePathsComboBox.showChildNodeTypes = False
    self.referencePathsComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Reference Paths: ", self.referencePathsComboBox)

    self.comparePathsComboBox = slicer.qMRMLNodeComboBox()
    self.comparePathsComboBox.nodeTypes = ["vtkMRMLPathReconstructionNode"]
    self.comparePathsComboBox.addEnabled = False
    self.comparePathsComboBox.renameEnabled = True
    self.comparePathsComboBox.removeEnabled = False
    self.comparePathsComboBox.noneEnabled = True
    self.comparePathsComboBox.showHidden = False
    self.comparePathsComboBox.showChildNodeTypes = False
    self.comparePathsComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Compare Paths: ", self.comparePathsComboBox)

    self.referenceEndMarkupsComboBox = slicer.qMRMLNodeComboBox()
    self.referenceEndMarkupsComboBox.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.referenceEndMarkupsComboBox.selectNodeUponCreation = True
    self.referenceEndMarkupsComboBox.addEnabled = True
    self.referenceEndMarkupsComboBox.removeEnabled = True
    self.referenceEndMarkupsComboBox.noneEnabled = True
    self.referenceEndMarkupsComboBox.showHidden = False
    self.referenceEndMarkupsComboBox.showChildNodeTypes = False
    self.referenceEndMarkupsComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Reference End Markups: ", self.referenceEndMarkupsComboBox)

    self.compareEndMarkupsComboBox = slicer.qMRMLNodeComboBox()
    self.compareEndMarkupsComboBox.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.compareEndMarkupsComboBox.selectNodeUponCreation = True
    self.compareEndMarkupsComboBox.addEnabled = True
    self.compareEndMarkupsComboBox.removeEnabled = True
    self.compareEndMarkupsComboBox.noneEnabled = True
    self.compareEndMarkupsComboBox.showHidden = False
    self.compareEndMarkupsComboBox.showChildNodeTypes = False
    self.compareEndMarkupsComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Compare End Markups: ", self.compareEndMarkupsComboBox)

    self.registrationComboBox = slicer.qMRMLNodeComboBox()
    self.registrationComboBox.nodeTypes = ["vtkMRMLFiducialRegistrationWizardNode"]
    self.registrationComboBox.selectNodeUponCreation = True
    self.registrationComboBox.addEnabled = True
    self.registrationComboBox.removeEnabled = True
    self.registrationComboBox.noneEnabled = True
    self.registrationComboBox.showHidden = False
    self.registrationComboBox.showChildNodeTypes = False
    self.registrationComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Fiducial Registration: ", self.registrationComboBox)

    self.compareToInitialTransformComboBox = slicer.qMRMLNodeComboBox()
    self.compareToInitialTransformComboBox.nodeTypes = ["vtkMRMLLinearTransformNode"]
    self.compareToInitialTransformComboBox.selectNodeUponCreation = True
    self.compareToInitialTransformComboBox.addEnabled = True
    self.compareToInitialTransformComboBox.removeEnabled = True
    self.compareToInitialTransformComboBox.noneEnabled = True
    self.compareToInitialTransformComboBox.showHidden = False
    self.compareToInitialTransformComboBox.showChildNodeTypes = False
    self.compareToInitialTransformComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("CompareToInitial Transform: ", self.compareToInitialTransformComboBox)

    self.registerInitialButton = qt.QPushButton("Register Initial")
    self.registerInitialButton.enabled = False
    verificationFormLayout.addRow(self.registerInitialButton)

    self.initialToReferenceTransformComboBox = slicer.qMRMLNodeComboBox()
    self.initialToReferenceTransformComboBox.nodeTypes = ["vtkMRMLLinearTransformNode"]
    self.initialToReferenceTransformComboBox.selectNodeUponCreation = True
    self.initialToReferenceTransformComboBox.addEnabled = True
    self.initialToReferenceTransformComboBox.removeEnabled = True
    self.initialToReferenceTransformComboBox.noneEnabled = True
    self.initialToReferenceTransformComboBox.showHidden = False
    self.initialToReferenceTransformComboBox.showChildNodeTypes = False
    self.initialToReferenceTransformComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("InitialToReference Transform: ", self.initialToReferenceTransformComboBox)

    self.registerICPButton = qt.QPushButton("Register ICP")
    self.registerICPButton.enabled = False
    verificationFormLayout.addRow(self.registerICPButton)

    self.compareToReferenceTransformComboBox = slicer.qMRMLNodeComboBox()
    self.compareToReferenceTransformComboBox.nodeTypes = ["vtkMRMLLinearTransformNode"]
    self.compareToReferenceTransformComboBox.selectNodeUponCreation = True
    self.compareToReferenceTransformComboBox.addEnabled = True
    self.compareToReferenceTransformComboBox.removeEnabled = True
    self.compareToReferenceTransformComboBox.noneEnabled = True
    self.compareToReferenceTransformComboBox.showHidden = False
    self.compareToReferenceTransformComboBox.showChildNodeTypes = False
    self.compareToReferenceTransformComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("CompareToReference Transform: ", self.compareToReferenceTransformComboBox)

    self.concatenateRegistrationButton = qt.QPushButton("Concatenate Registration")
    self.concatenateRegistrationButton.enabled = False
    verificationFormLayout.addRow(self.concatenateRegistrationButton)

    self.outputDistancesTableComboBox = slicer.qMRMLNodeComboBox()
    self.outputDistancesTableComboBox.nodeTypes = ["vtkMRMLTableNode"]
    self.outputDistancesTableComboBox.selectNodeUponCreation = True
    self.outputDistancesTableComboBox.addEnabled = True
    self.outputDistancesTableComboBox.removeEnabled = True
    self.outputDistancesTableComboBox.noneEnabled = True
    self.outputDistancesTableComboBox.showHidden = False
    self.outputDistancesTableComboBox.showChildNodeTypes = False
    self.outputDistancesTableComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Distances Table: ", self.outputDistancesTableComboBox)

    self.outputSummaryTableComboBox = slicer.qMRMLNodeComboBox()
    self.outputSummaryTableComboBox.nodeTypes = ["vtkMRMLTableNode"]
    self.outputSummaryTableComboBox.selectNodeUponCreation = True
    self.outputSummaryTableComboBox.addEnabled = True
    self.outputSummaryTableComboBox.removeEnabled = True
    self.outputSummaryTableComboBox.noneEnabled = True
    self.outputSummaryTableComboBox.showHidden = False
    self.outputSummaryTableComboBox.showChildNodeTypes = False
    self.outputSummaryTableComboBox.setMRMLScene( slicer.mrmlScene )
    verificationFormLayout.addRow("Summary Table: ", self.outputSummaryTableComboBox)

    self.computeButton = qt.QPushButton("Compute")
    self.computeButton.enabled = False
    verificationFormLayout.addRow(self.computeButton)

    # connections
    self.preprocessPathsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.segmentationComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.segmentExportButton.connect("clicked(bool)", self.onSegmentExportButton)
    self.trimButton.connect('clicked(bool)', self.onTrimButton)
    self.refitButton.connect('clicked(bool)', self.onRefitButton)
    self.referencePathsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.comparePathsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.referenceEndMarkupsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.compareEndMarkupsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.registrationComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.compareToInitialTransformComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.registerInitialButton.connect('clicked(bool)', self.onRegisterInitialButton)
    self.initialToReferenceTransformComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.registerICPButton.connect('clicked(bool)', self.onRegisterICPButton)
    self.compareToReferenceTransformComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.concatenateRegistrationButton.connect('clicked(bool)', self.onConcatenateRegistrationButton)
    self.outputDistancesTableComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.outputSummaryTableComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.computeButton.connect('clicked(bool)', self.onComputeButton)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Run button state
    self.onNodeChanged()

  def cleanup(self):
    pass

  def onNodeChanged(self):
    self.segmentExportButton.enabled = self.segmentationComboBox.currentNode()
    self.trimButton.enabled = self.preprocessPathsComboBox.currentNode()
    self.refitButton.enabled = self.preprocessPathsComboBox.currentNode()
    self.registerInitialButton.enabled = self.referencePathsComboBox.currentNode() and \
                                         self.comparePathsComboBox.currentNode()
    self.registerICPButton.enabled = self.referencePathsComboBox.currentNode() and \
                                     self.compareEndMarkupsComboBox.currentNode() and \
                                     self.comparePathsComboBox.currentNode() and \
                                     self.compareToInitialTransformComboBox.currentNode()
    self.concatenateRegistrationButton.enabled = self.comparePathsComboBox.currentNode() and \
                                                 self.compareEndMarkupsComboBox.currentNode() and \
                                                 self.compareToInitialTransformComboBox.currentNode() and \
                                                 self.initialToReferenceTransformComboBox.currentNode()
    self.computeButton.enabled = self.referencePathsComboBox.currentNode() and \
                                 self.comparePathsComboBox.currentNode() and \
                                 self.compareToReferenceTransformComboBox.currentNode()

  def onRegisterInitialButton(self):
    referencePathsNode = self.referencePathsComboBox.currentNode()
    comparePathsNode = self.comparePathsComboBox.currentNode()
    label = comparePathsNode.GetName()
    referenceEndMarkupsNode = self.referenceEndMarkupsComboBox.currentNode()
    if not referenceEndMarkupsNode:
      referenceEndMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      referenceEndMarkupsNode.SetName( label + "ReferenceEndMarkupsNode" )
      slicer.mrmlScene.AddNode(referenceEndMarkupsNode)
      self.referenceEndMarkupsComboBox.setCurrentNode(referenceEndMarkupsNode)
    compareEndMarkupsNode = self.compareEndMarkupsComboBox.currentNode()
    if not compareEndMarkupsNode:
      compareEndMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      compareEndMarkupsNode.SetName( label + "CompareEndMarkupsNode" )
      slicer.mrmlScene.AddNode(compareEndMarkupsNode)
      self.compareEndMarkupsComboBox.setCurrentNode(compareEndMarkupsNode)
    registrationNode = self.registrationComboBox.currentNode()
    if not registrationNode:
      registrationNode = slicer.vtkMRMLFiducialRegistrationWizardNode()
      registrationNode.SetName( label + "RegistrationNode" )
      registrationNode.SetUpdateModeToManual()
      registrationNode.SetPointMatchingToComputed()
      registrationNode.SetRegistrationModeToRigid()
      slicer.mrmlScene.AddNode(registrationNode)
      self.registrationComboBox.setCurrentNode(registrationNode)
    compareToInitialTransformNode = self.compareToInitialTransformComboBox.currentNode()
    if not compareToInitialTransformNode:
      compareToInitialTransformNode = slicer.vtkMRMLLinearTransformNode()
      compareToInitialTransformNode.SetName( label + "CompareToInitialTransformNode" )
      slicer.mrmlScene.AddNode(compareToInitialTransformNode)
      self.compareToInitialTransformComboBox.setCurrentNode(compareToInitialTransformNode)
    logic = PathVerificationLogic()
    logic.performRegistrationInitial(referencePathsNode, comparePathsNode, \
                                     referenceEndMarkupsNode, compareEndMarkupsNode, \
                                     registrationNode, compareToInitialTransformNode)

  def onRegisterICPButton(self):
    logging.debug('onRegisterICPButton')
    referencePathsNode = self.referencePathsComboBox.currentNode()
    comparePathsNode = self.comparePathsComboBox.currentNode()
    label = comparePathsNode.GetName()
    compareToInitialTransformNode = self.compareToInitialTransformComboBox.currentNode()
    initialToReferenceTransformNode = self.initialToReferenceTransformComboBox.currentNode()
    if not initialToReferenceTransformNode:
      initialToReferenceTransformNode = slicer.vtkMRMLLinearTransformNode()
      initialToReferenceTransformNode.SetName( label + "InitialToReferenceTransformNode" )
      slicer.mrmlScene.AddNode(initialToReferenceTransformNode)
      self.initialToReferenceTransformComboBox.setCurrentNode(initialToReferenceTransformNode)
    logic = PathVerificationLogic()
    logic.performRegistrationICP(referencePathsNode, comparePathsNode, \
                                 compareToInitialTransformNode, initialToReferenceTransformNode)

  def onConcatenateRegistrationButton(self):
    comparePathsNode = self.comparePathsComboBox.currentNode()
    label = comparePathsNode.GetName()
    compareEndMarkupsNode = self.compareEndMarkupsComboBox.currentNode()
    compareToInitialTransformNode = self.compareToInitialTransformComboBox.currentNode()
    initialToReferenceTransformNode = self.initialToReferenceTransformComboBox.currentNode()
    compareToReferenceTransformNode = self.compareToReferenceTransformComboBox.currentNode()
    if not compareToReferenceTransformNode:
      compareToReferenceTransformNode = slicer.vtkMRMLLinearTransformNode()
      compareToReferenceTransformNode.SetName( label + "CompareToReferenceTransformNode" )
      slicer.mrmlScene.AddNode(compareToReferenceTransformNode)
      self.compareToReferenceTransformComboBox.setCurrentNode(compareToReferenceTransformNode)
    logic = PathVerificationLogic()
    logic.concatenateRegistration(comparePathsNode, compareEndMarkupsNode, \
                                  compareToInitialTransformNode, initialToReferenceTransformNode, \
                                  compareToReferenceTransformNode)

  def onComputeButton(self):
    referencePathsNode = self.referencePathsComboBox.currentNode()
    comparePathsNode = self.comparePathsComboBox.currentNode()
    compareToReferenceTransformNode = self.compareToReferenceTransformComboBox.currentNode()
    label = comparePathsNode.GetName()
    outputSummaryTableNode = self.outputSummaryTableComboBox.currentNode()
    if not outputSummaryTableNode:
      outputSummaryTableNode = slicer.vtkMRMLTableNode()
      outputSummaryTableNode.SetName( label + "Summary" )
      slicer.mrmlScene.AddNode(outputSummaryTableNode)
      self.outputSummaryTableComboBox.setCurrentNode(outputSummaryTableNode)
    outputDistancesTableNode = self.outputDistancesTableComboBox.currentNode()
    if not outputDistancesTableNode:
      outputDistancesTableNode = slicer.vtkMRMLTableNode()
      outputDistancesTableNode.SetName( label + "Distances" )
      slicer.mrmlScene.AddNode(outputDistancesTableNode)
      self.outputDistancesTableComboBox.setCurrentNode(outputDistancesTableNode)
    logic = PathVerificationLogic()
    logic.computeStatistics(referencePathsNode, comparePathsNode, \
                            compareToReferenceTransformNode, \
                            outputDistancesTableNode, outputSummaryTableNode)

  def onSegmentExportButton(self):
    segmentationNode = self.segmentationComboBox.currentNode()
    segmentationPaths = self.preprocessPathsComboBox.currentNode()
    if not segmentationPaths:
      segmentationPaths = slicer.vtkMRMLPathReconstructionNode()
      segmentationPaths.SetName("CTPaths")
      slicer.mrmlScene.AddNode(segmentationPaths)
      self.preprocessPathsComboBox.setCurrentNode(segmentationPaths)
    logic = PathVerificationLogic()
    logic.exportSegmentsToPath(segmentationNode, segmentationPaths)

  def onTrimButton(self):
    trimPathsNode = self.preprocessPathsComboBox.currentNode()
    nearTrimDistance = self.nearTrimAmountSpinBox.value
    farTrimDistance = self.farTrimAmountSpinBox.value
    logic = PathVerificationLogic()
    logic.trimPoints( trimPathsNode, nearTrimDistance, farTrimDistance )
    self.nearTrimAmountSpinBox.setValue( 0 ) # avoid trimming twice by accident (e.g. double click)
    self.farTrimAmountSpinBox.setValue( 0 )

  def onRefitButton(self):
    trimPathsNode = self.preprocessPathsComboBox.currentNode()
    logic = PathVerificationLogic()
    logic.refitPath( trimPathsNode )

#
# PathVerificationLogic
#

class PathVerificationLogic(ScriptedLoadableModuleLogic):
  """
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def exportSegmentsToPath( self, segmentationNode, pathsNode ):
    segmentationsLogic = slicer.modules.segmentations.logic()
    modelHierarchyNode = slicer.vtkMRMLModelHierarchyNode()
    slicer.mrmlScene.AddNode( modelHierarchyNode )
    modelHierarchyName = pathsNode.GetName() + "_Models"
    modelHierarchyNode.SetName( modelHierarchyName )
    segmentationsLogic.ExportVisibleSegmentsToModelHierarchy( segmentationNode, modelHierarchyNode )

    if not pathsNode.GetMarkupsToModelNode():
      markupsToModelNode = slicer.vtkMRMLMarkupsToModelNode()
      slicer.mrmlScene.AddNode( markupsToModelNode )
      pathsNode.SetAndObserveMarkupsToModelNodeID( markupsToModelNode.GetID() )

    numberOfCatheters = modelHierarchyNode.GetNumberOfChildrenNodes()
    for catheterIndex in xrange( 0, numberOfCatheters ):
      catheterPointsNode = modelHierarchyNode.GetNthChildNode( catheterIndex ).GetModelNode()
      catheterPointsNode.SetName( pathsNode.GetName() + "_CatheterPoints" + str( catheterIndex ) )
      catheterPathNode = slicer.vtkMRMLModelNode()
      slicer.mrmlScene.AddNode( catheterPathNode )
      catheterPathNode.SetName( pathsNode.GetName() + "_CatheterPath" + str( catheterIndex ) )
      catheterPathNode.CreateDefaultDisplayNodes()
      pathsNode.AddPointsPathPairModelNodeIDs( catheterPointsNode.GetID(), catheterPathNode.GetID() )

  def trimPoints( self, pathsNode, nearTrimDistance, farTrimDistance ):
    if not pathsNode:
      logging.error("Trim paths node is null.")
      return False

    # Iterate through each path
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    if numberOfSuffixes <= 0:
      logging.warning("There are no paths")
      return
    
    # Record the direction of each catheter
    directions = {}
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pointsModelNode = pathsNode.GetPointsModelNodeBySuffix( suffix )
      points = pointsModelNode.GetPolyData().GetPoints()
      # find farthest two points
      firstPoint = points.GetPoint( 0 )
      lastPoint = points.GetPoint( points.GetNumberOfPoints() - 1 )
      direction = [ 0, 0, 0 ]
      direction[ 0 ] = lastPoint[ 0 ] - firstPoint[ 0 ]
      direction[ 1 ] = lastPoint[ 1 ] - firstPoint[ 1 ]
      direction[ 2 ] = lastPoint[ 2 ] - firstPoint[ 2 ]
      directions[ suffix ] = direction

    # Determine the 'average' direction of catetheters
    sumOfDirections = [ 0, 0, 0 ]
    firstDirection = [ 0, 0, 0 ]
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      direction = directions[ suffix ]
      if suffixIndex == 0:
        firstDirection = direction
      invertDirection = ( vtk.vtkMath.Dot( firstDirection, direction ) > 0 )
      if invertDirection == True:
        sumOfDirections[ 0 ] -= direction[ 0 ]
        sumOfDirections[ 1 ] -= direction[ 1 ]
        sumOfDirections[ 2 ] -= direction[ 2 ]
      else:
        sumOfDirections[ 0 ] += direction[ 0 ]
        sumOfDirections[ 1 ] += direction[ 1 ]
        sumOfDirections[ 2 ] += direction[ 2 ]
    averageDirection = [ 0, 0, 0 ]
    averageDirection[ 0 ] = sumOfDirections[ 0 ] / numberOfSuffixes
    averageDirection[ 1 ] = sumOfDirections[ 1 ] / numberOfSuffixes
    averageDirection[ 2 ] = sumOfDirections[ 2 ] / numberOfSuffixes
    vtk.vtkMath.Normalize( averageDirection )

    # Determine threshold values for filtering out points
    shortestMinimumAlongAverageDirection = float( "-inf" )
    shortestMaximumAlongAverageDirection = float( "inf" )
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pointsModelNode = pathsNode.GetPointsModelNodeBySuffix( suffix )
      points = pointsModelNode.GetPolyData().GetPoints()
      numberOfPoints = points.GetNumberOfPoints()
      minimumAlongAverageDirection = float( "inf" )
      maximumAlongAverageDirection = float( "-inf" )
      for pointIndex in xrange( 0, numberOfPoints ):
        point = points.GetPoint( pointIndex )
        pointAlongAverageDirection = vtk.vtkMath.Dot( point, averageDirection )
        if pointAlongAverageDirection < minimumAlongAverageDirection:
          minimumAlongAverageDirection = pointAlongAverageDirection
        if pointAlongAverageDirection > maximumAlongAverageDirection:
          maximumAlongAverageDirection = pointAlongAverageDirection
      if minimumAlongAverageDirection > shortestMinimumAlongAverageDirection:
        shortestMinimumAlongAverageDirection = minimumAlongAverageDirection
      if maximumAlongAverageDirection < shortestMaximumAlongAverageDirection:
        shortestMaximumAlongAverageDirection = maximumAlongAverageDirection
    shortestMinimumAlongAverageDirection += nearTrimDistance
    shortestMaximumAlongAverageDirection -= farTrimDistance
    
    # Do the actual filtering
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pointsModelNode = pathsNode.GetPointsModelNodeBySuffix( suffix )
      points = pointsModelNode.GetPolyData().GetPoints()
      numberOfPoints = points.GetNumberOfPoints()
      # make new list of points that are within the non-trimmed region
      newPoints = vtk.vtkPoints()
      newVerts = vtk.vtkCellArray()
      for pointIndex in xrange( 0, numberOfPoints ):
        point = points.GetPoint( pointIndex )
        pointAlongAverageDirection = vtk.vtkMath.Dot( point, averageDirection )
        if pointAlongAverageDirection < shortestMinimumAlongAverageDirection:
          continue
        if pointAlongAverageDirection > shortestMaximumAlongAverageDirection:
          continue
        newPoints.InsertNextPoint( point )
        newPointIndex = newPoints.GetNumberOfPoints() - 1
        newVerts.InsertNextCell( 1, [ newPointIndex ] )
      # assign to points model node
      newPolyData = vtk.vtkPolyData()
      newPolyData.SetVerts( newVerts )
      newPolyData.SetPoints( newPoints )
      pointsModelNode.SetAndObservePolyData( newPolyData )
    return True
  
  def refitPath( self, pathNode ):
    markupsToModelNode = pathNode.GetMarkupsToModelNode()
    markupsToModelNode.SetModelType( slicer.vtkMRMLMarkupsToModelNode.Curve )
    markupsToModelNode.SetCurveType( slicer.vtkMRMLMarkupsToModelNode.Polynomial )
    markupsToModelNode.SetPolynomialFitType( slicer.vtkMRMLMarkupsToModelNode.MovingLeastSquares )
    markupsToModelNode.SetPolynomialOrder( 1 )
    markupsToModelNode.SetPolynomialSampleWidth( 0.25 )
    markupsToModelNode.SetPointParameterType( slicer.vtkMRMLMarkupsToModelNode.MinimumSpanningTree )
    markupsToModelNode.SetPolynomialWeightType( slicer.vtkMRMLMarkupsToModelNode.Gaussian )
    markupsToModelNode.SetTubeRadius( 0.01 )
    markupsToModelNode.SetTubeSegmentsBetweenControlPoints( 1 )
    markupsToModelNode.SetTubeNumberOfSides( 4 )
    slicer.modules.pathreconstruction.logic().RefitAllPaths( pathNode )
  
  def registerPaths(self, referencePathsNode, comparePathsNode, \
                          referenceEndMarkupsNode, compareEndMarkupsNode, \
                          registrationNode, compareToReferenceLinearTransformNode):

    # make compare paths observe the registration transform node. This is purely cosmetic, does not touch underlying data

    return True

  def computeCombinePolyData( self, pathsNode, combinedPolyData ):
    appender = vtk.vtkAppendPolyData()
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pathModelNode = pathsNode.GetPathModelNodeBySuffix( suffix )
      polyData = pathModelNode.GetPolyData()
      appender.AddInputData( polyData )
    appender.Update()
    combinedPolyData.DeepCopy( appender.GetOutput() )

  def computeEndMarkups( self, pathsNode, endMarkupsNode ):
    endMarkupsNode.RemoveAllMarkups()
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pathModelNode = pathsNode.GetPathModelNodeBySuffix( suffix )
      polyData = pathModelNode.GetPolyData()
      points = polyData.GetPoints()
      firstPoint = points.GetPoint( 0 )
      endMarkupsNode.AddFiducialFromArray( firstPoint )
      numberOfPoints = points.GetNumberOfPoints()
      lastPoint = points.GetPoint( numberOfPoints - 1 )
      endMarkupsNode.AddFiducialFromArray( lastPoint )

  def performRegistrationInitial( self, referencePathsNode, comparePathsNode, \
                                  referenceEndMarkupsNode, compareEndMarkupsNode, \
                                  registrationNode, compareToInitialTransformNode ):
    if not referencePathsNode or \
       not comparePathsNode or \
       not referenceEndMarkupsNode or \
       not compareEndMarkupsNode or \
       not registrationNode or \
       not compareToInitialTransformNode:
      logging.error("A node is null. Check nodes for null.")
      return False

    self.computeEndMarkups( referencePathsNode, referenceEndMarkupsNode )
    self.computeEndMarkups( comparePathsNode, compareEndMarkupsNode )
    
    # perform an initial registration based on the endpoints of the catheters
    # transformNode is used as a container here to store an intermediate result
    # The final result is computed later on
    registrationNode.SetAndObserveFromFiducialListNodeId( compareEndMarkupsNode.GetID() )
    registrationNode.SetAndObserveToFiducialListNodeId( referenceEndMarkupsNode.GetID() )
    registrationNode.SetOutputTransformNodeId( compareToInitialTransformNode.GetID() )
    registrationLogic = slicer.modules.fiducialregistrationwizard.logic()
    registrationLogic.UpdateCalibration( registrationNode )

    # Update transform hierarchy
    self.setCompareDataToObserveRegistration( comparePathsNode, compareEndMarkupsNode, compareToInitialTransformNode )

    return True

  def performRegistrationICP( self, referencePathsNode, comparePathsNode, \
                              compareToInitialTransformNode, initialToReferenceTransformNode ):
    logging.debug('performRegistrationICP')

    if not referencePathsNode or \
       not comparePathsNode or \
       not compareToInitialTransformNode or \
       not initialToReferenceTransformNode:
      logging.error("A node is null. Check nodes for null.")
      return False

    referenceCombinedPolyData = vtk.vtkPolyData()
    self.computeCombinePolyData( referencePathsNode, referenceCombinedPolyData )
    compareCombinedPolyData = vtk.vtkPolyData()
    self.computeCombinePolyData( comparePathsNode, compareCombinedPolyData )

    # transform is preliminary, only contains the initial alignment
    # still need to run ICP on the combined model pairs
    initialAlignmentTransform = compareToInitialTransformNode.GetTransformToParent()
    initialAlignmentTransformFilter = vtk.vtkTransformPolyDataFilter()
    initialAlignmentTransformFilter.SetTransform( initialAlignmentTransform )
    initialAlignmentTransformFilter.SetInputData( compareCombinedPolyData )
    initialAlignmentTransformFilter.Update()
    initiallyAlignedCompareCombinedPolyData = initialAlignmentTransformFilter.GetOutput()
    icpTransform = vtk.vtkIterativeClosestPointTransform()
    icpTransform.SetSource( initiallyAlignedCompareCombinedPolyData )
    icpTransform.SetTarget( referenceCombinedPolyData )
    icpTransform.GetLandmarkTransform().SetModeToRigidBody()
    icpTransform.Update()

    icpMatrix = vtk.vtkMatrix4x4()
    icpTransform.GetMatrix( icpMatrix )
    initialToReferenceTransformNode.SetMatrixTransformToParent( icpMatrix )

    # Update transform hierarchy
    compareToInitialTransformNode.SetAndObserveTransformNodeID( initialToReferenceTransformNode.GetID() )

    return True

  def concatenateRegistration( self, comparePathsNode, compareEndMarkupsNode, \
                               compareToInitialTransformNode, initialToReferenceTransformNode, \
                               compareToReferenceTransformNode ):
    initialAlignmentMatrix = vtk.vtkMatrix4x4()
    compareToInitialTransformNode.GetMatrixTransformToParent( initialAlignmentMatrix )
    icpMatrix = vtk.vtkMatrix4x4()
    initialToReferenceTransformNode.GetMatrixTransformToParent( icpMatrix )
    concatenatedTransform = vtk.vtkTransform()
    concatenatedTransform.PostMultiply()
    concatenatedTransform.Concatenate( initialAlignmentMatrix )
    concatenatedTransform.Concatenate( icpMatrix )
    compareToReferenceTransformNode.SetAndObserveTransformToParent( concatenatedTransform )

    # Update transfrom hierarchy
    self.setCompareDataToObserveRegistration( comparePathsNode, compareEndMarkupsNode, compareToReferenceTransformNode )

    return True

  def setCompareDataToObserveRegistration( self, comparePathsNode, compareEndMarkupsNode, transformNode ):
    compareEndMarkupsNode.SetAndObserveTransformNodeID( transformNode.GetID() )
    compareSuffixArray = vtk.vtkIntArray()
    comparePathsNode.GetSuffixes( compareSuffixArray )
    numberOfCompareSuffixes = compareSuffixArray.GetNumberOfTuples()
    for compareSuffixIndex in xrange( 0, numberOfCompareSuffixes ):
      compareSuffix = int(compareSuffixArray.GetComponent( compareSuffixIndex, 0 ) )
      comparePathModelNode = comparePathsNode.GetPathModelNodeBySuffix( compareSuffix )
      comparePathModelNode.SetAndObserveTransformNodeID( transformNode.GetID() )
      comparePointsModelNode = comparePathsNode.GetPointsModelNodeBySuffix( compareSuffix )
      comparePointsModelNode.SetAndObserveTransformNodeID( transformNode.GetID() )
  
  def computeStatistics( self, referencePathsNode, comparePathsNode, \
                               compareToReferenceLinearTransformNode, \
                               outputDistancesTableNode, outputSummaryTableNode ):
    # set up the arrays that will go into the output table nodes
    # these are for raw distances
    distancesLabelArray = vtk.vtkStringArray()
    distancesLabelArray.SetName( "Label" )
    distancesSuffixArray = vtk.vtkIntArray()
    distancesSuffixArray.SetName( "Suffix" )
    distancesValueArray = vtk.vtkDoubleArray()
    distancesValueArray.SetName( "Distance" )
    # these are for summary statistics
    summaryLabelArray = vtk.vtkStringArray()
    summaryLabelArray.SetName( "Label" )
    summarySuffixArray = vtk.vtkIntArray()
    summarySuffixArray.SetName( "Suffix" )
    summaryLengthArray = vtk.vtkDoubleArray()
    summaryLengthArray.SetName( "Length" )
    summaryInputPointCountArray = vtk.vtkIntArray()
    summaryInputPointCountArray.SetName( "Point Count" )
    summaryDistanceCountArray = vtk.vtkIntArray()
    summaryDistanceCountArray.SetName( "Distance Count" )
    summaryMeanArray = vtk.vtkDoubleArray()
    summaryMeanArray.SetName( "Mean" )
    summaryStdevArray = vtk.vtkDoubleArray()
    summaryStdevArray.SetName( "Stdev" )
    summaryPercentile000Array = vtk.vtkDoubleArray()
    summaryPercentile000Array.SetName( "0th Percentile")
    summaryPercentile005Array = vtk.vtkDoubleArray()
    summaryPercentile005Array.SetName( "5th Percentile")
    summaryPercentile025Array = vtk.vtkDoubleArray()
    summaryPercentile025Array.SetName( "25th Percentile")
    summaryPercentile050Array = vtk.vtkDoubleArray()
    summaryPercentile050Array.SetName( "50th Percentile")
    summaryPercentile075Array = vtk.vtkDoubleArray()
    summaryPercentile075Array.SetName( "75th Percentile")
    summaryPercentile095Array = vtk.vtkDoubleArray()
    summaryPercentile095Array.SetName( "95th Percentile")
    summaryPercentile100Array = vtk.vtkDoubleArray()
    summaryPercentile100Array.SetName( "100th Percentile")
    summaryAngleDifferenceDegrees = vtk.vtkDoubleArray()
    summaryAngleDifferenceDegrees.SetName( "Angle Difference (Degrees)")

    # compute them
    comparePathsName = comparePathsNode.GetName() # used as a general text label in the outputs
    compareSuffixArray = vtk.vtkIntArray()
    comparePathsNode.GetSuffixes( compareSuffixArray )
    numberOfCompareSuffixes = compareSuffixArray.GetNumberOfTuples()
    for compareSuffixIndex in xrange( 0, numberOfCompareSuffixes ):
      compareSuffix = int(compareSuffixArray.GetComponent( compareSuffixIndex, 0 ) )
      comparePathModelNode = comparePathsNode.GetPathModelNodeBySuffix( compareSuffix )
      unregisteredComparePathPolyData = comparePathModelNode.GetPolyData()
      compareToReferenceTransformFilter = vtk.vtkTransformPolyDataFilter()
      compareToReferenceTransformFilter.SetTransform( compareToReferenceLinearTransformNode.GetTransformToParent() )
      compareToReferenceTransformFilter.SetInputData( unregisteredComparePathPolyData )
      compareToReferenceTransformFilter.Update()
      registeredComparePathPolyData = compareToReferenceTransformFilter.GetOutput()

      # Find the corresponding path node
      correspondingReferencePathPolyData = None
      distanceToCorrespondingPathNode = float("inf")
      comparePathCenterOfMassFilter = vtk.vtkCenterOfMass()
      comparePathCenterOfMassFilter.SetInputData( registeredComparePathPolyData )
      comparePathCenterOfMassFilter.Update()
      comparePathCenterOfMass = comparePathCenterOfMassFilter.GetCenter()
      referenceSuffixArray = vtk.vtkIntArray()
      referencePathsNode.GetSuffixes( referenceSuffixArray )
      numberOfReferenceSuffixes = referenceSuffixArray.GetNumberOfTuples()
      for referenceSuffixIndex in xrange( 0, numberOfReferenceSuffixes ):
        referenceSuffix = int(referenceSuffixArray.GetComponent( referenceSuffixIndex, 0 ) )
        referencePathModelNode = referencePathsNode.GetPathModelNodeBySuffix( referenceSuffix )
        referencePathCenterOfMassFilter = vtk.vtkCenterOfMass()
        referencePathPolyData = referencePathModelNode.GetPolyData()
        referencePathCenterOfMassFilter.SetInputData( referencePathModelNode.GetPolyData() )
        referencePathCenterOfMassFilter.Update()
        referencePathCenterOfMass = referencePathCenterOfMassFilter.GetCenter()
        distanceToReferencePath = vtk.vtkMath.Distance2BetweenPoints( comparePathCenterOfMass, referencePathCenterOfMass )
        if ( distanceToReferencePath < distanceToCorrespondingPathNode ):
          distanceToCorrespondingPathNode = distanceToReferencePath
          correspondingReferencePathPolyData = referencePathPolyData

      # Measure distances between the two paths
      polyDataDistanceFilter = slicer.vtkPolyDataDistanceHistogramFilter()
      polyDataDistanceFilter.SetInputComparePolyData( registeredComparePathPolyData )
      polyDataDistanceFilter.SetInputReferencePolyData( correspondingReferencePathPolyData )
      polyDataDistanceFilter.Update()

      rawDistances = polyDataDistanceFilter.GetOutputDistances()
      numberOfRawDistances = rawDistances.GetNumberOfTuples()
      for rawDistanceIndex in xrange( 0, numberOfRawDistances ):
        distancesLabelArray.InsertNextValue( comparePathsName )
        distancesSuffixArray.InsertNextTuple1( compareSuffix )
        rawDistance = rawDistances.GetComponent( rawDistanceIndex, 0 )
        distancesValueArray.InsertNextTuple1( rawDistance )

      # Measure angle difference
      comparePolyDataPoints = registeredComparePathPolyData.GetPoints()
      comparePathFirstPoint = comparePolyDataPoints.GetPoint(0)
      comparePathLastPoint = comparePolyDataPoints.GetPoint(comparePolyDataPoints.GetNumberOfPoints()-1)
      comparePathDirection = [ 0, 0, 0 ]
      comparePathDirection[ 0 ] = comparePathLastPoint[ 0 ] - comparePathFirstPoint[ 0 ]
      comparePathDirection[ 1 ] = comparePathLastPoint[ 1 ] - comparePathFirstPoint[ 1 ]
      comparePathDirection[ 2 ] = comparePathLastPoint[ 2 ] - comparePathFirstPoint[ 2 ]
      referencePolyDataPoints = correspondingReferencePathPolyData.GetPoints()
      referencePathFirstPoint = referencePolyDataPoints.GetPoint(0)
      referencePathLastPoint = referencePolyDataPoints.GetPoint(referencePolyDataPoints.GetNumberOfPoints()-1)
      referencePathDirection = [ 0, 0, 0 ]
      referencePathDirection[ 0 ] = referencePathLastPoint[ 0 ] - referencePathFirstPoint[ 0 ]
      referencePathDirection[ 1 ] = referencePathLastPoint[ 1 ] - referencePathFirstPoint[ 1 ]
      referencePathDirection[ 2 ] = referencePathLastPoint[ 2 ] - referencePathFirstPoint[ 2 ]
      angleDifferenceRadians = vtk.vtkMath.AngleBetweenVectors(comparePathDirection,referencePathDirection)
      angleDifferenceDegrees = angleDifferenceRadians * 180 / vtk.vtkMath.Pi()

      summaryLabelArray.InsertNextValue( comparePathsName )
      summarySuffixArray.InsertNextTuple1( compareSuffix )
      catheterLength = float( comparePathModelNode.GetAttribute( slicer.vtkMRMLMarkupsToModelNode.GetOutputCurveLengthAttributeName() ) )
      summaryLengthArray.InsertNextTuple1( catheterLength )
      comparePointsModelNode = comparePathsNode.GetPointsModelNodeBySuffix( compareSuffix )
      numberOfPoints = comparePointsModelNode.GetPolyData().GetPoints().GetNumberOfPoints()
      summaryInputPointCountArray.InsertNextTuple1( numberOfPoints )
      summaryDistanceCountArray.InsertNextTuple1( numberOfRawDistances / 4 )
      summaryMeanArray.InsertNextTuple1( polyDataDistanceFilter.GetAverageHausdorffDistance() )
      summaryStdevArray.InsertNextTuple1( polyDataDistanceFilter.GetStandardDeviationHausdorffDistance() )
      summaryPercentile000Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 0 ) )
      summaryPercentile005Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 5 ) )
      summaryPercentile025Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 25 ) )
      summaryPercentile050Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 50 ) )
      summaryPercentile075Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 75 ) )
      summaryPercentile095Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 95 ) )
      summaryPercentile100Array.InsertNextTuple1( polyDataDistanceFilter.GetNthPercentileHausdorffDistance( 100 ) )
      summaryAngleDifferenceDegrees.InsertNextTuple1( angleDifferenceDegrees )
      # Label, catheter suffix, mean distance, stdev, 0th, 5th, 25th, 50th, 75th, 95th, 100th, angle difference

    # Assign the arrays to the table nodes
    outputDistancesTableNode.RemoveAllColumns()
    outputDistancesTableNode.AddColumn( distancesLabelArray )
    outputDistancesTableNode.AddColumn( distancesSuffixArray )
    outputDistancesTableNode.AddColumn( distancesValueArray )
    outputSummaryTableNode.RemoveAllColumns()
    outputSummaryTableNode.AddColumn( summaryLabelArray )
    outputSummaryTableNode.AddColumn( summarySuffixArray )
    outputSummaryTableNode.AddColumn( summaryLengthArray )
    outputSummaryTableNode.AddColumn( summaryInputPointCountArray )
    outputSummaryTableNode.AddColumn( summaryDistanceCountArray )
    outputSummaryTableNode.AddColumn( summaryMeanArray )
    outputSummaryTableNode.AddColumn( summaryStdevArray )
    outputSummaryTableNode.AddColumn( summaryPercentile000Array )
    outputSummaryTableNode.AddColumn( summaryPercentile005Array )
    outputSummaryTableNode.AddColumn( summaryPercentile025Array )
    outputSummaryTableNode.AddColumn( summaryPercentile050Array )
    outputSummaryTableNode.AddColumn( summaryPercentile075Array )
    outputSummaryTableNode.AddColumn( summaryPercentile095Array )
    outputSummaryTableNode.AddColumn( summaryPercentile100Array )
    outputSummaryTableNode.AddColumn( summaryAngleDifferenceDegrees )

class PathVerificationTest(ScriptedLoadableModuleTest):
  """
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    self.setUp()
    self.test_PathVerification1()

  def test_PathVerification1(self):
    pass


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

    self.trimAmountSpinBox = ctk.ctkDoubleSpinBox()
    self.trimAmountSpinBox.setDecimals(1)
    self.trimAmountSpinBox.setValue(0)
    self.trimAmountSpinBox.minimum = 0
    self.trimAmountSpinBox.maximum = 100
    self.trimAmountSpinBox.singleStep = 1
    preprocessingFormLayout.addRow("Trim Amount (mm): ", self.trimAmountSpinBox)

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

    self.registerButton = qt.QPushButton("Register")
    self.registerButton.enabled = False
    verificationFormLayout.addRow(self.registerButton)

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
    self.compareToReferenceTransformComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.registerButton.connect('clicked(bool)', self.onRegisterButton)
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
    self.registerButton.enabled = self.referencePathsComboBox.currentNode() and \
                                  self.comparePathsComboBox.currentNode()
    self.computeButton.enabled = self.referencePathsComboBox.currentNode() and \
                                 self.comparePathsComboBox.currentNode() and \
                                 self.compareToReferenceTransformComboBox.currentNode()

  def onRegisterButton(self):
    referencePathsNode = self.referencePathsComboBox.currentNode()
    comparePathsNode = self.comparePathsComboBox.currentNode()
    referenceEndMarkupsNode = self.referenceEndMarkupsComboBox.currentNode()
    if not referenceEndMarkupsNode:
      referenceEndMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      referenceEndMarkupsNode.SetName("referenceEndMarkupsNode")
      slicer.mrmlScene.AddNode(referenceEndMarkupsNode)
      self.referenceEndMarkupsComboBox.setCurrentNode(referenceEndMarkupsNode)
    compareEndMarkupsNode = self.compareEndMarkupsComboBox.currentNode()
    if not compareEndMarkupsNode:
      compareEndMarkupsNode = slicer.vtkMRMLMarkupsFiducialNode()
      compareEndMarkupsNode.SetName("compareEndMarkupsNode")
      slicer.mrmlScene.AddNode(compareEndMarkupsNode)
      self.compareEndMarkupsComboBox.setCurrentNode(compareEndMarkupsNode)
    registrationNode = self.registrationComboBox.currentNode()
    if not registrationNode:
      registrationNode = slicer.vtkMRMLFiducialRegistrationWizardNode()
      registrationNode.SetName("registrationNode")
      slicer.mrmlScene.AddNode(registrationNode)
      self.registrationComboBox.setCurrentNode(registrationNode)
    compareToReferenceTransformNode = self.compareToReferenceTransformComboBox.currentNode()
    if not compareToReferenceTransformNode:
      compareToReferenceTransformNode = slicer.vtkMRMLLinearTransformNode()
      compareToReferenceTransformNode.SetName("compareToReferenceTransformNode")
      slicer.mrmlScene.AddNode(compareToReferenceTransformNode)
      self.compareToReferenceTransformComboBox.setCurrentNode(compareToReferenceTransformNode)
    logic = PathVerificationLogic()
    logic.registerPaths(referencePathsNode, comparePathsNode, \
                        referenceEndMarkupsNode, compareEndMarkupsNode, \
                        registrationNode, compareToReferenceTransformNode)

  def onComputeButton(self):
    referencePathsNode = self.referencePathsComboBox.currentNode()
    comparePathsNode = self.comparePathsComboBox.currentNode()
    compareToReferenceTransformNode = self.compareToReferenceTransformComboBox.currentNode()
    outputSummaryTableNode = self.outputSummaryTableComboBox.currentNode()
    if not outputSummaryTableNode:
      outputSummaryTableNode = slicer.vtkMRMLTableNode()
      outputSummaryTableNode.SetName("outputSummaryTableNode")
      slicer.mrmlScene.AddNode(outputSummaryTableNode)
      self.outputSummaryTableComboBox.setCurrentNode(outputSummaryTableNode)
    outputDistancesTableNode = self.outputDistancesTableComboBox.currentNode()
    if not outputDistancesTableNode:
      outputDistancesTableNode = slicer.vtkMRMLTableNode()
      outputDistancesTableNode.SetName("outputDistancesTableNode")
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
    trimDistance = self.trimAmountSpinBox.value
    logic = PathVerificationLogic()
    logic.trimPoints( trimPathsNode, trimDistance )
    self.trimAmountSpinBox.setValue( 0 ) # avoid trimming twice by accident (e.g. double click)

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

  def trimPoints( self, pathsNode, trimDistance ):
    if not pathsNode:
      logging.error("Trim paths node is null.")
      return False

    # Iterate through each path
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    for suffixIndex in xrange( 0, numberOfSuffixes ):
      suffix = int(suffixArray.GetComponent( suffixIndex, 0 ) )
      pointsModelNode = pathsNode.GetPointsModelNodeBySuffix( suffix )
      points = pointsModelNode.GetPolyData().GetPoints()
      # find farthest two points
      farthestPoint1 = [0, 0, 0]
      farthestPoint2 = [0, 0, 0]
      farthestDistance2 = 0
      numberOfPoints = points.GetNumberOfPoints()
      for pointIndex1 in xrange( 0, numberOfPoints ):
        point1 = points.GetPoint( pointIndex1 )
        for pointIndex2 in xrange( pointIndex1 + 1, numberOfPoints ):
          point2 = points.GetPoint( pointIndex2 )
          distance2 = vtk.vtkMath.Distance2BetweenPoints( point1, point2 )
          if ( distance2 > farthestDistance2 ):
            farthestDistance2 = distance2
            farthestPoint1 = point1
            farthestPoint2 = point2
      # make new list of points that are at least trimDistance away
      newPoints = vtk.vtkPoints()
      newVerts = vtk.vtkCellArray()
      trimDistance2 = trimDistance * trimDistance
      for pointIndex in xrange( 0, numberOfPoints ):
        point = points.GetPoint( pointIndex )
        distance2 = vtk.vtkMath.Distance2BetweenPoints( farthestPoint1, point )
        if distance2 < trimDistance2:
          continue
        distance2 = vtk.vtkMath.Distance2BetweenPoints( farthestPoint2, point )
        if distance2 < trimDistance2:
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
    if not referencePathsNode or \
       not comparePathsNode or \
       not referenceEndMarkupsNode or \
       not compareEndMarkupsNode or \
       not registrationNode or \
       not compareToReferenceLinearTransformNode:
      logging.error("A node is null. Check nodes for null.")
      return False

    # align catheter paths
    referenceCombinedPolyData = vtk.vtkPolyData()
    self.computeCombinePolyData( referencePathsNode, referenceCombinedPolyData )
    self.computeEndMarkups( referencePathsNode, referenceEndMarkupsNode )
    compareCombinedPolyData = vtk.vtkPolyData()
    self.computeCombinePolyData( comparePathsNode, compareCombinedPolyData )
    self.computeEndMarkups( comparePathsNode, compareEndMarkupsNode )
    self.performRegistration( referenceEndMarkupsNode, compareEndMarkupsNode, \
                              referenceCombinedPolyData, compareCombinedPolyData,
                              registrationNode, compareToReferenceLinearTransformNode )

    # make compare paths observe the registration transform node. This is purely cosmetic, does not touch underlying data
    self.setCompareDataToObserveRegistration( comparePathsNode, compareEndMarkupsNode, compareToReferenceLinearTransformNode )

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

  def performRegistration( self, referenceEndMarkupsNode, compareEndMarkupsNode, \
                                 referenceCombinedPolyData, compareCombinedPolyData, \
                                 registrationNode, transformNode ):
    # perform an initial registration based on the endpoints of the catheters
    # transformNode is used as a container here to store an intermediate result
    # The final result is computed later on
    registrationNode.SetUpdateModeToManual()
    registrationNode.SetPointMatchingToComputed()
    registrationNode.SetRegistrationModeToRigid()
    registrationNode.SetAndObserveFromFiducialListNodeId( compareEndMarkupsNode.GetID() )
    registrationNode.SetAndObserveToFiducialListNodeId( referenceEndMarkupsNode.GetID() )
    registrationNode.SetOutputTransformNodeId( transformNode.GetID() )
    registrationLogic = slicer.modules.fiducialregistrationwizard.logic()
    registrationLogic.UpdateCalibration( registrationNode )

    # transform is preliminary, only contains the initial alignment
    # still need to run ICP on the combined model pairs
    initialAlignmentTransform = transformNode.GetTransformToParent()
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

    # concatenate the initial transform with the icp
    icpMatrix = vtk.vtkMatrix4x4()
    icpTransform.GetMatrix( icpMatrix )
    initialAlignmentMatrix = vtk.vtkMatrix4x4()
    transformNode.GetMatrixTransformToParent( initialAlignmentMatrix )
    concatenatedTransform = vtk.vtkTransform()
    concatenatedTransform.PostMultiply()
    concatenatedTransform.Concatenate( initialAlignmentMatrix )
    concatenatedTransform.Concatenate( icpMatrix )

    # Update transformNode to store the final concatenated matrix
    transformNode.SetAndObserveTransformToParent( concatenatedTransform )

  def setCompareDataToObserveRegistration( self, comparePathsNode, compareEndMarkupsNode, compareToReferenceLinearTransformNode ):
    compareEndMarkupsNode.SetAndObserveTransformNodeID( compareToReferenceLinearTransformNode.GetID() )
    compareSuffixArray = vtk.vtkIntArray()
    comparePathsNode.GetSuffixes( compareSuffixArray )
    numberOfCompareSuffixes = compareSuffixArray.GetNumberOfTuples()
    for compareSuffixIndex in xrange( 0, numberOfCompareSuffixes ):
      compareSuffix = int(compareSuffixArray.GetComponent( compareSuffixIndex, 0 ) )
      comparePathModelNode = comparePathsNode.GetPathModelNodeBySuffix( compareSuffix )
      comparePathModelNode.SetAndObserveTransformNodeID( compareToReferenceLinearTransformNode.GetID() )
      comparePointsModelNode = comparePathsNode.GetPointsModelNodeBySuffix( compareSuffix )
      comparePointsModelNode.SetAndObserveTransformNodeID( compareToReferenceLinearTransformNode.GetID() )
  
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
      # Label, catheter suffix, mean distance, stdev, 0th, 5th, 25th, 50th, 75th, 95th, 100th

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


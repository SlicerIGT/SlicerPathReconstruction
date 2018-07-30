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

    self.moduleWarning = qt.QLabel("Note: Only initial registration works for now")
    self.layout.addWidget(self.moduleWarning)

    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)

    self.referencePathsComboBox = slicer.qMRMLNodeComboBox()
    self.referencePathsComboBox.nodeTypes = ["vtkMRMLPathReconstructionNode"]
    self.referencePathsComboBox.addEnabled = False
    self.referencePathsComboBox.removeEnabled = False
    self.referencePathsComboBox.noneEnabled = True
    self.referencePathsComboBox.showHidden = False
    self.referencePathsComboBox.showChildNodeTypes = False
    self.referencePathsComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Reference Paths: ", self.referencePathsComboBox)

    self.comparePathsComboBox = slicer.qMRMLNodeComboBox()
    self.comparePathsComboBox.nodeTypes = ["vtkMRMLPathReconstructionNode"]
    self.comparePathsComboBox.addEnabled = False
    self.comparePathsComboBox.removeEnabled = False
    self.comparePathsComboBox.noneEnabled = True
    self.comparePathsComboBox.showHidden = False
    self.comparePathsComboBox.showChildNodeTypes = False
    self.comparePathsComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Compare Paths: ", self.comparePathsComboBox)

    self.referenceEndMarkupsComboBox = slicer.qMRMLNodeComboBox()
    self.referenceEndMarkupsComboBox.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.referenceEndMarkupsComboBox.selectNodeUponCreation = True
    self.referenceEndMarkupsComboBox.addEnabled = True
    self.referenceEndMarkupsComboBox.removeEnabled = True
    self.referenceEndMarkupsComboBox.noneEnabled = True
    self.referenceEndMarkupsComboBox.showHidden = False
    self.referenceEndMarkupsComboBox.showChildNodeTypes = False
    self.referenceEndMarkupsComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Reference End Markups: ", self.referenceEndMarkupsComboBox)

    self.compareEndMarkupsComboBox = slicer.qMRMLNodeComboBox()
    self.compareEndMarkupsComboBox.nodeTypes = ["vtkMRMLMarkupsFiducialNode"]
    self.compareEndMarkupsComboBox.selectNodeUponCreation = True
    self.compareEndMarkupsComboBox.addEnabled = True
    self.compareEndMarkupsComboBox.removeEnabled = True
    self.compareEndMarkupsComboBox.noneEnabled = True
    self.compareEndMarkupsComboBox.showHidden = False
    self.compareEndMarkupsComboBox.showChildNodeTypes = False
    self.compareEndMarkupsComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Compare End Markups: ", self.compareEndMarkupsComboBox)

    self.referenceCombinedModelComboBox = slicer.qMRMLNodeComboBox()
    self.referenceCombinedModelComboBox.nodeTypes = ["vtkMRMLModelNode"]
    self.referenceCombinedModelComboBox.selectNodeUponCreation = True
    self.referenceCombinedModelComboBox.addEnabled = True
    self.referenceCombinedModelComboBox.removeEnabled = True
    self.referenceCombinedModelComboBox.noneEnabled = True
    self.referenceCombinedModelComboBox.showHidden = False
    self.referenceCombinedModelComboBox.showChildNodeTypes = False
    self.referenceCombinedModelComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Reference Combined Model: ", self.referenceCombinedModelComboBox)

    self.compareCombinedModelComboBox = slicer.qMRMLNodeComboBox()
    self.compareCombinedModelComboBox.nodeTypes = ["vtkMRMLModelNode"]
    self.compareCombinedModelComboBox.selectNodeUponCreation = True
    self.compareCombinedModelComboBox.addEnabled = True
    self.compareCombinedModelComboBox.removeEnabled = True
    self.compareCombinedModelComboBox.noneEnabled = True
    self.compareCombinedModelComboBox.showHidden = False
    self.compareCombinedModelComboBox.showChildNodeTypes = False
    self.compareCombinedModelComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Compare Combined Model: ", self.compareCombinedModelComboBox)

    self.registrationComboBox = slicer.qMRMLNodeComboBox()
    self.registrationComboBox.nodeTypes = ["vtkMRMLFiducialRegistrationWizardNode"]
    self.registrationComboBox.selectNodeUponCreation = True
    self.registrationComboBox.addEnabled = True
    self.registrationComboBox.removeEnabled = True
    self.registrationComboBox.noneEnabled = True
    self.registrationComboBox.showHidden = False
    self.registrationComboBox.showChildNodeTypes = False
    self.registrationComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Fiducial Registration: ", self.registrationComboBox)

    self.compareToReferenceTransformComboBox = slicer.qMRMLNodeComboBox()
    self.compareToReferenceTransformComboBox.nodeTypes = ["vtkMRMLLinearTransformNode"]
    self.compareToReferenceTransformComboBox.selectNodeUponCreation = True
    self.compareToReferenceTransformComboBox.addEnabled = True
    self.compareToReferenceTransformComboBox.removeEnabled = True
    self.compareToReferenceTransformComboBox.noneEnabled = True
    self.compareToReferenceTransformComboBox.showHidden = False
    self.compareToReferenceTransformComboBox.showChildNodeTypes = False
    self.compareToReferenceTransformComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("CompareToReference Transform: ", self.compareToReferenceTransformComboBox)

    self.outputTableComboBox = slicer.qMRMLNodeComboBox()
    self.outputTableComboBox.nodeTypes = ["vtkMRMLTableNode"]
    self.outputTableComboBox.selectNodeUponCreation = True
    self.outputTableComboBox.addEnabled = True
    self.outputTableComboBox.removeEnabled = True
    self.outputTableComboBox.noneEnabled = True
    self.outputTableComboBox.showHidden = False
    self.outputTableComboBox.showChildNodeTypes = False
    self.outputTableComboBox.setMRMLScene( slicer.mrmlScene )
    parametersFormLayout.addRow("Results Table: ", self.outputTableComboBox)

    self.runButton = qt.QPushButton("Run")
    self.runButton.enabled = False
    parametersFormLayout.addRow(self.runButton)

    # connections
    self.runButton.connect('clicked(bool)', self.onRunButton)
    self.referencePathsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.comparePathsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.referenceEndMarkupsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.compareEndMarkupsComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.referenceCombinedModelComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.compareCombinedModelComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.registrationComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.compareToReferenceTransformComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)
    self.outputTableComboBox.connect("currentNodeChanged(vtkMRMLNode*)", self.onNodeChanged)

    # Add vertical spacer
    self.layout.addStretch(1)

    # Refresh Run button state
    self.onNodeChanged()

  def cleanup(self):
    pass

  def onNodeChanged(self):
    self.runButton.enabled = self.referencePathsComboBox.currentNode() and \
                             self.comparePathsComboBox.currentNode()

  def onRunButton(self):
    logic = PathVerificationLogic()
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
    referenceCombinedModelNode = self.referenceCombinedModelComboBox.currentNode()
    if not referenceCombinedModelNode:
      referenceCombinedModelNode = slicer.vtkMRMLModelNode()
      referenceCombinedModelNode.SetName("referenceCombinedModelNode")
      slicer.mrmlScene.AddNode(referenceCombinedModelNode)
      referenceCombinedModelNode.CreateDefaultDisplayNodes()
      self.referenceCombinedModelComboBox.setCurrentNode(referenceCombinedModelNode)
    compareCombinedModelNode = self.compareCombinedModelComboBox.currentNode()
    if not compareCombinedModelNode:
      compareCombinedModelNode = slicer.vtkMRMLModelNode()
      compareCombinedModelNode.SetName("compareCombinedModelNode")
      slicer.mrmlScene.AddNode(compareCombinedModelNode)
      compareCombinedModelNode.CreateDefaultDisplayNodes()
      self.compareCombinedModelComboBox.setCurrentNode(compareCombinedModelNode)
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
    outputTableNode = self.outputTableComboBox.currentNode()
    if not outputTableNode:
      outputTableNode = slicer.vtkMRMLTableNode()
      outputTableNode.SetName("outputTableNode")
      slicer.mrmlScene.AddNode(outputTableNode)
      self.outputTableComboBox.setCurrentNode(outputTableNode)
    success = logic.run(referencePathsNode, comparePathsNode, \
                        referenceEndMarkupsNode, compareEndMarkupsNode, \
                        referenceCombinedModelNode, compareCombinedModelNode, \
                        registrationNode, compareToReferenceTransformNode, \
                        outputTableNode)
    if not success:
      logging.warning("Did not complete path verification.")

#
# PathVerificationLogic
#

class PathVerificationLogic(ScriptedLoadableModuleLogic):
  """
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  
  def run(self, referencePathsNode, comparePathsNode, \
                referenceEndMarkupsNode, compareEndMarkupsNode, \
                referenceCombinedModelNode, compareCombinedModelNode, \
                registrationNode, compareToReferenceLinearTransformNode, \
                outputTableNode):
    
    if not referencePathsNode or \
       not comparePathsNode or \
       not referenceEndMarkupsNode or \
       not compareEndMarkupsNode or \
       not referenceCombinedModelNode or \
       not compareCombinedModelNode or \
       not registrationNode or \
       not compareToReferenceLinearTransformNode or \
       not outputTableNode:
      logging.error("A node is null. Check nodes for null.")
      return False

    # align catheter paths
    self.computeCombinedModels(referencePathsNode, referenceCombinedModelNode)
    self.computeEndMarkups(referencePathsNode, referenceEndMarkupsNode)
    self.computeCombinedModels(comparePathsNode, compareCombinedModelNode)
    self.computeEndMarkups(comparePathsNode, compareEndMarkupsNode)
    self.performRegistration(referenceEndMarkupsNode, compareEndMarkupsNode, registrationNode, compareToReferenceLinearTransformNode)
    compareCombinedModelNode.SetAndObserveTransformNodeID(compareToReferenceLinearTransformNode.GetID())

    #TODO
    # 2. Optional: Determine correpsondence
    # 3. Measure distances between paths
    # 4. Output several tables
    # 5. Output averages

    return True

  def computeCombinedModels(self, pathsNode, combinedModelNode):
    appender = vtk.vtkAppendPolyData()
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    for suffixIndex in xrange(0, numberOfSuffixes):
      suffix = int(suffixArray.GetComponent(suffixIndex, 0))
      pathModelNode = pathsNode.GetPathModelNodeBySuffix(suffix)
      polyData = pathModelNode.GetPolyData()
      appender.AddInputData(polyData)
    appender.Update()
    combinedPolyData = appender.GetOutput()
    combinedModelNode.SetAndObservePolyData(combinedPolyData)

  def computeEndMarkups(self, pathsNode, endMarkupsNode):
    endMarkupsNode.RemoveAllMarkups()
    suffixArray = vtk.vtkIntArray()
    pathsNode.GetSuffixes( suffixArray )
    numberOfSuffixes = suffixArray.GetNumberOfTuples()
    for suffixIndex in xrange(0, numberOfSuffixes):
      suffix = int(suffixArray.GetComponent(suffixIndex, 0))
      pathModelNode = pathsNode.GetPathModelNodeBySuffix(suffix)
      polyData = pathModelNode.GetPolyData()
      points = polyData.GetPoints()
      firstPoint = points.GetPoint(0)
      endMarkupsNode.AddFiducialFromArray(firstPoint)
      numberOfPoints = points.GetNumberOfPoints()
      lastPoint = points.GetPoint(numberOfPoints-1)
      endMarkupsNode.AddFiducialFromArray(lastPoint)

  def performRegistration(self, referenceEndMarkupsNode, compareEndMarkupsNode, \
                          registrationNode, transformNode):
    registrationNode.SetUpdateModeToManual()
    registrationNode.SetPointMatchingToComputed()
    registrationNode.SetRegistrationModeToRigid()
    registrationNode.SetAndObserveFromFiducialListNodeId(compareEndMarkupsNode.GetID())
    registrationNode.SetAndObserveToFiducialListNodeId(referenceEndMarkupsNode.GetID())
    registrationNode.SetOutputTransformNodeId(transformNode.GetID())
    registrationLogic = slicer.modules.fiducialregistrationwizard.logic()
    registrationLogic.UpdateCalibration(registrationNode)
    #TODO transform is preliminary, still need to run ICP on the model pairs

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


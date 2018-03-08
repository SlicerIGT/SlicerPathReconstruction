/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// CollectPoints includes
#include "vtkSlicerPathReconstructionLogic.h"

// MRML includes
#include "vtkMRMLCollectPointsNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLPathReconstructionNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <cassert>
#include <sstream>

// vtk includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerPathReconstructionLogic);

//------------------------------------------------------------------------------
vtkSlicerPathReconstructionLogic::vtkSlicerPathReconstructionLogic()
{
}

//------------------------------------------------------------------------------
vtkSlicerPathReconstructionLogic::~vtkSlicerPathReconstructionLogic()
{
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue( vtkMRMLScene::NodeAddedEvent );
  events->InsertNextValue( vtkMRMLScene::NodeRemovedEvent );
  this->SetAndObserveMRMLSceneEventsInternal( newScene, events.GetPointer() );
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::RegisterNodes()
{
  if( !this->GetMRMLScene() )
  {
    vtkWarningMacro("MRML scene not yet created");
    return;
  }
  this->GetMRMLScene()->RegisterNodeClass( vtkSmartPointer< vtkMRMLPathReconstructionNode >::New() );
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::OnMRMLSceneNodeAdded( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro( "OnMRMLSceneNodeAdded: Invalid MRML scene or node" );
    return;
  }

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( node );
  if ( pathReconstructionNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeAdded: Module node added." );
    vtkUnObserveMRMLNodeMacro( pathReconstructionNode ); // Remove previous observers.
    vtkNew<vtkIntArray> events;
    events->InsertNextValue( vtkCommand::ModifiedEvent );
    vtkObserveMRMLNodeEventsMacro( pathReconstructionNode, events.GetPointer() );
  }
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::OnMRMLSceneNodeRemoved( vtkMRMLNode* node )
{
  if ( node == NULL || this->GetMRMLScene() == NULL )
  {
    vtkWarningMacro("OnMRMLSceneNodeRemoved: Invalid MRML scene or node");
    return;
  }
  
  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( node );
  if ( pathReconstructionNode )
  {
    vtkDebugMacro( "OnMRMLSceneNodeRemoved" );
    vtkUnObserveMRMLNodeMacro( node );
  }
}

//------------------------------------------------------------------------------
bool vtkSlicerPathReconstructionLogic::IsRecordingPossible( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( pathReconstructionNode == NULL ||
       pathReconstructionNode->GetMarkupsToModelNode() == NULL ||
       pathReconstructionNode->GetCollectPointsNode() == NULL ||
       pathReconstructionNode->GetSamplingTransformNode() == NULL )
  {
    return false;
  }
  return true;
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::ToggleRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( pathReconstructionNode == NULL )
  {
    vtkErrorMacro( "Path reconstruction node is not set. Cannot toggle recording." );
    return;
  }

  if ( pathReconstructionNode->GetRecordingState() == vtkMRMLPathReconstructionNode::Stopped )
  {
    this->StartRecording( pathReconstructionNode );
  }
  else if ( pathReconstructionNode->GetRecordingState() == vtkMRMLPathReconstructionNode::Recording )
  {
    this->StopRecording( pathReconstructionNode );
  }
  else
  {
    vtkErrorMacro( "Path reconstruction node is in an unrecognized recording state. Cannot toggle recording." );
    return;
  }
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::StartRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( !this->IsRecordingPossible( pathReconstructionNode ) )
  {
    vtkErrorMacro( "Parameters have not yet been set. Cannot record path." );
    return;
  }

  const char* anchorTransformNodeID = NULL;
  if ( pathReconstructionNode->GetAnchorTransformNode() != NULL )
  {
    anchorTransformNodeID = pathReconstructionNode->GetAnchorTransformNode()->GetID();
  }

  // create node for points storage
  vtkSmartPointer< vtkMRMLModelNode > pointsNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( pointsNode );
  std::stringstream pointsNameStream;
  pointsNameStream << pathReconstructionNode->GetPointsBaseName() << pathReconstructionNode->GetNextCount();
  pointsNode->SetName( pointsNameStream.str().c_str() );
  pointsNode->SetAndObserveTransformNodeID( anchorTransformNodeID );

  vtkMRMLCollectPointsNode* collectPointsNode = pathReconstructionNode->GetCollectPointsNode();
  const char* pointsNodeID = pointsNode->GetID();
  collectPointsNode->SetOutputNodeId( pointsNodeID );
  collectPointsNode->CreateDefaultDisplayNodesForOutputNodes();
  collectPointsNode->SetCollectModeToAutomatic();

  vtkMRMLModelDisplayNode* pointsDisplayNode = pointsNode->GetModelDisplayNode();
  double pointsRed = pathReconstructionNode->GetPointsColorRed();
  double pointsGreen = pathReconstructionNode->GetPointsColorGreen();
  double pointsBlue = pathReconstructionNode->GetPointsColorBlue();
  pointsDisplayNode->SetColor( pointsRed, pointsGreen, pointsBlue );

  // create node for path storage
  vtkSmartPointer< vtkMRMLModelNode > pathNode = vtkSmartPointer< vtkMRMLModelNode >::New();
  this->GetMRMLScene()->AddNode( pathNode );
  std::stringstream pathNameStream;
  pathNameStream << pathReconstructionNode->GetPathBaseName() << pathReconstructionNode->GetNextCount();
  pathNode->SetName( pathNameStream.str().c_str() );
  pathNode->SetAndObserveTransformNodeID( anchorTransformNodeID );
  pathNode->CreateDefaultDisplayNodes();

  vtkMRMLMarkupsToModelNode* markupsToModelNode = pathReconstructionNode->GetMarkupsToModelNode();
  markupsToModelNode->SetAndObserveInputNodeID( pointsNodeID );
  const char* pathNodeID = pathNode->GetID();
  markupsToModelNode->SetAndObserveOutputModelNodeID( pathNodeID );
  markupsToModelNode->SetAutoUpdateOutput( true );

  vtkMRMLModelDisplayNode* pathDisplayNode = pathNode->GetModelDisplayNode();
  double pathRed = pathReconstructionNode->GetPathColorRed();
  double pathGreen = pathReconstructionNode->GetPathColorGreen();
  double pathBlue = pathReconstructionNode->GetPathColorBlue();
  pathDisplayNode->SetColor( pathRed, pathGreen, pathBlue );

  // housekeeping
  pathReconstructionNode->SetRecordingStateToRecording();
  pathReconstructionNode->AddPointsPathPairModelNodeIDs( pointsNodeID, pathNodeID );
  pathReconstructionNode->SetNextCount( pathReconstructionNode->GetNextCount() + 1 );
}

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::StopRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( pathReconstructionNode == NULL )
  {
    vtkErrorMacro( "Parameter node is null. Cannot stop recording." );
    return;
  }

  vtkMRMLCollectPointsNode* collectPointsNode = pathReconstructionNode->GetCollectPointsNode();
  if ( collectPointsNode != NULL )
  {
    collectPointsNode->SetCollectModeToManual();
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = pathReconstructionNode->GetMarkupsToModelNode();
  if ( markupsToModelNode != NULL )
  {
    markupsToModelNode->SetAutoUpdateOutput( false );
  }

  pathReconstructionNode->SetRecordingStateToStopped();
}

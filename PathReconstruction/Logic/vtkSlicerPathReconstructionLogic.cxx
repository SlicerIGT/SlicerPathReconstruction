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
    events->InsertNextValue( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
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
void vtkSlicerPathReconstructionLogic::DeleteLastPath( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( pathReconstructionNode == NULL )
  {
    vtkErrorMacro( "Path reconstruction node is not set. Cannot delete last path." );
    return;
  }

  if ( pathReconstructionNode->GetNumberOfPathPointsPairs() <= 0 )
  {
    vtkWarningMacro( "Path reconstruction node has no paths. Cannot delete last path." );
    return;
  }

  int lastAddedSuffix = pathReconstructionNode->GetSuffixOfLastPathPointsPairAdded();
  vtkMRMLNode* pathModelNode = ( vtkMRMLNode* ) pathReconstructionNode->GetPathModelNodeBySuffix( lastAddedSuffix );
  vtkMRMLNode* pointsModelNode = ( vtkMRMLNode* ) pathReconstructionNode->GetPointsModelNodeBySuffix( lastAddedSuffix );
  pathReconstructionNode->RemovePointsPathPairBySuffix( lastAddedSuffix );
  vtkMRMLScene* scene = pathReconstructionNode->GetScene();
  if ( scene != NULL )
  {
    scene->RemoveNode( pathModelNode );
    scene->RemoveNode( pointsModelNode );
  }
  else
  {
    vtkWarningMacro( "Scene is null. Cannot delete points and path models from the scene." );
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
  collectPointsNode->SetOutputNodeID( pointsNodeID );
  collectPointsNode->CreateDefaultDisplayNodesForOutputNode();
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

//------------------------------------------------------------------------------
void vtkSlicerPathReconstructionLogic::RefitAllPaths( vtkMRMLPathReconstructionNode* pathReconstructionNode )
{
  if ( pathReconstructionNode == NULL )
  {
    vtkErrorMacro( "Parameter node is null. Cannot update paths." );
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = pathReconstructionNode->GetMarkupsToModelNode();
  if ( markupsToModelNode == NULL )
  {
    vtkErrorMacro( "Markups to model node is null. Cannot update paths." );
    return;
  }

  bool wasAutoUpdate = markupsToModelNode->GetAutoUpdateOutput();
  markupsToModelNode->SetAutoUpdateOutput( false );

  vtkSmartPointer< vtkIntArray > suffixArray = vtkSmartPointer< vtkIntArray >::New();
  pathReconstructionNode->GetSuffixes( suffixArray );
  int numberOfSuffixes = suffixArray->GetNumberOfTuples();
  for ( int suffixIndex = 0; suffixIndex < numberOfSuffixes; suffixIndex++ )
  {
    int suffix = suffixArray->GetComponent( suffixIndex, 0 );

    vtkMRMLModelNode* pointsNode = pathReconstructionNode->GetPointsModelNodeBySuffix( suffix );
    if ( pointsNode == NULL )
    {
      continue;
    }

    vtkMRMLModelNode* pathNode = pathReconstructionNode->GetPathModelNodeBySuffix( suffix );
    if ( pathNode == NULL )
    {
      // If we wanted to we could create a new model node here.
      // But until there is a need for such functionality,
      // we'll just leave things as we found them.
      continue;
    }

    vtkMRMLModelDisplayNode* pathDisplayNode = pathNode->GetModelDisplayNode();
    if ( pathDisplayNode == NULL )
    {
      pathNode->CreateDefaultDisplayNodes();
      pathDisplayNode = pathNode->GetModelDisplayNode();
    }
    if ( pathDisplayNode != NULL )
    {
      double pathColorRed = pathReconstructionNode->GetPathColorRed();
      double pathColorGreen = pathReconstructionNode->GetPathColorGreen();
      double pathColorBlue = pathReconstructionNode->GetPathColorBlue();
      pathDisplayNode->SetColor( pathColorRed, pathColorGreen, pathColorBlue );
    }
    else
    {
      vtkWarningMacro( "Unable to find or create display node for path node." );
    }

    markupsToModelNode->SetAndObserveInputNodeID( pointsNode->GetID() );
    markupsToModelNode->SetAndObserveOutputModelNodeID( pathNode->GetID() );
    markupsToModelNode->SetAutoUpdateOutput( true ); // TODO: This is a bit ugly, find another way to do this
    markupsToModelNode->SetAutoUpdateOutput( false );
  }

  markupsToModelNode->SetAutoUpdateOutput( wasAutoUpdate );
}

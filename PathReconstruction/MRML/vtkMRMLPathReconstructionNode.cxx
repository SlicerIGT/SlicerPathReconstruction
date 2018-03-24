/*==============================================================================

  Copyright (c) Thomas Vaughan
  Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// CollectPoints includes
#include "vtkMRMLPathReconstructionNode.h"

// slicer includes
#include "vtkMRMLCollectPointsNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLModelNode.h"

// std includes
#include <sstream>

// Constants ------------------------------------------------------------------
static const char* COLLECT_POINTS_ROLE = "CollectPointsRole";
static const char* MARKUPS_TO_MODEL_ROLE = "MarkupsToModelRole";
static const char* POINTS_MODEL_ROLE = "PointsModelRole";
static const char* PATH_MODEL_ROLE   = "PathModelRole";

vtkMRMLNodeNewMacro( vtkMRMLPathReconstructionNode );

//------------------------------------------------------------------------------
vtkMRMLPathReconstructionNode::vtkMRMLPathReconstructionNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  this->AddNodeReferenceRole( COLLECT_POINTS_ROLE );
  this->AddNodeReferenceRole( MARKUPS_TO_MODEL_ROLE );
  this->AddNodeReferenceRole( POINTS_MODEL_ROLE );
  this->AddNodeReferenceRole( PATH_MODEL_ROLE );
  this->PointsBaseName = "Points";
  this->PathBaseName = "Path";
  this->NextCount = 1;
  this->RecordingState = Stopped;
  this->PointsColorRed = 1.0f;
  this->PointsColorGreen = 0.5f;
  this->PointsColorBlue = 0.5f;
  this->PathColorRed = 1.0f;
  this->PathColorGreen = 1.0f;
  this->PathColorBlue = 0.0f;
}

//------------------------------------------------------------------------------
vtkMRMLPathReconstructionNode::~vtkMRMLPathReconstructionNode()
{
}

// ----------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML( of, nIndent ); // This will take care of referenced nodes

  vtkIndent indent( nIndent ); 
  of << indent << " PointsBaseName=\"" << this->PointsBaseName << "\"";
  of << indent << " PathBaseName=\"" << this->PathBaseName << "\"";
  of << indent << " NextCount=\"" << this->NextCount << "\"";
  // RecordingState should *not* be written to file. Recording should always
  // be initialized by the user. So default state is Stopped
}

// ----------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
  os << indent << " PointsBaseName=\"" << this->PointsBaseName << "\"";
  os << indent << " PathBaseName=\"" << this->PathBaseName << "\"";
  os << indent << " NextCount=\"" << this->NextCount << "\"";
  os << indent << " RecordingState=\"" << this->RecordingState << "\"";
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts); // This will take care of referenced nodes

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;
  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
    if ( ! strcmp( attName, "PointsBaseName" ) )
    {
      this->PointsBaseName = std::string(attValue);
      continue;
    }
    else if ( ! strcmp( attName, "PathBaseName" ) )
    {
      this->PointsBaseName = std::string(attValue);
      continue;
    }
    else if ( ! strcmp( attName, "NextCount" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->NextCount;
      continue;
    }
    // Do not read RecordingState from file. Should be Stopped
    // until it is manually started by user.
  }

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode ); // This will take care of referenced nodes
  this->Modified();
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLPathReconstructionNode::GetSamplingTransformNode()
{
  vtkMRMLCollectPointsNode* collectPointsNode = this->GetCollectPointsNode();
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "No CollectPoints module node is set. Returning NULL." );
    return NULL;
  }
  return collectPointsNode->GetSamplingTransformNode();
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* vtkMRMLPathReconstructionNode::GetAnchorTransformNode()
{
  vtkMRMLCollectPointsNode* collectPointsNode = this->GetCollectPointsNode();
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "No CollectPoints module node is set. Returning NULL." );
    return NULL;
  }
  return collectPointsNode->GetAnchorTransformNode();
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPointsColor( double red, double green, double blue )
{
  this->PointsColorRed = red;
  this->PointsColorGreen = green;
  this->PointsColorBlue = blue;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPathColor( double red, double green, double blue )
{
  this->PathColorRed = red;
  this->PathColorGreen = green;
  this->PathColorBlue = blue;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::CreateDefaultCollectPointsNode()
{
  vtkSmartPointer< vtkMRMLCollectPointsNode > collectPointsNode = vtkSmartPointer< vtkMRMLCollectPointsNode >::New();
  collectPointsNode->SetName( "CollectPointsNode_PathReconstruction" );
  this->ApplyDefaultSettingsToCollectPointsNode( collectPointsNode );
  this->GetScene()->AddNode( collectPointsNode );
  this->SetCollectPointsNodeID( collectPointsNode->GetID() );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::ApplyDefaultSettingsToCollectPointsNode( vtkMRMLCollectPointsNode* collectPointsNode )
{
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "CollectPoints node is null. Cannot apply default settings." )
    return;
  }
  collectPointsNode->SetMinimumDistance( 0.0 );
  collectPointsNode->SetCollectModeToManual();
}

//------------------------------------------------------------------------------
vtkMRMLCollectPointsNode* vtkMRMLPathReconstructionNode::GetCollectPointsNode()
{
  vtkMRMLCollectPointsNode* node = vtkMRMLCollectPointsNode::SafeDownCast( this->GetNodeReference( COLLECT_POINTS_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetCollectPointsNodeID( const char* nodeID )
{
  const char* currentNodeID = this->GetNodeReferenceID( COLLECT_POINTS_ROLE );
  if ( nodeID != NULL && currentNodeID != NULL && strcmp( nodeID, currentNodeID ) == 0 )
  {
    return; // not changed
  }
  this->SetNodeReferenceID( COLLECT_POINTS_ROLE, nodeID );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::CreateDefaultMarkupsToModelNode()
{
  vtkSmartPointer< vtkMRMLMarkupsToModelNode > markupsToModelNode = vtkSmartPointer< vtkMRMLMarkupsToModelNode >::New();
  markupsToModelNode->SetName( "MarkupsToModelNode_PathReconstruction" );
  this->ApplyDefaultSettingsToMarkupsToModelNode( markupsToModelNode );
  this->GetScene()->AddNode( markupsToModelNode );
  this->SetMarkupsToModelNodeID( markupsToModelNode->GetID() );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::ApplyDefaultSettingsToMarkupsToModelNode( vtkMRMLMarkupsToModelNode* markupsToModelNode )
{
  if ( markupsToModelNode == NULL )
  {
    vtkErrorMacro( "MarkupsToModel node is null. Cannot apply default settings." )
    return;
  }
  markupsToModelNode->SetModelType( vtkMRMLMarkupsToModelNode::Curve );
  markupsToModelNode->SetInterpolationType( vtkMRMLMarkupsToModelNode::Linear ); // TODO: Replace with moving least squares when available
  markupsToModelNode->SetPointParameterType( vtkMRMLMarkupsToModelNode::MinimumSpanningTree );
  markupsToModelNode->SetAutoUpdateOutput( false );
}

//------------------------------------------------------------------------------
vtkMRMLMarkupsToModelNode* vtkMRMLPathReconstructionNode::GetMarkupsToModelNode()
{
  vtkMRMLMarkupsToModelNode* node = vtkMRMLMarkupsToModelNode::SafeDownCast( this->GetNodeReference( MARKUPS_TO_MODEL_ROLE ) );
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetMarkupsToModelNodeID( const char* nodeID )
{
  const char* currentNodeID = this->GetNodeReferenceID( MARKUPS_TO_MODEL_ROLE );
  if ( nodeID != NULL && currentNodeID != NULL && strcmp( nodeID, currentNodeID ) == 0 )
  {
    return; // not changed
  }
  this->SetNodeReferenceID( MARKUPS_TO_MODEL_ROLE, nodeID );
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLPathReconstructionNode::GetNthPointsModelNode( int n )
{
  vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( POINTS_MODEL_ROLE, n ) );
  return node;
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLPathReconstructionNode::GetNthPathModelNode( int n )
{
  vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( PATH_MODEL_ROLE, n ) );
  return node;
}

//------------------------------------------------------------------------------
int vtkMRMLPathReconstructionNode::GetNumberOfPathPointsPairs()
{
  int numberOfPointsNodes = this->GetNumberOfNodeReferences( POINTS_MODEL_ROLE );
  int numberOfPathNodes = this->GetNumberOfNodeReferences( PATH_MODEL_ROLE );
  if ( numberOfPointsNodes != numberOfPathNodes )
  {
    vtkWarningMacro( "The number of points model nodes " << numberOfPointsNodes << " and " <<
                     "The number of path model nodes " << numberOfPathNodes << " are not equal. " <<
                     "This is a bug, and unexpected results may occur. Please save your scene and report this." <<
                     "Returning the least of the two values." );
  }

  return vtkMath::Min( numberOfPointsNodes, numberOfPathNodes );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::AddPointsPathPairModelNodeIDs( const char* pointsNodeID, const char* pathNodeID )
{
  int numberOfPointsNodes = this->GetNumberOfNodeReferences( POINTS_MODEL_ROLE );
  int numberOfPathNodes = this->GetNumberOfNodeReferences( PATH_MODEL_ROLE );
  if ( numberOfPointsNodes != numberOfPathNodes )
  {
    vtkErrorMacro( "The number of points model nodes " << numberOfPointsNodes << " and " <<
                   "The number of path model nodes " << numberOfPathNodes << " are not equal. " <<
                   "This is a bug, and unexpected results may occur. Please save your scene and report this." <<
                   "No nodes added." );
    return;
  }

  if ( IsModelNodeBeingObserved( pointsNodeID ) )
  {
    vtkErrorMacro( "Model node with ID " << pointsNodeID << " is already in a refence role. No nodes added." );
    return;
  }

  if ( IsModelNodeBeingObserved( pathNodeID ) )
  {
    vtkErrorMacro( "Model node with ID " << pathNodeID << " is already in a refence role. No nodes added." );
    return;
  }

  this->AddNodeReferenceID( POINTS_MODEL_ROLE, pointsNodeID );
  this->AddNodeReferenceID( PATH_MODEL_ROLE, pathNodeID );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::RemoveNthPointsPathPair( int n )
{
  int numberOfPointsNodes = this->GetNumberOfNodeReferences( POINTS_MODEL_ROLE );
  int numberOfPathNodes = this->GetNumberOfNodeReferences( PATH_MODEL_ROLE );
  if ( numberOfPointsNodes != numberOfPathNodes )
  {
    vtkErrorMacro( "The number of points model nodes " << numberOfPointsNodes << " and " <<
                   "The number of path model nodes " << numberOfPathNodes << " are not equal. " <<
                   "This is a bug, and unexpected results may occur. Please save your scene and report this. " <<
                   "No nodes removed.");
    return;
  }

  int numberOfPointsPathPairs = numberOfPointsNodes;
  if ( n >= numberOfPointsPathPairs )
  {
    vtkErrorMacro( "Number of points-path pairs is " << numberOfPointsPathPairs << ". " <<
                   "N " << n << " is out of range. No nodes removed." );
    return;
  }

  this->RemoveNthNodeReferenceID( POINTS_MODEL_ROLE, n );
  this->RemoveNthNodeReferenceID( PATH_MODEL_ROLE, n );
}

//------------------------------------------------------------------------------
bool vtkMRMLPathReconstructionNode::IsModelNodeBeingObserved( const char* nodeID )
{
  int numberOfPointsNodes = this->GetNumberOfNodeReferences( POINTS_MODEL_ROLE );
  for ( int pointsNodeIndex = 0; pointsNodeIndex < numberOfPointsNodes; pointsNodeIndex++ )
  {
    vtkMRMLModelNode* pointsNode = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( POINTS_MODEL_ROLE, pointsNodeIndex ) );
    if ( pointsNode == NULL )
    {
      vtkErrorMacro( "Points node at index " << pointsNodeIndex << " is not a model node. " <<
                     "This is indicative of a bug. Please save the scene and report this." );
      continue;
    }
    const char* pointsNodeID = pointsNode->GetID();
    if ( strcmp( nodeID, pointsNodeID ) == 0 )
    {
      return true;
    }
  }

  int numberOfPathNodes = this->GetNumberOfNodeReferences( PATH_MODEL_ROLE );
  for ( int pathNodeIndex = 0; pathNodeIndex < numberOfPathNodes; pathNodeIndex++ )
  {
    vtkMRMLModelNode* pathNode = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( PATH_MODEL_ROLE, pathNodeIndex ) );
    if ( pathNode == NULL )
    {
      vtkErrorMacro( "Points node at index " << pathNodeIndex << " is not a model node. " <<
                     "This is indicative of a bug. Please save the scene and report this." );
      continue;
    }
    const char* pathNodeID = pathNode->GetID();
    if ( strcmp( nodeID, pathNodeID ) == 0 )
    {
      return true;
    }
  }

  return false;
}

//------------------------------------------------------------------------------
int vtkMRMLPathReconstructionNode::RecordingStateFromString( const char* name )
{
  if ( name == NULL )
  {
    vtkGenericWarningMacro( "Null name provided." );
    return -1;
  }
  for ( int i = 0; i < RecordingState_Last; i++ )
  {
    if ( strcmp( name, vtkMRMLPathReconstructionNode::RecordingStateAsString( i ) ) == 0 )
    {
      return i;
    }
  }
  vtkGenericWarningMacro( "Unknown name provided " << name );
  return -1;
}

//------------------------------------------------------------------------------
const char* vtkMRMLPathReconstructionNode::RecordingStateAsString( int id )
{
  switch ( id )
  {
  case Stopped: return "Stopped";
  case Recording: return "Recording";
  default:
    vtkGenericWarningMacro( "Unknown id provided " << id );
    return "Unknown";
  }
}

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
static const char* POINTS_MODEL_ROLE_PREFIX = "PointsModelRole";
static const char* PATH_MODEL_ROLE_PREFIX   = "PathModelRole";

vtkMRMLNodeNewMacro( vtkMRMLPathReconstructionNode );

//------------------------------------------------------------------------------
vtkMRMLPathReconstructionNode::vtkMRMLPathReconstructionNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );

  vtkSmartPointer< vtkIntArray > observedCollectPointsEvents = vtkSmartPointer< vtkIntArray >::New();
  observedCollectPointsEvents->InsertNextValue( vtkMRMLCollectPointsNode::InputDataModifiedEvent );
  this->AddNodeReferenceRole( COLLECT_POINTS_ROLE, NULL, observedCollectPointsEvents );

  this->AddNodeReferenceRole( MARKUPS_TO_MODEL_ROLE );
  this->ReferenceRoleSuffixes = std::set< int >();
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
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  Superclass::WriteXML( of, nIndent ); // This will take care of referenced nodes

  vtkIndent indent( nIndent ); 
  of << indent << " PointsBaseName=\"" << this->PointsBaseName << "\"";
  of << indent << " PathBaseName=\"" << this->PathBaseName << "\"";
  of << indent << " NextCount=\"" << this->NextCount << "\"";
  of << indent << " ReferenceRoleSuffixes=\"";
  for ( std::set< int >::iterator suffixIterator = this->ReferenceRoleSuffixes.begin(); suffixIterator != this->ReferenceRoleSuffixes.end(); suffixIterator++ )
  { 
    if ( suffixIterator != this->ReferenceRoleSuffixes.begin() )
    {
      of << " ";
    }
    of << *suffixIterator;
  }
  of << "\"";
  // RecordingState should *not* be written to file. Recording should always
  // be initialized by the user. So default state is Stopped
}

// ----------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::PrintSelf( ostream& os, vtkIndent indent )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  Superclass::PrintSelf( os, indent );
  os << indent << " PointsBaseName=\"" << this->PointsBaseName << "\"";
  os << indent << " PathBaseName=\"" << this->PathBaseName << "\"";
  os << indent << " NextCount=\"" << this->NextCount << "\"";
  os << indent << " ReferenceRoleSuffixes=\"";
  for ( std::set< int >::iterator suffixIterator = this->ReferenceRoleSuffixes.begin(); suffixIterator != this->ReferenceRoleSuffixes.end(); suffixIterator++ )
  { 
    if ( suffixIterator != this->ReferenceRoleSuffixes.begin() )
    {
      os << " ";
    }
    os << *suffixIterator;
  }
  os << "\"";
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
      this->PathBaseName = std::string(attValue);
      continue;
    }
    else if ( ! strcmp( attName, "NextCount" ) )
    {
      std::stringstream ss;
      ss << attValue;
      ss >> this->NextCount;
      continue;
    }
    else if ( ! strcmp( attName, "ReferenceRoleSuffixes" ) )
    {
      this->ReferenceRoleSuffixes.clear();
      std::stringstream suffixStream( attValue );
      while ( ! suffixStream.eof() )
      {
        int suffix;
        suffixStream >> suffix;
        this->ReferenceRoleSuffixes.insert( suffix );
      }
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
void vtkMRMLPathReconstructionNode::ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData )
{
  Superclass::ProcessMRMLEvents( caller, event, callData );

  vtkMRMLNode* callerNode = vtkMRMLNode::SafeDownCast( caller );
  if ( callerNode == NULL )
  {
    return;
  }

  if ( callerNode == this->GetCollectPointsNode() )
  {
    if ( event == vtkMRMLCollectPointsNode::InputDataModifiedEvent )
    {
      this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
    }
  }
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
void vtkMRMLPathReconstructionNode::SetRecordingState( int newState )
{
  this->RecordingState = newState;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetNextCount( int newCount )
{
  this->NextCount = newCount;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPointsBaseName( std::string newBaseName )
{
  this->PointsBaseName = newBaseName;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPathBaseName( std::string newBaseName )
{
  this->PathBaseName = newBaseName;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPointsColor( double red, double green, double blue )
{
  this->PointsColorRed = red;
  this->PointsColorGreen = green;
  this->PointsColorBlue = blue;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::SetPathColor( double red, double green, double blue )
{
  this->PathColorRed = red;
  this->PathColorGreen = green;
  this->PathColorBlue = blue;
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::CreateDefaultCollectPointsNode()
{
  vtkSmartPointer< vtkMRMLCollectPointsNode > collectPointsNode = vtkSmartPointer< vtkMRMLCollectPointsNode >::New();
  collectPointsNode->SetName( "CollectPointsNode_PathReconstruction" );
  this->ApplyDefaultSettingsToCollectPointsNode( collectPointsNode );
  this->GetScene()->AddNode( collectPointsNode );
  this->SetAndObserveCollectPointsNodeID( collectPointsNode->GetID() );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::ApplyDefaultSettingsToCollectPointsNode( vtkMRMLCollectPointsNode* collectPointsNode )
{
  if ( collectPointsNode == NULL )
  {
    vtkErrorMacro( "CollectPoints node is null. Cannot apply default settings." );
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
void vtkMRMLPathReconstructionNode::SetAndObserveCollectPointsNodeID( const char* nodeID )
{
  const char* currentNodeID = this->GetNodeReferenceID( COLLECT_POINTS_ROLE );
  if ( nodeID != NULL && currentNodeID != NULL && strcmp( nodeID, currentNodeID ) == 0 )
  {
    return; // not changed
  }
  this->SetAndObserveNodeReferenceID( COLLECT_POINTS_ROLE, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::CreateDefaultMarkupsToModelNode()
{
  vtkSmartPointer< vtkMRMLMarkupsToModelNode > markupsToModelNode = vtkSmartPointer< vtkMRMLMarkupsToModelNode >::New();
  markupsToModelNode->SetName( "MarkupsToModelNode_PathReconstruction" );
  this->ApplyDefaultSettingsToMarkupsToModelNode( markupsToModelNode );
  this->GetScene()->AddNode( markupsToModelNode );
  this->SetAndObserveMarkupsToModelNodeID( markupsToModelNode->GetID() );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::ApplyDefaultSettingsToMarkupsToModelNode( vtkMRMLMarkupsToModelNode* markupsToModelNode )
{
  if ( markupsToModelNode == NULL )
  {
    vtkErrorMacro( "MarkupsToModel node is null. Cannot apply default settings." );
    return;
  }
  markupsToModelNode->SetModelType( vtkMRMLMarkupsToModelNode::Curve );
  markupsToModelNode->SetCurveType( vtkMRMLMarkupsToModelNode::Linear ); // TODO: Replace with moving least squares when available
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
void vtkMRMLPathReconstructionNode::SetAndObserveMarkupsToModelNodeID( const char* nodeID )
{
  const char* currentNodeID = this->GetNodeReferenceID( MARKUPS_TO_MODEL_ROLE );
  if ( nodeID != NULL && currentNodeID != NULL && strcmp( nodeID, currentNodeID ) == 0 )
  {
    return; // not changed
  }
  this->SetAndObserveNodeReferenceID( MARKUPS_TO_MODEL_ROLE, nodeID );
  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::InputDataModifiedEvent );
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLPathReconstructionNode::GetPointsModelNodeBySuffix( int suffix )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  std::string referenceRole = this->GetNodeReferenceRole( POINTS_MODEL_ROLE_PREFIX, suffix );
  vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( referenceRole.c_str() ) );
  return node;
}

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLPathReconstructionNode::GetPathModelNodeBySuffix( int suffix )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  std::string referenceRole = this->GetNodeReferenceRole( PATH_MODEL_ROLE_PREFIX, suffix );
  vtkMRMLModelNode* node = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( referenceRole.c_str() ) );
  return node;
}

//------------------------------------------------------------------------------
int vtkMRMLPathReconstructionNode::GetNumberOfPathPointsPairs()
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  return this->ReferenceRoleSuffixes.size();
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::GetSuffixes( vtkIntArray* suffixesArray )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }
  
  if ( suffixesArray == NULL )
  {
    vtkWarningMacro( "Suffixes array is null - cannot save reference role suffixes." );
    return;
  }

  suffixesArray->Reset();
  suffixesArray->SetNumberOfComponents( 1 );
  for ( std::set<int>::const_iterator currentSuffix = this->ReferenceRoleSuffixes.begin(); currentSuffix != this->ReferenceRoleSuffixes.end(); currentSuffix++ )
  {
    suffixesArray->InsertNextTuple1( *currentSuffix );
  }
}

//------------------------------------------------------------------------------
int vtkMRMLPathReconstructionNode::GetSuffixOfLastPathPointsPairAdded()
{
  if ( this->GetNumberOfPathPointsPairs() <= 0 )
  {
    return -1;
  }

  int largestNumberInSet = *( this->ReferenceRoleSuffixes.rbegin() );
  return largestNumberInSet;
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::AddPointsPathPairModelNodeIDs( const char* pointsNodeID, const char* pathNodeID )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  if ( this->IsModelNodeBeingObserved( pointsNodeID ) )
  {
    vtkErrorMacro( "Model node with ID " << pointsNodeID << " is already in a refence role. No nodes added." );
    return;
  }

  if ( this->IsModelNodeBeingObserved( pathNodeID ) )
  {
    vtkErrorMacro( "Model node with ID " << pathNodeID << " is already in a refence role. No nodes added." );
    return;
  }

  int newSuffix = this->GetSuffixOfLastPathPointsPairAdded() + 1; // Get largest element, add 1
  this->ReferenceRoleSuffixes.insert( newSuffix );

  std::string pointsReferenceRole = this->GetNodeReferenceRole( POINTS_MODEL_ROLE_PREFIX, newSuffix );
  this->AddNodeReferenceRole( pointsReferenceRole.c_str() );
  this->SetAndObserveNodeReferenceID( pointsReferenceRole.c_str(), pointsNodeID );

  std::string pathReferenceRole = this->GetNodeReferenceRole( PATH_MODEL_ROLE_PREFIX, newSuffix );
  this->AddNodeReferenceRole( pathReferenceRole.c_str() );
  this->SetAndObserveNodeReferenceID( pathReferenceRole.c_str(), pathNodeID );

  this->InvokeCustomModifiedEvent( vtkMRMLPathReconstructionNode::PathAddedEvent );
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::RemovePointsPathPairBySuffix( int suffix )
{
  // update reference roles if needed
  if ( this->ArePointsPathRolesUsingBaseNameOnly() )
  {
    this->FixPointsPathRolesUsingBaseNameOnly();
  }

  // check if it contains this node (note: std::set::contains will be added in C++20)
  if ( this->ReferenceRoleSuffixes.find( suffix ) == this->ReferenceRoleSuffixes.end() )
  {
    vtkErrorMacro( "Points-path pair with suffix = " << suffix << " does not exist. No nodes removed." );
    return;
  }

  std::string pointsReferenceRole = this->GetNodeReferenceRole( POINTS_MODEL_ROLE_PREFIX, suffix );
  this->RemoveNodeReferenceIDs( pointsReferenceRole.c_str() );

  std::string pathReferenceRole = this->GetNodeReferenceRole( PATH_MODEL_ROLE_PREFIX, suffix );
  this->RemoveNodeReferenceIDs( pathReferenceRole.c_str() );
  
  this->ReferenceRoleSuffixes.erase( suffix );

  this->Modified();
}

//------------------------------------------------------------------------------
std::string vtkMRMLPathReconstructionNode::GetNodeReferenceRole( const char* prefix, int suffix )
{
  std::stringstream nodeReferenceRoleStream;
  nodeReferenceRoleStream << prefix << suffix;
  std::string nodeReferenceRoleString = nodeReferenceRoleStream.str();
  return nodeReferenceRoleString;
}

//------------------------------------------------------------------------------
bool vtkMRMLPathReconstructionNode::IsModelNodeBeingObserved( const char* nodeID )
{
  for ( std::set< int >::iterator suffixIterator = this->ReferenceRoleSuffixes.begin(); suffixIterator != this->ReferenceRoleSuffixes.end(); suffixIterator++ )
  {
    std::string pointsReferenceRole = this->GetNodeReferenceRole( POINTS_MODEL_ROLE_PREFIX, *suffixIterator );
    vtkMRMLModelNode* pointsNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( pointsReferenceRole.c_str() ) );
    if ( pointsNode != NULL )
    {
      const char* pointsNodeID = pointsNode->GetID();
      if ( strcmp( nodeID, pointsNodeID ) == 0 )
      {
        return true;
      }
    }
    
    std::string pathReferenceRole = this->GetNodeReferenceRole( PATH_MODEL_ROLE_PREFIX, *suffixIterator );
    vtkMRMLModelNode* pathNode = vtkMRMLModelNode::SafeDownCast( this->GetNodeReference( pathReferenceRole.c_str() ) );
    if ( pathNode != NULL )
    {
      const char* pathNodeID = pathNode->GetID();
      if ( strcmp( nodeID, pathNodeID ) == 0 )
      {
        return true;
      }
    }
  }

  return false;
}

//------------------------------------------------------------------------------
bool vtkMRMLPathReconstructionNode::ArePointsPathRolesUsingBaseNameOnly()
{
  // Current implementation uses a base role name plus a number.
  // If the number is missing, then the role name(s) need to be updated.
  vtkMRMLNode* pointsNode = this->GetNodeReference( POINTS_MODEL_ROLE_PREFIX );
  if ( pointsNode != NULL )
  {
    return true;
  }

  vtkMRMLNode* pathNode = this->GetNodeReference( PATH_MODEL_ROLE_PREFIX );
  if ( pathNode != NULL )
  {
    return true;
  }

  return false;
}

//------------------------------------------------------------------------------
void vtkMRMLPathReconstructionNode::FixPointsPathRolesUsingBaseNameOnly()
{
  vtkInfoMacro( "Attempting to update role names in vtkMRMLPathReconstructionNode " << this->GetName() );

  bool wasModify = this->StartModify();

  int numberPointsReferences = ( this->GetNumberOfNodeReferences( POINTS_MODEL_ROLE_PREFIX ) );
  int numberPathReferences = ( this->GetNumberOfNodeReferences( PATH_MODEL_ROLE_PREFIX ) );

  if ( numberPointsReferences != numberPathReferences )
  {
    vtkWarningMacro( "Failed to update role names in vtkMRMLPathReconstructionNode " << this->GetName() <<
                     " - there are " << numberPointsReferences << " points and " << numberPathReferences << " paths." );
    return;
  }

  int numberOfPointsPathPairs = numberPathReferences;
  for ( int pointsPathPairIndex = numberOfPointsPathPairs - 1; pointsPathPairIndex >= 0; pointsPathPairIndex-- )
  {
    this->ReferenceRoleSuffixes.insert( pointsPathPairIndex );

    vtkMRMLModelNode* pointsNode = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( POINTS_MODEL_ROLE_PREFIX, pointsPathPairIndex ) );
    if ( pointsNode == NULL )
    {
      vtkWarningMacro( "Failed to update role names in vtkMRMLPathReconstructionNode " << this->GetName() <<
      " - points node number " << pointsPathPairIndex << " is null." );
      return;
    }
    std::string newPointsReferenceRole = this->GetNodeReferenceRole( POINTS_MODEL_ROLE_PREFIX, pointsPathPairIndex );
    this->AddNodeReferenceRole( newPointsReferenceRole.c_str() );
    this->SetAndObserveNodeReferenceID( newPointsReferenceRole.c_str(), pointsNode->GetID() );
    this->RemoveNthNodeReferenceID( POINTS_MODEL_ROLE_PREFIX, pointsPathPairIndex );

    vtkMRMLModelNode* pathNode = vtkMRMLModelNode::SafeDownCast( this->GetNthNodeReference( PATH_MODEL_ROLE_PREFIX, pointsPathPairIndex ) );
    if ( pathNode == NULL )
    {
      vtkWarningMacro( "Failed to update role names in vtkMRMLPathReconstructionNode " << this->GetName() <<
      " - path node number " << pointsPathPairIndex << " is null." );
      return;
    }
    std::string newPathReferenceRole = this->GetNodeReferenceRole( PATH_MODEL_ROLE_PREFIX, pointsPathPairIndex );
    this->AddNodeReferenceRole( newPathReferenceRole.c_str() );
    this->SetAndObserveNodeReferenceID( newPathReferenceRole.c_str(), pathNode->GetID() );
    this->RemoveNthNodeReferenceID( PATH_MODEL_ROLE_PREFIX, pointsPathPairIndex );
  }

  this->EndModify( wasModify );

  vtkInfoMacro( "Successfully updated role names in vtkMRMLPathReconstructionNode " << this->GetName() );
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

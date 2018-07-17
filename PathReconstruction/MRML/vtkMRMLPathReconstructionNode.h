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


#ifndef __vtkMRMLPathReconstructionNode_h
#define __vtkMRMLPathReconstructionNode_h

// vtk includes
#include <vtkObject.h>
#include <vtkCommand.h>

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

class vtkMRMLCollectPointsNode;
class vtkMRMLTransformNode;
class vtkMRMLMarkupsToModelNode;
class vtkMRMLModelNode;

#include "vtkSlicerPathReconstructionModuleMRMLExport.h"

class VTK_SLICER_PATHRECONSTRUCTION_MODULE_MRML_EXPORT vtkMRMLPathReconstructionNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    /// The node stores both inputs (e.g., tooltip position, model, colors, etc.) and computed parameters.
    /// ModifiedEvent event is called if either an input or output node is set.
    /// InputDataModifiedEvent is invoked when input parameters/nodes are changed (including the relevant data inside nodes).
    InputDataModifiedEvent = vtkCommand::UserEvent + 595,
    // PathAddedEvent is invoked when a path is added to this node
    PathAddedEvent = vtkCommand::UserEvent + 596
  };

  enum RecordingState
  {
    Stopped = 0,
    Recording,
    RecordingState_Last // valid types go above this line
  };

  vtkTypeMacro( vtkMRMLPathReconstructionNode, vtkMRMLNode );
  
  // Standard MRML node methods
  static vtkMRMLPathReconstructionNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "PathReconstruction"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  vtkMRMLPathReconstructionNode();
  virtual ~vtkMRMLPathReconstructionNode();
  vtkMRMLPathReconstructionNode ( const vtkMRMLPathReconstructionNode& );
  void operator=( const vtkMRMLPathReconstructionNode& );

public:
  void ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData ) VTK_OVERRIDE;

  // Convenient access to the corresponding nodes in the CollectPoints module.
  vtkMRMLTransformNode* GetSamplingTransformNode();
  vtkMRMLTransformNode* GetAnchorTransformNode();

  vtkGetMacro( PointsColorRed, double );
  vtkGetMacro( PointsColorGreen, double );
  vtkGetMacro( PointsColorBlue, double );
  void SetPointsColor( double red, double green, double blue );

  vtkGetMacro( PathColorRed, double );
  vtkGetMacro( PathColorGreen, double );
  vtkGetMacro( PathColorBlue, double );
  void SetPathColor( double red, double green, double blue );

  vtkGetMacro( RecordingState, int );
  void SetRecordingState( int );
  void SetRecordingStateToStopped() { this->SetRecordingState( Stopped ); }
  void SetRecordingStateToRecording() { this->SetRecordingState( Recording ); }

  vtkGetMacro( NextCount, int );
  void SetNextCount( int );

  void CreateDefaultCollectPointsNode();
  void ApplyDefaultSettingsToCollectPointsNode( vtkMRMLCollectPointsNode* node );
  vtkMRMLCollectPointsNode* GetCollectPointsNode();
  void SetAndObserveCollectPointsNodeID( const char* nodeID );

  vtkGetMacro( PointsBaseName, std::string );
  void SetPointsBaseName( std::string );

  void CreateDefaultMarkupsToModelNode();
  void ApplyDefaultSettingsToMarkupsToModelNode( vtkMRMLMarkupsToModelNode* node );
  vtkMRMLMarkupsToModelNode* GetMarkupsToModelNode();
  void SetAndObserveMarkupsToModelNodeID( const char* nodeID );

  vtkGetMacro( PathBaseName, std::string );
  void SetPathBaseName( std::string );

  void GetSuffixes( vtkIntArray* array );
  
  // outputs
  vtkMRMLModelNode* GetPointsModelNodeBySuffix( int suffix );
  vtkMRMLModelNode* GetPathModelNodeBySuffix( int suffix );
  int GetNumberOfPathPointsPairs();
  int GetSuffixOfLastPathPointsPairAdded(); // returns -1 if there are no pairs
  void AddPointsPathPairModelNodeIDs( const char* pointsNodeID, const char* pathNodeID );
  void RemovePointsPathPairBySuffix( int suffix );

  static int RecordingStateFromString( const char* name );
  static const char* RecordingStateAsString( int id );
  
private:
  // the next collected pair of points and path will have these base names:
  // PointsBaseName + NextCount
  // PathBaseName + NextCount
  std::string PointsBaseName;
  std::string PathBaseName;
  int NextCount;

  // These are the node reference ID suffixes (append these to the prefixes defined in the .cxx file to get node reference ID's)
  std::set< int > ReferenceRoleSuffixes;

  // Determine when new paths are being recorded. Options are stopped and recording.
  int RecordingState;

  // store the color selections made in this module (need to be copied each time a new model node is created)
  double PointsColorRed;
  double PointsColorGreen;
  double PointsColorBlue;
  double PathColorRed;
  double PathColorGreen;
  double PathColorBlue;

  // helper to avoid duplicates in the list. Duplicates should never occur.
  bool IsModelNodeBeingObserved( const char* nodeID );

  // helper to get node ID's that have prefix+suffix
  std::string GetNodeReferenceRole( const char* prefix, int suffix );

  // update the reference roles from older versions
  bool ArePointsPathRolesUsingBaseNameOnly();
  void FixPointsPathRolesUsingBaseNameOnly();
};

#endif

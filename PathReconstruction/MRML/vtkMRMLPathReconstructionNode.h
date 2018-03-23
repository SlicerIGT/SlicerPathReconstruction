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

// FiducialRegistrationWizard includes
#include "vtkSlicerPathReconstructionModuleMRMLExport.h"

class VTK_SLICER_PATHRECONSTRUCTION_MODULE_MRML_EXPORT vtkMRMLPathReconstructionNode
: public vtkMRMLNode
{
public:

  enum Events
  {
    /// The node stores both inputs (e.g., tooltip position, model, colors, etc.) and computed parameters.
    /// InputDataModifiedEvent is only invoked when input parameters are changed.
    /// In contrast, ModifiedEvent event is called if either an input or output parameter is changed.
    // vtkCommand::UserEvent + 595 is just a random value that is very unlikely to be used for anything else in this class
    InputDataModifiedEvent = vtkCommand::UserEvent + 595
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
  vtkSetMacro( RecordingState, int );
  void SetRecordingStateToStopped() { this->SetRecordingState( Stopped ); }
  void SetRecordingStateToRecording() { this->SetRecordingState( Recording ); }

  vtkGetMacro( NextCount, int );
  vtkSetMacro( NextCount, int );

  void CreateDefaultCollectPointsNode();
  void ApplyDefaultSettingsToCollectPointsNode( vtkMRMLCollectPointsNode* node );
  vtkMRMLCollectPointsNode* GetCollectPointsNode();
  void SetCollectPointsNodeID( const char* nodeID );

  vtkGetMacro( PointsBaseName, std::string );
  vtkSetMacro( PointsBaseName, std::string );

  void CreateDefaultMarkupsToModelNode();
  void ApplyDefaultSettingsToMarkupsToModelNode( vtkMRMLMarkupsToModelNode* node );
  vtkMRMLMarkupsToModelNode* GetMarkupsToModelNode();
  void SetMarkupsToModelNodeID( const char* nodeID );

  vtkGetMacro( PathBaseName, std::string );
  vtkSetMacro( PathBaseName, std::string );
  
  // outputs
  vtkMRMLModelNode* GetNthPointsModelNode( int n );
  vtkMRMLModelNode* GetNthPathModelNode( int n );
  void AddPointsPathPairModelNodeIDs( const char* pointsNodeID, const char* pathNodeID );
  void RemoveNthPointsPathPair( int n );

  static int RecordingStateFromString( const char* name );
  static const char* RecordingStateAsString( int id );
  
private:
  // the next collected pair of points and path will have these base names:
  // PointsBaseName + NextCount
  // PathBaseName + NextCount
  std::string PointsBaseName;
  std::string PathBaseName;
  int NextCount;

  // Determine when new paths are being recorded. Options are stopped and recording.
  int RecordingState;

  // helper to avoid duplicates in the list. Duplicates should never occur.
  bool IsModelNodeBeingObserved( const char* nodeID );

  // store the color selections made in this module (need to be copied each time a new model node is created)
  double PointsColorRed;
  double PointsColorGreen;
  double PointsColorBlue;
  double PathColorRed;
  double PathColorGreen;
  double PathColorBlue;
};

#endif

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

#ifndef __vtkSlicerPathReconstructionLogic_h
#define __vtkSlicerPathReconstructionLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

class vtkMRMLCollectPointsNode;
class vtkMRMLLinearTransformNode;
class vtkMRMLMarkupsToModelNode;
class vtkMRMLModelNode;
class vtkMRMLPathReconstructionNode;

// STD includes
#include <string>
#include <cstdlib>

// includes related to PathReconstruction
#include "vtkSlicerPathReconstructionModuleLogicExport.h"

/// \ingroup Slicer_QtModules_PathReconstruction
class VTK_SLICER_PATHRECONSTRUCTION_MODULE_LOGIC_EXPORT vtkSlicerPathReconstructionLogic :
  public vtkSlicerModuleLogic
{
public:
  
  static vtkSlicerPathReconstructionLogic *New();
  vtkTypeMacro(vtkSlicerPathReconstructionLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  bool IsRecordingPossible( vtkMRMLPathReconstructionNode* pathReconstructionNode );
  void ToggleRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode );
  void DeleteLastPath( vtkMRMLPathReconstructionNode* pathReconstructionNode );
  void RefitAllPaths( vtkMRMLPathReconstructionNode* pathReconstructionNode );

protected:
  vtkSlicerPathReconstructionLogic();
  virtual ~vtkSlicerPathReconstructionLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene * newScene);
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:
  void StartRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode );
  void StopRecording( vtkMRMLPathReconstructionNode* pathReconstructionNode );
  vtkSlicerPathReconstructionLogic( const vtkSlicerPathReconstructionLogic& ); // Not implemented
  void operator= ( const vtkSlicerPathReconstructionLogic& );             // Not implemented
};

#endif


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

#ifndef __qSlicerPathReconstructionModuleWidget_h
#define __qSlicerPathReconstructionModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerPathReconstructionModuleExport.h"

class qSlicerPathReconstructionModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_PathReconstruction
class Q_SLICER_QTMODULES_PATHRECONSTRUCTION_EXPORT qSlicerPathReconstructionModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPathReconstructionModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPathReconstructionModuleWidget();

public slots:
  void setMRMLScene( vtkMRMLScene* scene );

protected slots:
  void blockAllSignals( bool block );
  void enableAllWidgets( bool enable );
  void onSceneImportedEvent();
  void onParameterNodeSelected();
  void onSamplingTransformSelected();
  void onAnchorTransformSelected();
  void onCollectPointsAdded( vtkMRMLNode* );
  void onCollectPointsSelected();
  void onCollectPointsColorChanged( QColor );
  void onPointsBaseNameChanged();
  void onPathBaseNameChanged();
  void onNextCountChanged();
  void onRecordingButtonClicked();
  void onDeleteButtonClicked();
  void onDeleteAllClicked();
  void updateGUIFromMRML();

protected:
  QScopedPointer<qSlicerPathReconstructionModuleWidgetPrivate> d_ptr;
  virtual void setup();
  virtual void enter();
  virtual void exit();

private:
  Q_DECLARE_PRIVATE(qSlicerPathReconstructionModuleWidget);
  Q_DISABLE_COPY(qSlicerPathReconstructionModuleWidget);
};

#endif

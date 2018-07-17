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

#ifndef __qSlicerPathReconstructionTableWidget_h
#define __qSlicerPathReconstructionTableWidget_h

// Qt includes
#include "qSlicerWidget.h"

// Slicer includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"

#include "qSlicerPathReconstructionModuleWidgetsExport.h"
#include "ui_qSlicerPathReconstructionTableWidget.h"

class qSlicerPathReconstructionTableWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_QTMODULES_PATHRECONSTRUCTION_WIDGETS_EXPORT qSlicerPathReconstructionTableWidget
  : public qSlicerWidget
{
  Q_OBJECT

public:
  typedef qSlicerWidget Superclass;
  qSlicerPathReconstructionTableWidget( QWidget *parent=0 );
  virtual ~qSlicerPathReconstructionTableWidget();

  /// Get the currently selected node.
  Q_INVOKABLE vtkMRMLNode* currentNode() const;

  /// Get the table widget
  Q_INVOKABLE QTableWidget* tableWidget() const;

  /// Get the node selector combo box
  Q_INVOKABLE qMRMLNodeComboBox* pathReconstructionNodeComboBox() const;

public slots:

  void setMRMLScene( vtkMRMLScene* scene );

  /// Set the currently selected node.
  void setPathReconstructionNode( vtkMRMLNode* currentNode );

  /// Make the combo box node selector visible or not
  void setPathReconstructionNodeComboBoxVisible( bool visible );

protected slots:

  /// Update the GUI to reflect the currently selected node.
  void updateGUIFromMRML();

protected:
  QScopedPointer< qSlicerPathReconstructionTableWidgetPrivate > d_ptr;
  virtual void setup();

private:
  Q_DECLARE_PRIVATE( qSlicerPathReconstructionTableWidget );
  Q_DISABLE_COPY( qSlicerPathReconstructionTableWidget );
};

#endif

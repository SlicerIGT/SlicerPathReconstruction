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

// Qt includes
#include <QtGui>
#include <QDebug>
#include <QMenu>

// SlicerQt includes
#include "qSlicerPathReconstructionModuleWidget.h"
#include "ui_qSlicerPathReconstructionModule.h"

// module includes
#include "vtkMRMLPathReconstructionNode.h"
#include "vtkSlicerPathReconstructionLogic.h"

// slicer includes
#include "vtkMRMLCollectPointsNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLMarkupsToModelNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLModelDisplayNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathReconstruction
class qSlicerPathReconstructionModuleWidgetPrivate : public Ui_qSlicerPathReconstructionModuleWidget
{
  Q_DECLARE_PUBLIC( qSlicerPathReconstructionModuleWidget ); 
  
protected:
  qSlicerPathReconstructionModuleWidget* const q_ptr;
public:
  qSlicerPathReconstructionModuleWidgetPrivate( qSlicerPathReconstructionModuleWidget& object );
  vtkSlicerPathReconstructionLogic* logic() const;

  vtkWeakPointer< vtkMRMLPathReconstructionNode > PathReconstructionNode;

  QMenu* DeleteMenu;
};

//-----------------------------------------------------------------------------
// qSlicerPathReconstructionModuleWidgetPrivate methods
//-----------------------------------------------------------------------------
qSlicerPathReconstructionModuleWidgetPrivate::qSlicerPathReconstructionModuleWidgetPrivate( qSlicerPathReconstructionModuleWidget& object )
: q_ptr( &object )
{
}

//-----------------------------------------------------------------------------
vtkSlicerPathReconstructionLogic* qSlicerPathReconstructionModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerPathReconstructionModuleWidget );
  return vtkSlicerPathReconstructionLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerPathReconstructionModuleWidget methods
//-----------------------------------------------------------------------------
qSlicerPathReconstructionModuleWidget::qSlicerPathReconstructionModuleWidget( QWidget* _parent )
: Superclass( _parent ), d_ptr( new qSlicerPathReconstructionModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerPathReconstructionModuleWidget::~qSlicerPathReconstructionModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::setup()
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  d->setupUi( this );
  this->Superclass::setup();

  this->setMRMLScene( d->logic()->GetMRMLScene() );

  connect( d->ParameterNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onParameterNodeSelected() ) );
  connect( d->SamplingTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onSamplingTransformSelected() ) );
  connect( d->AnchorTransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onAnchorTransformSelected() ) );

  // Advanced properties
  connect( d->NextCountSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( onNextCountChanged() ) );
  connect( d->PointsBaseNameLineEdit, SIGNAL( editingFinished() ), this, SLOT( onPointsBaseNameChanged() ) );
  connect( d->CollectPointsComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onCollectPointsSelected() ) );
  connect( d->CollectPointsComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onCollectPointsAdded( vtkMRMLNode* ) ) );
  connect( d->CollectPointsColorPicker, SIGNAL( colorChanged( QColor ) ), this, SLOT( onCollectPointsColorChanged( QColor ) ) );
  connect( d->PathBaseNameLineEdit, SIGNAL( editingFinished() ), this, SLOT( onPathBaseNameChanged() ) );
  connect( d->MarkupsToModelComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelSelected() ) );
  connect( d->MarkupsToModelComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onMarkupsToModelAdded( vtkMRMLNode* ) ) );
  connect( d->MarkupsToModelColorPicker, SIGNAL( colorChanged( QColor ) ), this, SLOT( onMarkupsToModelColorChanged( QColor ) ) );

  connect( d->RecordingButton, SIGNAL( clicked() ), this, SLOT( onRecordingButtonClicked() ) );

  connect( d->DeleteButton, SIGNAL( clicked() ), this, SLOT( onDeleteButtonClicked() ) );
  d->DeleteMenu = new QMenu(tr( "Delete options" ), d->DeleteButton );
  d->DeleteMenu->setObjectName( "DeleteMenu" );
  d->DeleteMenu->addAction( d->ActionDeleteAll );
  QObject::connect( d->ActionDeleteAll, SIGNAL( triggered() ), this, SLOT( onDeleteAllClicked() ) );
  d->DeleteButton->setMenu( d->DeleteMenu );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::enter()
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  this->Superclass::enter();

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << "Invalid scene!";
    return;
  }

  // For convenience, select a default parameter node.
  if ( d->ParameterNodeComboBox->currentNode() == NULL )
  {
    vtkMRMLNode* node = this->mrmlScene()->GetNthNodeByClass( 0, "vtkMRMLPathReconstructionNode" );
    // Create a new parameter node if there is none in the scene.
    if ( node == NULL )
    {
      node = this->mrmlScene()->AddNewNodeByClass( "vtkMRMLPathReconstructionNode" );
    }
    vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( node );
    if ( pathReconstructionNode == NULL )
    {
      qCritical( "Failed to create module node vtkMRMLPathReconstructionNode" );
      return;
    }
    d->ParameterNodeComboBox->setCurrentNode( pathReconstructionNode );
    pathReconstructionNode->CreateDefaultCollectPointsNode();
    pathReconstructionNode->CreateDefaultMarkupsToModelNode();
  }
 
  // Need to update the GUI so that it observes whichever parameter node is selected
  this->onParameterNodeSelected();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::exit()
{
  Superclass::exit();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::blockAllSignals( bool block )
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  d->ParameterNodeComboBox->blockSignals( block );
  d->SamplingTransformComboBox->blockSignals( block );
  d->AnchorTransformComboBox->blockSignals( block );
  d->NextCountSpinBox->blockSignals( block );
  d->PointsBaseNameLineEdit->blockSignals( block );
  d->CollectPointsComboBox->blockSignals( block );
  d->CollectPointsColorPicker->blockSignals( block );
  d->PathBaseNameLineEdit->blockSignals( block );
  d->MarkupsToModelComboBox->blockSignals( block );
  d->MarkupsToModelColorPicker->blockSignals( block );
  d->RecordingButton->blockSignals( block );
  d->DeleteButton->blockSignals( block );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::enableAllWidgets( bool enable )
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  // don't ever disable parameter node, need to be able to select between them
  d->SamplingTransformComboBox->setEnabled( enable );
  d->AnchorTransformComboBox->setEnabled( enable );
  d->NextCountSpinBox->setEnabled( enable );
  d->PointsBaseNameLineEdit->setEnabled( enable );
  d->CollectPointsComboBox->setEnabled( enable );
  d->CollectPointsColorPicker->setEnabled( enable );
  d->PathBaseNameLineEdit->setEnabled( enable );
  d->MarkupsToModelComboBox->setEnabled( enable );
  d->MarkupsToModelColorPicker->setEnabled( enable );
  d->RecordingButton->setEnabled( enable );
  d->DeleteButton->setEnabled( enable );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::setMRMLScene( vtkMRMLScene* scene )
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  this->Superclass::setMRMLScene( scene );
  qvtkReconnect( d->logic(), scene, vtkMRMLScene::EndImportEvent, this, SLOT( onSceneImportedEvent() ) );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onSceneImportedEvent()
{
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onParameterNodeSelected()
{
  Q_D( qSlicerPathReconstructionModuleWidget );
  vtkMRMLPathReconstructionNode* selectedPathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  qvtkReconnect( d->PathReconstructionNode, selectedPathReconstructionNode, vtkMRMLPathReconstructionNode::InputDataModifiedEvent, this, SLOT( updateGUIFromMRML() ) );
  qvtkReconnect( d->PathReconstructionNode, selectedPathReconstructionNode, vtkCommand::ModifiedEvent, this, SLOT( updateGUIFromMRML() ) );
  d->PathReconstructionNode = selectedPathReconstructionNode;
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onSamplingTransformSelected()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  const char* samplingTransformNodeID = NULL;
  vtkMRMLLinearTransformNode* samplingTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->SamplingTransformComboBox->currentNode() );
  if( samplingTransformNode != NULL )
  {
    samplingTransformNodeID = samplingTransformNode->GetID();
  }

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->CollectPointsComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid CollectPoints node";
    return;
  }
  collectPointsNode->SetAndObserveSamplingTransformNodeID( samplingTransformNodeID );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onAnchorTransformSelected()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  const char* anchorTransformNodeID = NULL;
  vtkMRMLLinearTransformNode* anchorTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( d->AnchorTransformComboBox->currentNode() );
  if( anchorTransformNode != NULL )
  {
    anchorTransformNodeID = anchorTransformNode->GetID();
  }

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->CollectPointsComboBox->currentNode() );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid CollectPoints node";
    return;
  }
  collectPointsNode->SetAndObserveAnchorTransformNodeID( anchorTransformNodeID );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onCollectPointsAdded( vtkMRMLNode* newNode )
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( newNode );
  if ( collectPointsNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid CollectPoints node";
    return;
  }
  pathReconstructionNode->ApplyDefaultSettingsToCollectPointsNode( collectPointsNode );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onCollectPointsSelected()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  const char* collectPointsNodeID = NULL;
  vtkMRMLCollectPointsNode* collectPointsNode = vtkMRMLCollectPointsNode::SafeDownCast( d->CollectPointsComboBox->currentNode() );
  if( collectPointsNode != NULL )
  {
    collectPointsNodeID = collectPointsNode->GetID();
  }
  pathReconstructionNode->SetAndObserveCollectPointsNodeID( collectPointsNodeID );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onCollectPointsColorChanged( QColor col )
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  double red = (double)col.redF();
  double green = (double)col.greenF();
  double blue = (double)col.blueF();
  pathReconstructionNode->SetPointsColor( red, green, blue );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onMarkupsToModelAdded( vtkMRMLNode* newNode )
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( newNode );
  if ( markupsToModelNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid MarkupsToModel node";
    return;
  }
  pathReconstructionNode->ApplyDefaultSettingsToMarkupsToModelNode( markupsToModelNode );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onMarkupsToModelSelected()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  const char* markupsToModelNodeID = NULL;
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->MarkupsToModelComboBox->currentNode() );
  if( markupsToModelNode != NULL )
  {
    markupsToModelNodeID = markupsToModelNode->GetID();
  }
  pathReconstructionNode->SetAndObserveMarkupsToModelNodeID( markupsToModelNodeID );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onMarkupsToModelColorChanged( QColor col )
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  double red = (double)col.redF();
  double green = (double)col.greenF();
  double blue = (double)col.blueF();
  pathReconstructionNode->SetPathColor( red, green, blue );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onPointsBaseNameChanged()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  std::string newText = d->PointsBaseNameLineEdit->text().toStdString();
  pathReconstructionNode->SetPointsBaseName( newText );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onPathBaseNameChanged()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  std::string newText = d->PathBaseNameLineEdit->text().toStdString();
  pathReconstructionNode->SetPathBaseName( newText );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onNextCountChanged()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  int newNextValue = d->NextCountSpinBox->value();
  pathReconstructionNode->SetNextCount( newNextValue );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onRecordingButtonClicked()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  d->logic()->ToggleRecording( pathReconstructionNode );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onDeleteButtonClicked()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  int numberOfPaths = pathReconstructionNode->GetNumberOfPathPointsPairs();
  if ( numberOfPaths <= 0 )
  {
    qWarning() << Q_FUNC_INFO << "Cannot delete last path because there are no paths";
    return;
  }

  pathReconstructionNode->SetNextCount( numberOfPaths ); // reduce by one

  d->logic()->DeleteLastPath( pathReconstructionNode );

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::onDeleteAllClicked()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  pathReconstructionNode->SetNextCount( 1 );

  int whileLoopSafetyCounter = 500;
  while ( pathReconstructionNode->GetNumberOfPathPointsPairs() > 0 && whileLoopSafetyCounter > 0 )
  {
    d->logic()->DeleteLastPath( pathReconstructionNode );
    whileLoopSafetyCounter--;
  }

  if ( whileLoopSafetyCounter <= 0 )
  {
    qCritical() << Q_FUNC_INFO << ": tried to delete all paths, but may have gotten caught in infinite loop. Stopping.";
  }

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModuleWidget::updateGUIFromMRML()
{
  Q_D( qSlicerPathReconstructionModuleWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->ParameterNodeComboBox->currentNode() );
  if ( pathReconstructionNode == NULL )
  {
    // should not be able to change parameters in a non-existing node...
    this->enableAllWidgets( false );
    return;
  }

  // assume at first everything should be enabled, disable on an individual basis
  this->enableAllWidgets( true );

  // temporarily block signals so nothing gets triggered when updating the GUI
  this->blockAllSignals( true );

  vtkMRMLCollectPointsNode* collectPointsNode = pathReconstructionNode->GetCollectPointsNode();
  d->CollectPointsComboBox->setCurrentNode( collectPointsNode );

  if ( collectPointsNode )
  {
    vtkMRMLTransformNode* samplingTransformNode = pathReconstructionNode->GetSamplingTransformNode();
    d->SamplingTransformComboBox->setCurrentNode( samplingTransformNode );
    vtkMRMLTransformNode* anchorTransformNode = pathReconstructionNode->GetAnchorTransformNode();
    d->AnchorTransformComboBox->setCurrentNode( anchorTransformNode );
  }
  else
  {
    d->SamplingTransformComboBox->setEnabled( false );
    d->SamplingTransformComboBox->setCurrentNode( NULL );
    d->AnchorTransformComboBox->setEnabled( false );
    d->AnchorTransformComboBox->setCurrentNode( NULL );
  }

  QColor pointsColor;
  double pointsRed = pathReconstructionNode->GetPointsColorRed();
  double pointsGreen = pathReconstructionNode->GetPointsColorGreen();
  double pointsBlue = pathReconstructionNode->GetPointsColorBlue();
  pointsColor.setRgbF( pointsRed, pointsGreen, pointsBlue );
  d->CollectPointsColorPicker->setColor( pointsColor );

  vtkMRMLMarkupsToModelNode* markupsToModelNode = pathReconstructionNode->GetMarkupsToModelNode();
  d->MarkupsToModelComboBox->setCurrentNode( markupsToModelNode );

  QColor pathColor;
  double pathRed = pathReconstructionNode->GetPathColorRed();
  double pathGreen = pathReconstructionNode->GetPathColorGreen();
  double pathBlue = pathReconstructionNode->GetPathColorBlue();
  pathColor.setRgbF( pathRed, pathGreen, pathBlue );
  d->MarkupsToModelColorPicker->setColor( pathColor );

  QString pointsBaseLabel = QString( pathReconstructionNode->GetPointsBaseName().c_str() );
  d->PointsBaseNameLineEdit->setText( pointsBaseLabel );

  QString pathBaseLabel = QString( pathReconstructionNode->GetPathBaseName().c_str() );
  d->PathBaseNameLineEdit->setText( pathBaseLabel );

  int nextCount = pathReconstructionNode->GetNextCount();
  d->NextCountSpinBox->setValue( nextCount );

  if ( pathReconstructionNode->GetRecordingState() == vtkMRMLPathReconstructionNode::Stopped )
  {
    d->RecordingButton->setChecked( false );
    d->RecordingButton->setText( "Start Recording" );
    if ( !d->logic()->IsRecordingPossible( pathReconstructionNode ) )
    {
      d->RecordingButton->setEnabled( false );
    }
  }
  else
  {
    d->RecordingButton->setChecked( true );
    d->RecordingButton->setText( "Stop Recording" );
    // shouldn't need to support change of inputs while recording
    d->DeleteButton->setEnabled( false );
    d->SamplingTransformComboBox->setEnabled( false );
    d->AnchorTransformComboBox->setEnabled( false );
    d->CollectPointsComboBox->setEnabled( false );
    d->CollectPointsColorPicker->setEnabled( false );
    d->MarkupsToModelComboBox->setEnabled( false );
    d->MarkupsToModelColorPicker->setEnabled( false );
    d->NextCountSpinBox->setEnabled( false );
    d->PointsBaseNameLineEdit->setEnabled( false );
    d->PathBaseNameLineEdit->setEnabled( false );
  }

  int numberOfPaths = pathReconstructionNode->GetNumberOfPathPointsPairs();
  if ( numberOfPaths > 0 )
  {
    d->RecordingButton->setEnabled( true );
  }

  this->blockAllSignals( false );
}

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

#include "qSlicerPathReconstructionTableWidget.h"

#include "vtkSlicerPathReconstructionLogic.h"

// QSlicer includes
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include "vtkMRMLPathReconstructionNode.h"
#include "vtkMRMLMarkupsToModelNode.h"

// Qt includes
#include <QDebug>
#include <QtGui>
#include <QtPlugin>
#include <QTableWidgetItem>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathReconstruction
class qSlicerPathReconstructionTableWidgetPrivate : public Ui_qSlicerPathReconstructionTableWidget
{
  Q_DECLARE_PUBLIC( qSlicerPathReconstructionTableWidget );
protected:
  qSlicerPathReconstructionTableWidget* const q_ptr;

public:
  qSlicerPathReconstructionTableWidgetPrivate( qSlicerPathReconstructionTableWidget& object);
  vtkWeakPointer< vtkMRMLPathReconstructionNode > PathReconstructionNode;
  vtkWeakPointer< vtkSlicerPathReconstructionLogic > PathReconstructionLogic;
};

// --------------------------------------------------------------------------
qSlicerPathReconstructionTableWidgetPrivate::qSlicerPathReconstructionTableWidgetPrivate( qSlicerPathReconstructionTableWidget& object ) : q_ptr( &object )
{
}

//-----------------------------------------------------------------------------
qSlicerPathReconstructionTableWidget::qSlicerPathReconstructionTableWidget( QWidget* parentWidget ) : Superclass( parentWidget ) , d_ptr( new qSlicerPathReconstructionTableWidgetPrivate( *this ) )
{
  this->setup();
}

//-----------------------------------------------------------------------------
qSlicerPathReconstructionTableWidget::~qSlicerPathReconstructionTableWidget()
{
  this->setPathReconstructionNode( NULL );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setup()
{
  Q_D( qSlicerPathReconstructionTableWidget );
  d->setupUi( this );

  // This cannot be called by the constructor, because Slicer may not exist when the constructor is called
  d->PathReconstructionLogic = NULL;
  if (qSlicerApplication::application() != NULL && qSlicerApplication::application()->moduleManager() != NULL)
  {
    qSlicerAbstractCoreModule* pathReconstructionModule = qSlicerApplication::application()->moduleManager()->module( "PathReconstruction" );
    if ( pathReconstructionModule != NULL )
    {
      d->PathReconstructionLogic = vtkSlicerPathReconstructionLogic::SafeDownCast( pathReconstructionModule->logic() );
      vtkMRMLScene* scene = d->PathReconstructionLogic->GetMRMLScene();
      this->setMRMLScene( scene );
    }
  }
  if (d->PathReconstructionLogic == NULL)
  {
    qCritical() << Q_FUNC_INFO << ": path reconstruction module logic not found. Some functionality will not work.";
  }

  connect( d->PathReconstructionNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateGUIFromMRML() ) );
  connect( d->FittingParametersComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( onFittingParametersAdded( vtkMRMLNode* ) ) );
  connect( d->FittingParametersComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onFittingParametersChanged() ) );
  connect( d->FittingColorPicker, SIGNAL( colorChanged( QColor ) ), this, SLOT( onFittingColorChanged( QColor ) ) );
  connect( d->RefitPathsButton, SIGNAL( clicked() ), this, SLOT( onRefitPathsButtonClicked() ) );
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerPathReconstructionTableWidget::currentNode() const
{
  Q_D( const qSlicerPathReconstructionTableWidget );
  return d->PathReconstructionNode;
}

//-----------------------------------------------------------------------------
QTableWidget* qSlicerPathReconstructionTableWidget::tableWidget() const
{
  Q_D( const qSlicerPathReconstructionTableWidget );
  return d->PathsTable;
}

//------------------------------------------------------------------------------
qMRMLNodeComboBox* qSlicerPathReconstructionTableWidget::pathReconstructionNodeComboBox() const
{
  Q_D( const qSlicerPathReconstructionTableWidget );
  return d->PathReconstructionNodeComboBox;
}

//------------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setPathReconstructionNode( vtkMRMLNode* currentNode )
{
  Q_D( qSlicerPathReconstructionTableWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( currentNode );
  if ( pathReconstructionNode == d->PathReconstructionNode )
  {
    return;
  }

  this->qvtkReconnect( d->PathReconstructionNode, pathReconstructionNode, vtkCommand::ModifiedEvent, this, SLOT( updateGUIFromMRML() ) );
  this->qvtkReconnect( d->PathReconstructionNode, pathReconstructionNode, vtkMRMLPathReconstructionNode::InputDataModifiedEvent, this, SLOT( updateGUIFromMRML() ) );

  // The table should scroll to the bottom when new paths are added
  this->qvtkReconnect( d->PathReconstructionNode, pathReconstructionNode, vtkMRMLPathReconstructionNode::PathAddedEvent, d->PathsTable, SLOT( scrollToBottom() ) );

  d->PathReconstructionNode = pathReconstructionNode;

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setPathReconstructionNodeComboBoxVisible( bool visible )
{
  Q_D( qSlicerPathReconstructionTableWidget );
  d->PathReconstructionNodeComboBox->setVisible( visible );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setFittingParametersVisible( bool visible )
{
  Q_D( qSlicerPathReconstructionTableWidget );
  d->FittingParametersLabel->setVisible( visible );
  d->FittingParametersComboBox->setVisible( visible );
  d->RefitPathsButton->setVisible( visible );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::onFittingParametersAdded( vtkMRMLNode* newNode )
{
  Q_D( qSlicerPathReconstructionTableWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
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
void qSlicerPathReconstructionTableWidget::onFittingParametersChanged()
{
  Q_D( qSlicerPathReconstructionTableWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }
  
  const char* markupsToModelNodeID = NULL;
  vtkMRMLMarkupsToModelNode* markupsToModelNode = vtkMRMLMarkupsToModelNode::SafeDownCast( d->FittingParametersComboBox->currentNode() );
  if( markupsToModelNode != NULL )
  {
    markupsToModelNodeID = markupsToModelNode->GetID();
  }
  pathReconstructionNode->SetAndObserveMarkupsToModelNodeID( markupsToModelNodeID );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::onFittingColorChanged( QColor col )
{
  Q_D( qSlicerPathReconstructionTableWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
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
void qSlicerPathReconstructionTableWidget::onRefitPathsButtonClicked()
{
  Q_D( qSlicerPathReconstructionTableWidget );

  vtkMRMLPathReconstructionNode* pathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
  if ( pathReconstructionNode == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": invalid parameter node";
    return;
  }

  if ( d->PathReconstructionLogic == NULL )
  {
    qWarning() << Q_FUNC_INFO << ": invalid module logic";
    return;
  }

  d->PathReconstructionLogic->RefitAllPaths( pathReconstructionNode );
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::updateGUIFromMRML()
{
  Q_D(qSlicerPathReconstructionTableWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical() << Q_FUNC_INFO << ": Scene is invalid";
    return;
  }

  vtkMRMLPathReconstructionNode* currentPathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
  if ( currentPathReconstructionNode == NULL )
  {
    d->FittingParametersComboBox->setEnabled( false );
    d->FittingColorPicker->setEnabled( false );
    d->RefitPathsButton->setEnabled( false );
    d->PathsTable->setRowCount( 0 );
    return;
  }

  // Update the fitting parameters
  bool wasBlockedFittingColor = d->FittingColorPicker->blockSignals( true );
  d->FittingColorPicker->setEnabled( true );
  QColor pathColor;
  double pathRed = currentPathReconstructionNode->GetPathColorRed();
  double pathGreen = currentPathReconstructionNode->GetPathColorGreen();
  double pathBlue = currentPathReconstructionNode->GetPathColorBlue();
  pathColor.setRgbF( pathRed, pathGreen, pathBlue );
  d->FittingColorPicker->setColor( pathColor );
  d->FittingColorPicker->blockSignals( wasBlockedFittingColor );

  bool wasBlockedFittingParameters = d->FittingParametersComboBox->blockSignals( true );
  d->FittingParametersComboBox->setEnabled( true ); 
  vtkMRMLMarkupsToModelNode* fittingParametersNode = currentPathReconstructionNode->GetMarkupsToModelNode();
  if ( fittingParametersNode != NULL )
  {
    d->FittingParametersComboBox->setCurrentNode( fittingParametersNode );
    d->RefitPathsButton->setEnabled( true );
  }
  else
  {
    d->RefitPathsButton->setEnabled( false );
  }
  d->FittingParametersComboBox->blockSignals( wasBlockedFittingParameters );

  // Update the fiducials table
  bool wasBlockedTableWidget = d->PathsTable->blockSignals( true );

  const int suffixColumn = 0;
  const int pointsNameColumn = 1;
  const int pathNameColumn = 2;
  const int numberOfColumns = 3;

  d->PathsTable->clear();
  d->PathsTable->setColumnCount( numberOfColumns );
  d->PathsTable->setHorizontalHeaderLabels( QStringList() << "ID" << "Markups" << "Model" );
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
  d->PathsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
#else
  d->PathsTable->horizontalHeader()->setSectionResizeMode( QHeaderView::Stretch );
#endif
  d->PathsTable->setContextMenuPolicy( Qt::CustomContextMenu );
  d->PathsTable->setSelectionBehavior( QAbstractItemView::SelectRows ); // only select rows rather than cells

  vtkSmartPointer< vtkIntArray > suffixes = vtkSmartPointer< vtkIntArray >::New();
  currentPathReconstructionNode->GetSuffixes( suffixes );
  int numberOfSuffixes = suffixes->GetNumberOfTuples();
  d->PathsTable->setRowCount( numberOfSuffixes );
  for ( int suffixIndex = 0; suffixIndex < numberOfSuffixes; suffixIndex++ )
  {
    int suffix = suffixes->GetTuple1( suffixIndex );
    QTableWidgetItem* suffixItem = new QTableWidgetItem( QString::number( suffix ) );
    d->PathsTable->setItem( suffixIndex, suffixColumn, suffixItem );

    std::string pointsName;
    vtkMRMLModelNode* pointsNode = currentPathReconstructionNode->GetPointsModelNodeBySuffix( suffix );
    if ( pointsNode == NULL )
    {
      pointsName = "NULL";
    }
    else
    {
      pointsName = pointsNode->GetName();
    }
    QTableWidgetItem* pointsNameItem = new QTableWidgetItem( QString::fromStdString( pointsName ) );
    d->PathsTable->setItem( suffixIndex, pointsNameColumn, pointsNameItem );

    std::string pathName;
    vtkMRMLModelNode* pathNode = currentPathReconstructionNode->GetPathModelNodeBySuffix( suffix );
    if ( pathNode == NULL )
    {
      pathName = "NULL";
    }
    else
    {
      pathName = pathNode->GetName();
    }
    QTableWidgetItem* pathNameItem = new QTableWidgetItem( QString::fromStdString( pathName ) );
    d->PathsTable->setItem( suffixIndex, pathNameColumn, pathNameItem );
  }

  d->PathsTable->blockSignals( wasBlockedTableWidget );
}

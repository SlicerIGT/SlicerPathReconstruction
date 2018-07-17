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

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLModelNode.h>
#include "vtkMRMLPathReconstructionNode.h"

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
  connect( d->PathReconstructionNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( updateGUIFromMRML() ) );
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

  // This widget only needs to update if the nodes are changed, so we only check for Modified event
  this->qvtkReconnect( d->PathReconstructionNode, pathReconstructionNode, vtkCommand::ModifiedEvent, this, SLOT( updateGUIFromMRML() ) );

  // The table should scroll to the bottom when new paths are added
  this->qvtkReconnect( d->PathReconstructionNode, pathReconstructionNode, vtkMRMLPathReconstructionNode::PathAddedEvent, d->PathsTable, SLOT( scrollToBottom() ) );

  d->PathReconstructionNode = pathReconstructionNode;

  this->updateGUIFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::setPathReconstructionNodeComboBoxVisible( bool visible )
{
  Q_D( qSlicerPathReconstructionTableWidget );
  d->PathReconstructionNodeComboBox->setVisible(visible);
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionTableWidget::updateGUIFromMRML()
{
  Q_D(qSlicerPathReconstructionTableWidget);

  if ( this->mrmlScene() == NULL )
  {
    qCritical("Scene is invalid");
  }

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

  vtkMRMLPathReconstructionNode* currentPathReconstructionNode = vtkMRMLPathReconstructionNode::SafeDownCast( d->PathReconstructionNode );
  if ( currentPathReconstructionNode == NULL )
  {
    d->PathsTable->setRowCount( 0 );
    return;
  }

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

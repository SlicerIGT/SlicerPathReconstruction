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
#include <QtPlugin>

// PathReconstruction Logic includes
#include <vtkSlicerPathReconstructionLogic.h>

// PathReconstruction includes
#include "qSlicerPathReconstructionModule.h"
#include "qSlicerPathReconstructionModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerPathReconstructionModule, qSlicerPathReconstructionModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_PathReconstruction
class qSlicerPathReconstructionModulePrivate
{
public:
  qSlicerPathReconstructionModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPathReconstructionModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPathReconstructionModulePrivate::qSlicerPathReconstructionModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPathReconstructionModule methods

//-----------------------------------------------------------------------------
qSlicerPathReconstructionModule::qSlicerPathReconstructionModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPathReconstructionModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathReconstructionModule::categories()const
{
  return QStringList() << "IGT";
}

//-----------------------------------------------------------------------------
qSlicerPathReconstructionModule::~qSlicerPathReconstructionModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPathReconstructionModule::helpText()const
{
  return "For help on how to use this module visit: <a href='http://www.slicerigt.org/'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerPathReconstructionModule::acknowledgementText()const
{
  return "This work was was funded by an Ontario Graduate Scholarship, and by a CANARIE grant.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathReconstructionModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Thomas Vaughan (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerPathReconstructionModule::dependencies()const
{
	QStringList dependencies;
	dependencies << QString("SlicerIGT");
	dependencies << QString("MarkupsToModel");
	return dependencies;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPathReconstructionModule::icon()const
{
  return QIcon(":/Icons/PathReconstruction.png");
}

//-----------------------------------------------------------------------------
void qSlicerPathReconstructionModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPathReconstructionModule::createWidgetRepresentation()
{
  return new qSlicerPathReconstructionModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPathReconstructionModule::createLogic()
{
  return vtkSlicerPathReconstructionLogic::New();
}

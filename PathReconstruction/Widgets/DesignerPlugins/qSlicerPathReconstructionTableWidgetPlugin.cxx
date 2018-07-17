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

#include "qSlicerPathReconstructionTableWidgetPlugin.h"
#include "qSlicerPathReconstructionTableWidget.h"

//-----------------------------------------------------------------------------
qSlicerPathReconstructionTableWidgetPlugin::
  qSlicerPathReconstructionTableWidgetPlugin(QObject *newParent) : QObject(newParent)
{
}

//-----------------------------------------------------------------------------
QWidget *qSlicerPathReconstructionTableWidgetPlugin::createWidget(QWidget *newParent)
{
  qSlicerPathReconstructionTableWidget* newWidget = new qSlicerPathReconstructionTableWidget(newParent);
  return newWidget;
}

//-----------------------------------------------------------------------------
QString qSlicerPathReconstructionTableWidgetPlugin::domXml() const
{
  return "<widget class=\"qSlicerPathReconstructionTableWidget\" \
          name=\"PathsTableWidget\">\n"
          " <property name=\"geometry\">\n"
          "  <rect>\n"
          "   <x>0</x>\n"
          "   <y>0</y>\n"
          "   <width>200</width>\n"
          "   <height>200</height>\n"
          "  </rect>\n"
          " </property>\n"
          "</widget>\n";
}

//-----------------------------------------------------------------------------
QIcon qSlicerPathReconstructionTableWidgetPlugin::icon() const
{
  return QIcon(":/Icons/table.png");
}

//-----------------------------------------------------------------------------
QString qSlicerPathReconstructionTableWidgetPlugin::includeFile() const
{
  return "qSlicerPathReconstructionTableWidget.h";
}

//-----------------------------------------------------------------------------
bool qSlicerPathReconstructionTableWidgetPlugin::isContainer() const
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerPathReconstructionTableWidgetPlugin::name() const
{
  return "qSlicerPathReconstructionTableWidget";
}

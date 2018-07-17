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

#ifndef __qSlicerPathReconstructionTableWidgetPlugin_h
#define __qSlicerPathReconstructionTableWidgetPlugin_h

#include "qSlicerPathReconstructionModuleWidgetsAbstractPlugin.h"

class Q_SLICER_QTMODULES_PATHRECONSTRUCTION_WIDGETS_PLUGINS_EXPORT qSlicerPathReconstructionTableWidgetPlugin
  : public QObject
  , public qSlicerPathReconstructionModuleWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerPathReconstructionTableWidgetPlugin(QObject * newParent = 0);

  QWidget *createWidget(QWidget *newParent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;

};

#endif

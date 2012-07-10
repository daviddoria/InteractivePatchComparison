/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "TableModelTopPatches.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

TableModelTopPatches::TableModelTopPatches(const std::vector<SelfPatchCompare::PatchDataType>& patchData, QObject * parent) :
    QAbstractTableModel(parent), PatchDisplaySize(20), MaxTopPatchesToDisplay(0), TopPatchData(patchData), Image(NULL)
{
}

void TableModelTopPatches::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags TableModelTopPatches::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

int TableModelTopPatches::rowCount(const QModelIndex& parent) const
{
  unsigned int numberOfRowsToDisplay = std::min(this->TopPatchData.size(), this->MaxTopPatchesToDisplay);

  return numberOfRowsToDisplay;
}

int TableModelTopPatches::columnCount(const QModelIndex& parent) const
{
  return 3;
}

void TableModelTopPatches::SetMaxTopPatchesToDisplay(const unsigned int maxTopPatchesToDisplay)
{
  this->MaxTopPatchesToDisplay = maxTopPatchesToDisplay;
}

QVariant TableModelTopPatches::data(const QModelIndex& index, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole && index.row() >= 0)
    {
    itk::ImageRegion<2> sourceRegion = this->TopPatchData[index.row()].first;
    //std::cout << "sourceRegion: " << index.row() << " " << sourceRegion << std::endl;
    switch(index.column())
      {
      case 0:
        {
        QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, sourceRegion);

        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

        //std::cout << "patchImage.size: w=" << patchImage.size().width() << "h=" << patchImage.size().height() << std::endl;

        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      case 1:
        {
        // returnValue = index.row(); // This is the id
        returnValue = this->TopPatchData[index.row()].second;
        break;
        }
//       case 2:
//         {
//         returnValue = ITKHelpers::GetIndexString(sourceRegion.GetIndex()).c_str();
//         break;
//         }
      case 2:
        {
        returnValue = ClusterIDs[index.row()];
        break;
        }
      } // end switch

    } // end if DisplayRole

  return returnValue;
}

QVariant TableModelTopPatches::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole)
    {
    if(orientation == Qt::Horizontal)
      {
      switch(section)
        {
        case 0:
          returnValue = "Patch";
          break;
        case 1:
          returnValue = "Score";
          break;
//         case 2:
//           returnValue = "Location";
//           break;
        case 2:
          returnValue = "Cluster";
          break;
        } // end switch
      }// end Horizontal orientation
    } // end DisplayRole

  return returnValue;
}

void TableModelTopPatches::Refresh()
{
  beginResetModel();
  endResetModel();
}

void TableModelTopPatches::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}

void TableModelTopPatches::SetImage(ImageType* const image)
{
  this->Image = image;
}

void TableModelTopPatches::SetTopPatchData(const std::vector<SelfPatchCompare::PatchDataType>& topPatchData)
{
  this->TopPatchData = topPatchData;
  this->ClusterIDs.resize(this->TopPatchData.size(), 0);
  Refresh();
}

std::vector<SelfPatchCompare::PatchDataType> TableModelTopPatches::GetTopPatchData()
{
  return this->TopPatchData;
}

void TableModelTopPatches::SetClusterIDs(const std::vector<unsigned int>& clusterIDs)
{
  this->ClusterIDs = clusterIDs;
}

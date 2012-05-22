/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
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

TableModelTopPatches::TableModelTopPatches(QObject * parent) :
    QAbstractTableModel(parent), MaxTopPatchesToDisplay(100), Image(NULL)
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
  unsigned int numberOfRowsToDisplay = std::min(this->TopPatchRegions.size(), this->MaxTopPatchesToDisplay);

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
    itk::ImageRegion<2> sourceRegion = this->TopPatchRegions[index.row()];

    switch(index.column())
      {
      case 0:
        {
        QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, sourceRegion);

        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);

        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      case 1:
        {
        returnValue = index.row();
        break;
        }
      case 2:
        {
        returnValue = ITKHelpers::GetIndexString(sourceRegion.GetIndex()).c_str();
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
          returnValue = "Id";
          break;
        case 2:
          returnValue = "Location";
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

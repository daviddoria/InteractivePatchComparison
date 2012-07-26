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

#ifndef TableModelTopPatches_HPP
#define TableModelTopPatches_HPP

#include "TableModelTopPatches.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

template <typename TImage>
TableModelTopPatches<TImage>::TableModelTopPatches(
    const std::vector<typename SelfPatchCompare<TImage>::PatchDataType>& patchData, QObject * parent) :
    QAbstractTableModel(parent), PatchDisplaySize(20), MaxTopPatchesToDisplay(0),
    TopPatchData(patchData), Image(NULL)
{
}

template <typename TImage>
void TableModelTopPatches<TImage>::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

template <typename TImage>
Qt::ItemFlags TableModelTopPatches<TImage>::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

template <typename TImage>
int TableModelTopPatches<TImage>::rowCount(const QModelIndex& parent) const
{
  unsigned int numberOfRowsToDisplay = std::min(this->TopPatchData.size(), this->MaxTopPatchesToDisplay);

  return numberOfRowsToDisplay;
}

template <typename TImage>
int TableModelTopPatches<TImage>::columnCount(const QModelIndex& parent) const
{
  return 2;
}

template <typename TImage>
void TableModelTopPatches<TImage>::SetMaxTopPatchesToDisplay(const unsigned int maxTopPatchesToDisplay)
{
  this->MaxTopPatchesToDisplay = maxTopPatchesToDisplay;
}

template <typename TImage>
QVariant TableModelTopPatches<TImage>::data(const QModelIndex& index, int role) const
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

        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      case 1:
        {
        // returnValue = index.row(); // This is the id
        returnValue = this->TopPatchData[index.row()].second;
        break;
        }
      } // end switch

    } // end if DisplayRole

  return returnValue;
}

template <typename TImage>
QVariant TableModelTopPatches<TImage>::headerData(int section, Qt::Orientation orientation, int role) const
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
        } // end switch
      }// end Horizontal orientation
    } // end DisplayRole

  return returnValue;
}

template <typename TImage>
void TableModelTopPatches<TImage>::Refresh()
{
  beginResetModel();
  endResetModel();
}

template <typename TImage>
void TableModelTopPatches<TImage>::selectionChanged(const QItemSelection& selected,
                                            const QItemSelection& deselected)
{
  //std::cout << "TopPatchesTableModel::selectionChanged()" << std::endl;
}

template <typename TImage>
void TableModelTopPatches<TImage>::SetImage(TImage* const image)
{
  this->Image = image;
}

template <typename TImage>
void TableModelTopPatches<TImage>::SetTopPatchData(
  const std::vector<typename SelfPatchCompare<TImage>::PatchDataType>& topPatchData)
{
  this->TopPatchData = topPatchData;

  Refresh();
}

template <typename TImage>
std::vector<typename SelfPatchCompare<TImage>::PatchDataType>
TableModelTopPatches<TImage>::GetTopPatchData()
{
  return this->TopPatchData;
}

#endif

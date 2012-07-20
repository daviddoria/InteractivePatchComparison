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

#include "TableModelViewAllMatches.h"

// Qt
#include <QLabel>
#include <QAbstractItemView>

// Custom
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

TableModelViewAllMatches::TableModelViewAllMatches(QObject * parent) :
QAbstractTableModel(parent), PatchDisplaySize(20), MaxPairsToDisplay(0), Image(NULL)
{
}

TableModelViewAllMatches::TableModelViewAllMatches(ImageType* const image,
    const std::vector<PairType>& allPairs, QObject * parent) :
    QAbstractTableModel(parent), PatchDisplaySize(20), MaxPairsToDisplay(0),
    AllPairs(allPairs), Image(image)
{
  std::cout << "There are " << this->AllPairs.size() << " pairs." << std::endl;
}

void TableModelViewAllMatches::SetPatchDisplaySize(const unsigned int value)
{
  this->PatchDisplaySize = value;
}

Qt::ItemFlags TableModelViewAllMatches::flags(const QModelIndex& index) const
{
  //Qt::ItemFlags itemFlags = (!Qt::ItemIsEditable) | Qt::ItemIsSelectable | Qt::ItemIsEnabled | (!Qt::ItemIsUserCheckable) | (!Qt::ItemIsTristate);
  Qt::ItemFlags itemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  return itemFlags;
}

int TableModelViewAllMatches::rowCount(const QModelIndex& parent) const
{
  unsigned int numberOfRowsToDisplay = std::min(this->AllPairs.size(), this->MaxPairsToDisplay);

  //std::cout << "There are " << numberOfRowsToDisplay << " rows to display." << std::endl;
  return numberOfRowsToDisplay;
}

int TableModelViewAllMatches::columnCount(const QModelIndex& parent) const
{
  return 3;
}

void TableModelViewAllMatches::SetMaxPairsToDisplay(const unsigned int maxPairsToDisplay)
{
  this->MaxPairsToDisplay = maxPairsToDisplay;
  Refresh();
}

QVariant TableModelViewAllMatches::data(const QModelIndex& index, int role) const
{
  //std::cout << "data()" << std::endl;
  QVariant returnValue;

  if(role == Qt::DisplayRole && index.row() >= 0 &&
     index.row() < static_cast<int>(this->AllPairs.size()) &&
     index.row() < static_cast<int>(this->MaxPairsToDisplay))
    {
    switch(index.column())
      {
      case 0:
        {
        returnValue = index.row();
        break;
        }
      case 1:
        {
        itk::ImageRegion<2> targetRegion = this->AllPairs[index.row()].first;
        //std::cout << "Target region: " << targetRegion << std::endl;
        QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, targetRegion);
        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);
        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      case 2:
        {
        itk::ImageRegion<2> sourceRegion = this->AllPairs[index.row()].second;
        //std::cout << "Source region: " << sourceRegion << std::endl;
        QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, sourceRegion);
        patchImage = patchImage.scaledToHeight(this->PatchDisplaySize);
        returnValue = QPixmap::fromImage(patchImage);
        break;
        }
      } // end switch

    } // end if DisplayRole

  return returnValue;
}

QVariant TableModelViewAllMatches::headerData(int section, Qt::Orientation orientation, int role) const
{
  QVariant returnValue;
  if(role == Qt::DisplayRole)
    {
    if(orientation == Qt::Horizontal)
      {
      switch(section)
        {
        case 0:
          returnValue = "ID";
          break;
        case 1:
          returnValue = "Target";
          break;
        case 2:
          returnValue = "Source";
          break;
        } // end switch
      }// end Horizontal orientation
    } // end DisplayRole

  return returnValue;
}

void TableModelViewAllMatches::Refresh()
{
  beginResetModel();
  endResetModel();
}

void TableModelViewAllMatches::SetImage(ImageType* const image)
{
  this->Image = image;
  Refresh();
}

void TableModelViewAllMatches::SetTopPatchData(const std::vector<PairType>& topPatchData)
{
  this->AllPairs = topPatchData;

  Refresh();
}

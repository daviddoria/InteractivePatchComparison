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

#ifndef TableModelTopPatches_H
#define TableModelTopPatches_H

// Qt
#include <QAbstractTableModel>
#include <QItemSelection>

// ITK
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// STL
#include <vector>

// Custom
#include "PatchComparison/SelfPatchCompare.h"

template <typename TImage>
class TableModelTopPatches : public QAbstractTableModel
{
public:
  /** Constructor.*/
  TableModelTopPatches(const std::vector<typename SelfPatchCompare<TImage>::PatchDataType>& patchData,
                       QObject * parent);

  /** Return the number of rows.*/
  int rowCount(const QModelIndex& parent) const;

  /** Return the number of columns.*/
  int columnCount(const QModelIndex& parent) const;

  /** Display the data.*/
  QVariant data(const QModelIndex& index, int role) const;

  /** Display the headers.*/
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  /** Set the properties of the displayed items.*/
  Qt::ItemFlags flags(const QModelIndex& index) const;

  /** Set the maximum number of patches to display. This function is not just called SetTopPatchesToDisplay
    * because if the number of total patches is less than this, the number of total patches is used as the number of
    * patches to display. */
  void SetMaxTopPatchesToDisplay(const unsigned int maxTopPatchesToDisplay);

  /** Respond when the user clicks a row.*/
  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  /** Refresh the widget.*/
  void Refresh();

  /** Set the size to display the patches.*/
  void SetPatchDisplaySize(const unsigned int value);

  /** Set the image from which to get the patches.*/
  void SetImage(TImage* const image);

  /** Set the data for the top patches.*/
  void SetTopPatchData(const std::vector<typename SelfPatchCompare<TImage>::PatchDataType>& topPatchData);

  /** Get the data for the top patches.*/
  std::vector<typename SelfPatchCompare<TImage>::PatchDataType> GetTopPatchData();

private:

  /** The size to draw the patches in the table. */
  // TODO: This should be set by the size of the target patch, or a multiplier, or something
  unsigned int PatchDisplaySize;

  /** The number of patches to display.*/
  unsigned int MaxTopPatchesToDisplay;

  /** The data for the top patches.*/
  std::vector<typename SelfPatchCompare<TImage>::PatchDataType> TopPatchData;

  /** The image from which to get the patches.*/
  TImage* Image;
};

#include "TableModelTopPatches.hpp"

#endif

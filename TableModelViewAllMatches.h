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

class TableModelTopPatches : public QAbstractTableModel
{
public:
  typedef itk::VectorImage<float, 2> ImageType;
  
  //TableModelTopPatches(QObject * parent);
  TableModelTopPatches(const std::vector<SelfPatchCompare<ImageType>::PatchDataType>& patchData,
                       QObject * parent);

  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void SetMaxTopPatchesToDisplay(const unsigned int maxTopPatchesToDisplay);

  void selectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

  void Refresh();

  void SetPatchDisplaySize(const unsigned int value);

  void SetImage(ImageType* const image);

  void SetTopPatchData(const std::vector<SelfPatchCompare<ImageType>::PatchDataType>& topPatchData);

  void SetClusterIDs(const std::vector<unsigned int>& clusterIDs);

  std::vector<SelfPatchCompare<ImageType>::PatchDataType> GetTopPatchData();

private:

  /** This is how big to draw the patches in the table. */
  // TODO: This should be set by the size of the target patch, or a multiplier, or something
  unsigned int PatchDisplaySize;

  unsigned int MaxTopPatchesToDisplay;

  std::vector<SelfPatchCompare<ImageType>::PatchDataType> TopPatchData;

  std::vector<unsigned int> ClusterIDs;

  ImageType* Image;
};

#endif

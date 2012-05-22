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
#include "SelfPatchCompare.h"

class TableModelTopPatches : public QAbstractTableModel
{
public:
  typedef itk::VectorImage<float, 2> ImageType;
  
  TableModelTopPatches(QObject * parent);

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

  //void SetTopPatchRegions(const std::vector<itk::ImageRegion<2> >& topPatchRegions);
  void SetTopPatchData(const std::vector<SelfPatchCompare::PatchDataType>& topPatchData);
  
private:

  unsigned int PatchDisplaySize;

  unsigned int MaxTopPatchesToDisplay;

  //std::vector<itk::ImageRegion<2> > TopPatchRegions;
  std::vector<SelfPatchCompare::PatchDataType> TopPatchData;

  ImageType* Image;
};

#endif

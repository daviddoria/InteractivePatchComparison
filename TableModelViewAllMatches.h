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

#ifndef TableModelViewAllMatches_H
#define TableModelViewAllMatches_H

// Qt
#include <QAbstractTableModel>
#include <QItemSelection>

// ITK
#include "itkImageRegion.h"
#include "itkVectorImage.h"

// STL
#include <vector>

class TableModelViewAllMatches : public QAbstractTableModel
{
public:
  typedef itk::VectorImage<unsigned char, 2> ImageType;

  typedef std::pair<itk::ImageRegion<2>, itk::ImageRegion<2> > PairType;
  
  TableModelViewAllMatches(ImageType* const image, const std::vector<PairType>& allPairs,
                           QObject * parent);

  int rowCount(const QModelIndex& parent) const;
  int columnCount(const QModelIndex& parent) const;
  QVariant data(const QModelIndex& index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  Qt::ItemFlags flags(const QModelIndex& index) const;

  void SetMaxPairsToDisplay(const unsigned int maxPairsToDisplay);

  void Refresh();

  void SetPatchDisplaySize(const unsigned int value);

  void SetImage(ImageType* const image);

  void SetTopPatchData(const std::vector<PairType>& allPairs);

private:

  /** This is how big to draw the patches in the table. */
  // TODO: This should be set by the size of the target patch, or a multiplier, or something
  unsigned int PatchDisplaySize;

  unsigned int MaxPairsToDisplay;

  std::vector<PairType> AllPairs;

  ImageType* Image;
};

#endif

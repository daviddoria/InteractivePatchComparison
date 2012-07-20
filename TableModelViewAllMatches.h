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
  /** The type of the image data. */
  typedef itk::VectorImage<unsigned char, 2> ImageType;

  /** The type of the pair data. */
  typedef std::pair<itk::ImageRegion<2>, itk::ImageRegion<2> > PairType;

  /** Default constructor. */
  TableModelViewAllMatches(QObject * parent);

  /** Constructor with all necessary information. */
  TableModelViewAllMatches(ImageType* const image, const std::vector<PairType>& allPairs,
                           QObject * parent);

  /** Report the number of rows to display */
  int rowCount(const QModelIndex& parent) const;

  /** Report the number of columns to display. */
  int columnCount(const QModelIndex& parent) const;

  /** Draw the patches in the table. */
  QVariant data(const QModelIndex& index, int role) const;

  /** Setup the column headers. */
  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

  /** Setup the properties of the cells */
  Qt::ItemFlags flags(const QModelIndex& index) const;

  /** Set the maximum number of pairs to display */
  void SetMaxPairsToDisplay(const unsigned int maxPairsToDisplay);

  /** Refresh the view. */
  void Refresh();

  /** Set the size of the patches to display */
  void SetPatchDisplaySize(const unsigned int value);

  /** Set the image from which to extract the patches */
  void SetImage(ImageType* const image);

  /** Set the pairs to display */
  void SetTopPatchData(const std::vector<PairType>& allPairs);

private:

  /** This is how big to draw the patches in the table. */
  // TODO: This should be set by the size of the target patch, or a multiplier, or something
  unsigned int PatchDisplaySize;

  /** The max number of pairs to display */
  unsigned int MaxPairsToDisplay;

  /** The pairs to display */
  std::vector<PairType> AllPairs;

  /** The image from which to extract the pair regions. */
  ImageType* Image;
};

#endif

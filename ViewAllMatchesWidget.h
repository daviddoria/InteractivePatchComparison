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

#ifndef ViewAllMatchesWidget_H
#define ViewAllMatchesWidget_H

#include "ui_ViewAllMatchesWidget.h"

// Qt
#include <QDialog>
#include <QObject>

// ITK
#include "itkVectorImage.h"

// Submodules
#include "Mask/Mask.h"

// Custom
#include "TableModelViewAllMatches.h"

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class ViewAllMatchesWidget : public QWidget, public Ui::ViewAllMatchesWidget
{
Q_OBJECT

public:

  typedef itk::VectorImage<unsigned char, 2> ImageType;

  ViewAllMatchesWidget(QWidget* parent = NULL);
  ViewAllMatchesWidget(const std::string& imageFileName, const std::string& matchFileName,
                       const unsigned int patchRadius, QWidget* parent = NULL);
  void SharedConstructor();

public slots:

  /** When a patch (or patches) is clicked or the arrow keys are used, emit a signal. */
  void slot_SelectionChanged(const QItemSelection &, const QItemSelection &);

private:
  ImageType::Pointer Image;

  typedef std::pair<itk::ImageRegion<2>, itk::ImageRegion<2> > PairType;
  std::vector<PairType> Pairs;

  // TODO: this should be added to helpers
  std::istream& MyIgnore(std::istream& ss);

  TableModelViewAllMatches* TableModel;
};

#endif // ViewAllMatchesWidget_H

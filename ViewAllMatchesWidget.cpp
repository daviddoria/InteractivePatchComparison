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

#include "ViewAllMatchesWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QLineEdit>
#include <QProgressDialog>
#include <QSortFilterProxyModel>

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

// Custom
#include "PixmapDelegate.h"

// Boost
#include <boost/regex.hpp>

ViewAllMatchesWidget::ViewAllMatchesWidget(const std::string& imageFileName,
                                           const std::string& matchFileName,
                                           const unsigned int patchRadius, QWidget* parent) : QWidget(parent)
{
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  this->Image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), this->Image.GetPointer());
  std::cout << "Image size: " << this->Image->GetLargestPossibleRegion().GetSize() << std::endl;

  // Parse matches list
  std::ifstream fin(matchFileName.c_str());

  if(fin == NULL)
  {
    std::stringstream ss;
    ss << "Cannot open file " << matchFileName;
    throw std::runtime_error(ss.str());
  }

  std::string line;

  itk::Size<2> patchSize = {{patchRadius*2 + 1, patchRadius*2 + 1}};
  while(getline(fin, line))
  {
    //std::cout << "Current line: " << line << std::endl;

    // Setup the expression for a [x,y]
    // The real regex is \[\s*[0-9]*\s*,\s*[0-9]*\s*\] which means match a [,
    // then any number of whitespace, then any number of digits, then any number of white space,
    // then a comma, then any number of whitespace, then any number of digits, then a ]
    // The following has extra escape characters (\) for c++
    // Additionally, the things we want to extract (the digits) are in subexpressions (parenthesis)
    std::string myRegExString = "\\[\\s*([0-9]*)\\s*,\\s*([0-9]*)\\s*\\]";

    const boost::regex myRegEx(myRegExString);

    boost::match_results<std::string::const_iterator> matches;

    std::string::const_iterator beginSearch = line.begin();
    // Cannot just put this in the regex_search() call, aparently .end() does not return
    // a const_iterator unless it is explicitly assigned to one?
    std::string::const_iterator endSearch = line.end();

    // Find the first match (target)
    boost::regex_search(beginSearch, endSearch, matches, myRegEx);

    std::string matchStringTargetX(matches[1].first, matches[1].second);
    std::string matchStringTargetY(matches[2].first, matches[2].second);

    std::stringstream matchSSTargetX(matchStringTargetX);
    std::stringstream matchSSTargetY(matchStringTargetY);

    itk::Index<2> targetIndex;
    matchSSTargetX >> targetIndex[0];
    matchSSTargetY >> targetIndex[1];

    itk::ImageRegion<2> targetRegion(targetIndex, patchSize);

    // Find the second match (source)
    beginSearch = matches[0].second;
    boost::regex_search(beginSearch, endSearch, matches, myRegEx);

    std::string matchStringSourceX(matches[1].first, matches[1].second);
    std::string matchStringSourceY(matches[2].first, matches[2].second);

    std::stringstream matchSSSourceX(matchStringSourceX);
    std::stringstream matchSSSourceY(matchStringSourceY);

    itk::Index<2> sourceIndex;
    matchSSSourceX >> sourceIndex[0];
    matchSSSourceY >> sourceIndex[1];

    itk::ImageRegion<2> sourceRegion(sourceIndex, patchSize);

    PairType p(targetRegion, sourceRegion);
    this->Pairs.push_back(p);
  } // end while

  std::cout << "There are " << this->Pairs.size() << " pairs." << std::endl;
  this->TableModel = new TableModelViewAllMatches(this->Image, this->Pairs, this);

  SharedConstructor();
}

ViewAllMatchesWidget::ViewAllMatchesWidget(QWidget* parent) : QWidget(parent), Image(NULL)
{
  this->TableModel = new TableModelViewAllMatches(this);
  SharedConstructor();
}

void ViewAllMatchesWidget::SharedConstructor()
{
  this->setupUi(this);
  // Make the cells fit the images (based on the sizeHint from the PixmapDelegate)
  this->tblviewAllMatches->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  this->tblviewAllMatches->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  this->tblviewAllMatches->setModel(this->TableModel);
  this->TableModel->SetMaxPairsToDisplay(500000);

//   std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
//   this->TableModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;
  this->tblviewAllMatches->setItemDelegate(pixmapDelegate);

  connect(this->tblviewAllMatches->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(slot_SelectionChanged(const QItemSelection &, const QItemSelection &)));
}

std::istream& ViewAllMatchesWidget::MyIgnore(std::istream& ss)
{
  ss.ignore(std::numeric_limits<std::streamsize>::max(),':');
  return ss;
}

void ViewAllMatchesWidget::slot_SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  std::cout << selected.indexes()[0].row() << std::endl;
}

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

ViewAllMatchesWidget::ViewAllMatchesWidget(const std::string& imageFileName,
                                           const std::string& matchFileName, QWidget* parent) : QWidget(parent)
{
  typedef itk::ImageFileReader<ImageType> ImageReaderType;
  ImageReaderType::Pointer imageReader = ImageReaderType::New();
  imageReader->SetFileName(imageFileName);
  imageReader->Update();

  this->Image = ImageType::New();
  ITKHelpers::DeepCopy(imageReader->GetOutput(), this->Image.GetPointer());

  // Parse matches list
  std::ifstream fin(matchFileName.c_str());

  if(fin == NULL)
  {
    std::stringstream ss;
    ss << "Cannot open file " << matchFileName;
    throw std::runtime_error(ss.str());
  }

  std::string line;

  while(getline(fin, line))
  {
    std::stringstream ss;
    ss << line;
    while(!ss.rdbuf()->in_avail()) // while there are more characters to read
    {
      std::string test;
      ss >> test;
      std::cout << test << std::endl;
    }
//     PairType p(targetRegion, sourceRegion);
//     this->Pairs.push_back(p);
  }

  
  SharedConstructor();
}

ViewAllMatchesWidget::ViewAllMatchesWidget(QWidget* parent) : QWidget(parent), Image(NULL)
{
  SharedConstructor();
}

void ViewAllMatchesWidget::SharedConstructor()
{
  this->setupUi(this);
  // Make the cells fit the images (based on the sizeHint from the PixmapDelegate)
//   this->tblviewTopPatches->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
//   this->tblviewTopPatches->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
// 
//   this->TargetPatchItem = new QGraphicsPixmapItem;
//   this->TargetPatchScene = new QGraphicsScene();
//   this->gfxTargetPatch->setScene(this->TargetPatchScene);
// 
//   this->ProxyModel = new QSortFilterProxyModel;
//   this->TopPatchesModel = new TableModelTopPatches(this->TopPatchData, this);
//   this->ProxyModel->setSourceModel(this->TopPatchesModel);
//   this->tblviewTopPatches->setModel(ProxyModel);
//   this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->spinNumberOfBestPatches->value());
// 
//   std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
//   this->TopPatchesModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());
// 
//   PixmapDelegate* pixmapDelegate = new PixmapDelegate;
// 
//   this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

}

std::istream& ViewAllMatchesWidget::MyIgnore(std::istream& ss)
{
  ss.ignore(std::numeric_limits<std::streamsize>::max(),':');
  return ss;
}

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

// #include "ui_InteractiveBestPatchesWidget.h"

#include "TopPatchesWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>

// Custom
// #include "Types.h"

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKHelpers/ITKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "Mask/MaskOperations.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

const unsigned char TopPatchesWidget::Green[3] = {0,255,0};
const unsigned char TopPatchesWidget::Red[3] = {255,0,0};

// Constructor
TopPatchesWidget::TopPatchesWidget(QWidget* parent) : QWidget(parent)
{
  this->setupUi(this);

  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTarget->setScene(TargetPatchScene);

  this->Image = NULL;
  this->MaskImage = NULL;

  this->tableWidget->resizeColumnsToContents();
};

void TopPatchesWidget::on_txtPatchRadius_returnPressed()
{
  SetupPatches();
}

void TopPatchesWidget::SetupPatches()
{
  Refresh();
}

void TopPatchesWidget::RefreshSlot()
{
  Refresh();
}

void TopPatchesWidget::Refresh()
{

}

void TopPatchesWidget::on_txtNumberOfPatches_returnPressed()
{
  DisplaySourcePatches();
  Refresh();
}

void TopPatchesWidget::DisplaySourcePatches()
{
  
  unsigned int numberOfPatches = this->txtNumberOfPatches->text().toUInt();
  
  if(numberOfPatches > this->PatchCompare.SourcePatches.size())
    {
    std::cout << "You have requested more patches (" << numberOfPatches << ") than have been computed (" << this->PatchCompare.SourcePatches.size() << ")" << std::endl;
    return;
    }
    
  // Clear the table
  this->tableWidget->setRowCount(0);
  
  for(unsigned int i = 0; i < numberOfPatches; ++i)
    {
    this->tableWidget->insertRow(this->tableWidget->rowCount());
  
    Patch currentPatch = this->PatchCompare.SourcePatches[i];
    std::cout << "Generating table row for " << currentPatch.Region << std::endl;

    QImage sourceImage = GetQImage(currentPatch.Region);
  
    ClickableLabel* imageLabel = new ClickableLabel;
    imageLabel->setPixmap(QPixmap::fromImage(sourceImage));
    imageLabel->Id = i; // This is the ith best match
    imageLabel->setScaledContents(false);
    this->tableWidget->setCellWidget(i,0,imageLabel);

    connect( imageLabel, SIGNAL( ClickedSignal(unsigned int) ), this, SLOT(PatchClickedSlot(unsigned int)) );

    std::stringstream ssLabel;
    ssLabel << "( " << currentPatch.Region.GetIndex()[0] << ", " << currentPatch.Region.GetIndex()[1] << ")";

    QTableWidgetItem* indexLabel = new QTableWidgetItem;
    indexLabel->setText(ssLabel.str().c_str());
    this->tableWidget->setItem(i,1,indexLabel);

    // Total absolute score
    QTableWidgetItem* totalAbsoluteScoreLabel = new QTableWidgetItem;
    totalAbsoluteScoreLabel->setData(Qt::DisplayRole, currentPatch.TotalAbsoluteScore);
    this->tableWidget->setItem(i,2,totalAbsoluteScoreLabel);

    }
    
  this->tableWidget->resizeRowsToContents();
  this->tableWidget->resizeColumnsToContents();
  
}

void TopPatchesWidget::on_btnCompute_clicked()
{
  this->PatchCompare.SetImage(this->Image);
  this->PatchCompare.SetMask(this->MaskImage);

  // This checks to see if both the image and mask have been set to something non-NULL
  if(!this->PatchCompare.IsReady())
    {
    std::cout << "Not ready to compute! Image, MaskImage or NumberOfComponentsPerPixel\
                  may not be set on the PatchCompare object!" << std::endl;
    return;
    }

  this->PatchCompare.ComputePatchScores();
  
  DisplaySourcePatches();

  // Automatically display the best patch
  PatchClickedSlot(0);
}

void TopPatchesWidget::on_chkFillPatch_clicked()
{
  PatchClickedSlot(this->DisplayedSourcePatch);
}

void TopPatchesWidget::PatchClickedSlot(const unsigned int value)
{
  // 'value' here is the "ith best match". E.g. the third best match would have value=2.
  this->DisplayedSourcePatch = value;
  
  std::cout << "PatchClickedSlot " << value << std::endl;
  
//   Patch patch = this->PatchCompare.SourcePatches[value];
//   
//   std::cout << "Region: " << patch.Region << std::endl;

  Refresh();
}

void TopPatchesWidget::SetTargetRegion(const itk::ImageRegion<2>& region)
{
  std::cout << "SetTargetRegion to " << region << std::endl;
  this->TargetRegion = region;

  QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, region);
  //patchImage = QtHelpers::FitToGraphicsView(patchImage, this->gfxTarget);

  QPixmap pixmap = QPixmap::fromImage(patchImage);

  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTarget->setScene(TargetPatchScene);
  
  this->TargetPatchScene->addPixmap(pixmap);
  
  Refresh();
}

void TopPatchesWidget::SetImage(ImageType* const image)
{
  this->Image = image;
}

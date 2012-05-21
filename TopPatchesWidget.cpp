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
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkImageSliceMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Custom
#include "SwitchBetweenStyle.h"
#include "Types.h"

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKHelpers/ITKHelpers.h"
#include "VTKHelpers/VTKHelpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "Mask/MaskOperations.h"

const unsigned char TopPatchesWidget::Green[3] = {0,255,0};
const unsigned char TopPatchesWidget::Red[3] = {255,0,0};


TopPatchesWidget::TopPatchesWidget(const std::string& imageFileName, const std::string& maskFileName)
{
  SharedConstructor();
}

// Constructor
TopPatchesWidget::TopPatchesWidget(QWidget* parent)
{
  SharedConstructor();
}

void TopPatchesWidget::SharedConstructor()
{
  this->setupUi(this);
  
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTarget->setScene(TargetPatchScene);
  
  this->PatchScale = 5;

  this->Image = NULL;
  this->MaskImage = NULL;

  this->tableWidget->resizeColumnsToContents();
};

void TopPatchesWidget::on_txtPatchRadius_returnPressed()
{
  SetupPatches();
}

void TopPatchesWidget::PositionTarget()
{
//   double position[3];
//   this->TargetPatchSlice->GetPosition(position);
//   position[0] = txtTargetX->text().toUInt();
//   position[1] = txtTargetY->text().toUInt();
//   this->TargetPatchSlice->SetPosition(position);
//   
//   PatchesMoved();
}

void TopPatchesWidget::GetPatchSize()
{
  // The edge length of the patch is the (radius*2) + 1
  this->PatchSize[0] = this->txtPatchRadius->text().toUInt() * 2 + 1;
  this->PatchSize[1] = this->txtPatchRadius->text().toUInt() * 2 + 1;
}

void TopPatchesWidget::SetupPatches()
{
  GetPatchSize();

//   InitializePatch(this->SourcePatch, this->Green);
//   
//   InitializePatch(this->TargetPatch, this->Red);
  
  Refresh();
}


void TopPatchesWidget::RefreshSlot()
{
  Refresh();
}

void TopPatchesWidget::Refresh()
{

}

void TopPatchesWidget::SetMaskedPixelsToGreen(const itk::ImageRegion<2>& targetRegion, vtkImageData* image)
{
  itk::ImageRegionIterator<Mask> maskIterator(this->MaskImage, targetRegion);

  while(!maskIterator.IsAtEnd())
    {
    if(this->MaskImage->IsHole(maskIterator.GetIndex()))
      {
      itk::Index<2> index = maskIterator.GetIndex();
      index[0] -= targetRegion.GetIndex()[0];
      index[1] -= targetRegion.GetIndex()[1];
      unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(index[0], index[1],0));
      pixel[0] = 0;
      pixel[1] = 255;
      pixel[2] = 0;
      }
    ++maskIterator;
    }  
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
  
//     Patch currentPatch = this->PatchCompare.SourcePatches[i];
//     std::cout << "Generating table row for " << currentPatch.Region << std::endl;
//   
    //QImage sourceImage = GetQImage(currentPatch.Region);
  
//     ClickableLabel* imageLabel = new ClickableLabel;
//     imageLabel->setPixmap(QPixmap::fromImage(sourceImage));
//     imageLabel->Id = i; // This is the ith best match
//     imageLabel->setScaledContents(false);
//     this->tableWidget->setCellWidget(i,0,imageLabel);
//             
//     connect( imageLabel, SIGNAL( ClickedSignal(unsigned int) ), this, SLOT(PatchClickedSlot(unsigned int)) );
//     
//     std::stringstream ssLabel;
//     ssLabel << "( " << currentPatch.Region.GetIndex()[0] << ", " << currentPatch.Region.GetIndex()[1] << ")";
//     
//     QTableWidgetItem* indexLabel = new QTableWidgetItem;
//     indexLabel->setText(ssLabel.str().c_str());
//     this->tableWidget->setItem(i,1,indexLabel);
// 
//     // Total absolute score
//     QTableWidgetItem* totalAbsoluteScoreLabel = new QTableWidgetItem;
//     totalAbsoluteScoreLabel->setData(Qt::DisplayRole, currentPatch.TotalAbsoluteScore);
//     this->tableWidget->setItem(i,2,totalAbsoluteScoreLabel);

    }
    
  this->tableWidget->resizeRowsToContents();
  this->tableWidget->resizeColumnsToContents();
  
}

void TopPatchesWidget::on_btnCompute_clicked()
{
  PositionTarget();
  
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
  this->TargetRegion = region;
}

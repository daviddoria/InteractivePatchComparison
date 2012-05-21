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

#include "PatchInfoWidget.h"

// Qt
#include <QTextEdit>

// Submodules
#include "Mask/ITKHelpers/Helpers/Helpers.h"
#include "Mask/MaskOperations.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

// Patch Comparison Submodule
#include "PatchComparison/AverageScore.hpp"
#include "PatchComparison/VarianceScore.hpp"
#include "PatchComparison/AveragePixelDifference.hpp"
#include "PatchComparison/PixelDifferences.hpp"

// ITK
#include "itkRegionOfInterestImageFilter.h"

PatchInfoWidget::PatchInfoWidget(QWidget* parent) : QWidget(parent)
{
  setupUi(this);
}

void PatchInfoWidget::SetImage(VectorImageType* const image)
{
  this->Image = image;
}

unsigned int PatchInfoWidget::GetRadius()
{
  // This assumes square patches with odd side lengths
  return Region.GetSize()[0] / 2;
}

void PatchInfoWidget::SetMask(Mask* const mask)
{
  this->MaskImage = mask;
}
  
void PatchInfoWidget::on_txtXCenter_returnPressed()
{
  itk::Index<2> currentCenter = ITKHelpers::GetRegionCenter(this->Region);

  itk::Index<2> newCenter = currentCenter;
  newCenter[0] = txtXCenter->text().toUInt();

  this->Region = ITKHelpers::GetRegionInRadiusAroundPixel(newCenter, this->GetRadius());

  emit signal_PatchMoved(Region);
}

void PatchInfoWidget::on_txtYCenter_returnPressed()
{
  itk::Index<2> currentCenter = ITKHelpers::GetRegionCenter(this->Region);

  itk::Index<2> newCenter = currentCenter;
  newCenter[1] = txtYCenter->text().toUInt();

  this->Region = ITKHelpers::GetRegionInRadiusAroundPixel(newCenter, this->GetRadius());

  emit signal_PatchMoved(Region);
}


void PatchInfoWidget::slot_Update(const itk::ImageRegion<2>& patchRegion)
{
  if(!Image->GetLargestPossibleRegion().IsInside(patchRegion))
  {
    return;
  }

  Region = patchRegion;
  itk::Index<2> patchCenter = ITKHelpers::GetRegionCenter(patchRegion);
  this->txtXCenter->setText(QString::number(patchCenter[0]));
  this->txtYCenter->setText(QString::number(patchCenter[1]));

  VectorImageType::PixelType average;
  average.SetSize(Image->GetNumberOfComponentsPerPixel());
  average.Fill(0);
  
  VectorImageType::PixelType variance;
  variance.SetSize(Image->GetNumberOfComponentsPerPixel());
  variance.Fill(0);
  
  if(MaskImage->CountValidPixels(patchRegion) > 0)
  {
    //average = Helpers::AverageInRegionMasked(Image.GetPointer(), MaskImage.GetPointer(), patchRegion);

    // This assumes both patches are fully valid (often the case when exploring)
    average = ITKHelpers::AverageInRegion(Image.GetPointer(), patchRegion);

    lblPixelMean->setText(ITKHelpers::VectorToString(average).c_str());

    variance = MaskOperations::VarianceInRegionMasked(this->Image.GetPointer(), this->MaskImage, patchRegion);

    lblPixelVariance->setText(ITKHelpers::VectorToString(variance).c_str());
  }
  else
  {
    lblPixelMean->setText("Invalid");
    lblPixelVariance->setText("Invalid");
  }

  // Patch display
  QImage sourcePatchImage = MaskOperations::GetQImageMasked(this->Image.GetPointer(), this->MaskImage,
                            patchRegion);

  sourcePatchImage = QtHelpers::FitToGraphicsView(sourcePatchImage, this->graphicsView);

  QGraphicsScene* sourceScene = new QGraphicsScene();
  sourceScene->addPixmap(QPixmap::fromImage(sourcePatchImage));
  this->graphicsView->setScene(sourceScene);

  // Average color display
  VectorImageType::Pointer averageColorImage = VectorImageType::New();
  itk::Index<2> corner = {{0,0}};
  itk::Size<2> size = {{10,10}};
  itk::ImageRegion<2> averageColorRegion(corner,size);
  
  averageColorImage->SetRegions(averageColorRegion);
  averageColorImage->SetNumberOfComponentsPerPixel(Image->GetNumberOfComponentsPerPixel());
  averageColorImage->Allocate();

  ITKHelpers::SetImageToConstant(averageColorImage.GetPointer(), average);
  
  QImage averageColorQImage = ITKQtHelpers::GetQImageColor(averageColorImage.GetPointer(), averageColorImage->GetLargestPossibleRegion());

  averageColorQImage = QtHelpers::FitToGraphicsView(averageColorQImage, this->graphicsView_AverageColor);

  QGraphicsScene* averageColorScene = new QGraphicsScene();
  averageColorScene->addPixmap(QPixmap::fromImage(averageColorQImage));
  this->graphicsView_AverageColor->setScene(averageColorScene);

}

void PatchInfoWidget::Save(const std::string& prefix)
{
  typedef itk::RegionOfInterestImageFilter< VectorImageType, VectorImageType> RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(this->Region);
  regionOfInterestImageFilter->SetInput(Image);
  regionOfInterestImageFilter->Update();

  std::stringstream ss;
  ss << prefix << "_patch.png";
  ITKHelpers::WriteImage(regionOfInterestImageFilter->GetOutput(), ss.str());
}

itk::ImageRegion<2> PatchInfoWidget::GetRegion() const
{
  return Region;
}

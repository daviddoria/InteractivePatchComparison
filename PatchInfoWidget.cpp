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

void PatchInfoWidget::SetImage(ImageType* const image)
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

  ImageType::PixelType average;
  average.SetSize(Image->GetNumberOfComponentsPerPixel());
  average.Fill(0);
  
  ImageType::PixelType variance;
  variance.SetSize(Image->GetNumberOfComponentsPerPixel());
  variance.Fill(0);
  
  if(MaskImage->CountValidPixels(patchRegion) > 0)
  {
    //average = Helpers::AverageInRegionMasked(Image.GetPointer(), MaskImage.GetPointer(), patchRegion);

    // This assumes both patches are fully valid (often the case when exploring)
    average = ITKHelpers::AverageInRegion(Image, patchRegion);

    lblPixelMean->setText(ITKHelpers::VectorToString(average).c_str());

    variance = MaskOperations::VarianceInRegionMasked(this->Image, this->MaskImage, patchRegion);

    lblPixelVariance->setText(ITKHelpers::VectorToString(variance).c_str());
  }
  else
  {
    lblPixelMean->setText("Invalid");
    lblPixelVariance->setText("Invalid");
  }

  // Patch display
  QImage sourcePatchImage = MaskOperations::GetQImageMasked(this->Image, this->MaskImage,
                            patchRegion);

  sourcePatchImage = QtHelpers::FitToGraphicsView(sourcePatchImage, this->graphicsView_Patch);

  QGraphicsScene* sourceScene = new QGraphicsScene();
  sourceScene->addPixmap(QPixmap::fromImage(sourcePatchImage));
  this->graphicsView_Patch->setScene(sourceScene);

  // Average color display
  QImage averageColorQImage = QImage(1, 1, QImage::Format_ARGB32); // A 1x1 color image
  QColor averageColor = ITKQtHelpers::GetQColor(average);
  QtHelpers::SetImageToConstant(averageColorQImage, averageColor);

  averageColorQImage = QtHelpers::FitToGraphicsView(averageColorQImage, this->graphicsView_AverageColor);

  QGraphicsScene* averageColorScene = new QGraphicsScene();
  averageColorScene->addPixmap(QPixmap::fromImage(averageColorQImage));
  this->graphicsView_AverageColor->setScene(averageColorScene);
}

void PatchInfoWidget::Save(const std::string& prefix)
{
  typedef itk::RegionOfInterestImageFilter<ImageType, ImageType> RegionOfInterestImageFilterType;
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

void PatchInfoWidget::MakeInvalid()
{
  QImage solidImage = QImage(1, 1, QImage::Format_ARGB32); // A 1x1 color image
  QColor color(Qt::black);
  QtHelpers::SetImageToConstant(solidImage, color);

  solidImage = QtHelpers::FitToGraphicsView(solidImage, this->graphicsView_AverageColor);

  QGraphicsScene* scene = new QGraphicsScene();
  scene->addPixmap(QPixmap::fromImage(solidImage));
  this->graphicsView_Patch->setScene(scene);
  this->graphicsView_AverageColor->setScene(scene);

  lblPixelMean->setText("Invalid");
  lblPixelVariance->setText("Invalid");
}

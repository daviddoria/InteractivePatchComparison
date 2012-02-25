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

// Custom
#include "Helpers.h"
#include "AverageScore.hpp"
#include "VarianceScore.hpp"

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
  
void PatchInfoWidget::on_txtX_returnPressed()
{
  itk::Index<2> index = this->Region.GetIndex();
  index[0] = txtXCenter->text().toUInt();

  this->Region = Helpers::GetRegionInRadiusAroundPixel(index, this->GetRadius());

  emit signal_PatchMoved(Region);
}

void PatchInfoWidget::on_txtY_returnPressed()
{
  itk::Index<2> index = this->Region.GetIndex();
  index[1] = txtYCenter->text().toUInt();

  this->Region = Helpers::GetRegionInRadiusAroundPixel(index, this->GetRadius());

  emit signal_PatchMoved(Region);
}


void PatchInfoWidget::slot_Update(const itk::ImageRegion<2>& patchRegion)
{
  if(!Image->GetLargestPossibleRegion().IsInside(patchRegion))
  {
    return;
  }

  Region = patchRegion;
  itk::Index<2> patchCenter = Helpers::GetRegionCenter(patchRegion);
  this->txtXCenter->setText(QString::number(patchCenter[0]));
  this->txtYCenter->setText(QString::number(patchCenter[1]));

  VectorImageType::PixelType average = Helpers::AverageInRegionMasked(Image.GetPointer(),
                                                                      MaskImage.GetPointer(), patchRegion);

  lblPixelMean->setText(Helpers::VectorToString(average).c_str());
  
  VectorImageType::PixelType variance = Helpers::VarianceInRegionMasked(this->Image.GetPointer(),
                                                                        this->MaskImage, patchRegion);

  lblPixelVariance->setText(Helpers::VectorToString(variance).c_str());


  // Display

  // Source display
  QImage sourcePatchImage = Helpers::GetQImageMasked(this->Image.GetPointer(), this->MaskImage,
                            patchRegion);

  sourcePatchImage = Helpers::FitToGraphicsView(sourcePatchImage, this->graphicsView);

  QGraphicsScene* sourceScene = new QGraphicsScene();
  sourceScene->addPixmap(QPixmap::fromImage(sourcePatchImage));
  this->graphicsView->setScene(sourceScene);

}

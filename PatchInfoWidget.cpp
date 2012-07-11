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

#include "PatchInfoWidget.h"

// Qt
#include <QFileInfo>
#include <QLineEdit>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>

// Submodules
#include "Mask/ITKHelpers/Helpers/Helpers.h"
#include "Mask/MaskOperations.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"

// Patch Comparison Submodule
#include "PatchComparison/AverageValueDifference.hpp"
#include "PatchComparison/VarianceDifference.hpp"
#include "PatchComparison/SSD.hpp"
#include "PatchComparison/PixelDifferences.hpp"

// ITK
#include "itkRegionOfInterestImageFilter.h"

PatchInfoWidget::PatchInfoWidget(QWidget* parent) : QWidget(parent)
{
  this->MaskImage = NULL;
  this->Image = NULL;

  setupUi(this);
//   this->txtXCenter->installEventFilter(this);
//   this->txtYCenter->installEventFilter(this);

//   QIntValidator* intValidator = new QIntValidator(0, 0, this);
//   this->spinXCenter->findChild<QLineEdit*>()->setValidator(intValidator);
//   this->spinYCenter->findChild<QLineEdit*>()->setValidator(intValidator);
}

void PatchInfoWidget::SetImage(ImageType* const image)
{
  this->Image = image;

  unsigned int radius = this->Region.GetSize()[0] / 2;
  
//   QIntValidator* xValidator = new QIntValidator(radius, image->GetLargestPossibleRegion().GetSize()[0] - 1 - radius);
//   QIntValidator* yValidator = new QIntValidator(radius, image->GetLargestPossibleRegion().GetSize()[1] - 1 - radius);
//   this->spinXCenter->findChild<QLineEdit*>()->setValidator(xValidator);
//   this->spinYCenter->findChild<QLineEdit*>()->setValidator(yValidator);

  this->spinXCenter->setMinimum(radius);
  this->spinXCenter->setMaximum(image->GetLargestPossibleRegion().GetSize()[0] - 1 - radius);

  this->spinYCenter->setMinimum(radius);
  this->spinYCenter->setMaximum(image->GetLargestPossibleRegion().GetSize()[1] - 1 - radius);
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

// void PatchInfoWidget::on_txtXCenter_valueChanged(int value)
// {
//   // When the focus enters one of the text boxes
//   QColor activeColor = QColor(255, 0, 0);
//   QPalette p = this->spinXCenter->findChild<QLineEdit*>()->palette();
//   p.setColor( QPalette::Normal, QPalette::Base, activeColor);
//   this->spinXCenter->findChild<QLineEdit*>()->setPalette(p);
// }
// 
// void PatchInfoWidget::on_txtYCenter_valueChanged(int value)
// {
//   QColor activeColor = QColor(255, 0, 0);
//   QPalette p = this->spinYCenter->findChild<QLineEdit*>()->palette();
//   p.setColor( QPalette::Normal, QPalette::Base, activeColor);
//   this->spinYCenter->findChild<QLineEdit*>()->setPalette(p);
// }

void PatchInfoWidget::on_spinXCenter_valueChanged(int value)
{
  itk::Index<2> currentCenter = ITKHelpers::GetRegionCenter(this->Region);

  itk::Index<2> newCenter = currentCenter;
  newCenter[0] = spinXCenter->value();

  this->Region = ITKHelpers::GetRegionInRadiusAroundPixel(newCenter, this->GetRadius());

  QColor normalColor = QColor(255, 255, 255);
  QPalette p = this->spinXCenter->findChild<QLineEdit*>()->palette();
  p.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->spinXCenter->findChild<QLineEdit*>()->setPalette(p);

  emit signal_PatchMoved(Region);
}

void PatchInfoWidget::on_spinYCenter_valueChanged(int value)
{
  itk::Index<2> currentCenter = ITKHelpers::GetRegionCenter(this->Region);

  itk::Index<2> newCenter = currentCenter;
  newCenter[1] = spinYCenter->value();

  this->Region = ITKHelpers::GetRegionInRadiusAroundPixel(newCenter, this->GetRadius());

  QColor normalColor = QColor(255, 255, 255);
  QPalette p = this->spinYCenter->findChild<QLineEdit*>()->palette();
  p.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->spinYCenter->findChild<QLineEdit*>()->setPalette(p);

  emit signal_PatchMoved(Region);
}


void PatchInfoWidget::slot_Update(const itk::ImageRegion<2>& patchRegion)
{
  if(!this->Image->GetLargestPossibleRegion().IsInside(patchRegion))
  {
    return;
  }

  this->Region = patchRegion;
  itk::Index<2> patchCenter = ITKHelpers::GetRegionCenter(patchRegion);
  this->spinXCenter->setValue(QString::number(patchCenter[0]).toUInt());
  this->spinYCenter->setValue(QString::number(patchCenter[1]).toUInt());

  ImageType::PixelType average;
  average.SetSize(Image->GetNumberOfComponentsPerPixel());
  average.Fill(0);
  
  ImageType::PixelType variance;
  variance.SetSize(Image->GetNumberOfComponentsPerPixel());
  variance.Fill(0);
  
  if(this->MaskImage && this->MaskImage->CountValidPixels(patchRegion) > 0)
  {
    //average = Helpers::AverageInRegionMasked(Image.GetPointer(), MaskImage.GetPointer(), patchRegion);

    // This assumes both patches are fully valid (often the case when exploring)
    average = ITKHelpers::AverageInRegion(Image, patchRegion);

    this->lblPixelMean->setText(ITKHelpers::VectorToString(average).c_str());

    variance = MaskOperations::VarianceInRegionMasked(this->Image, this->MaskImage, patchRegion);

    this->lblPixelVariance->setText(ITKHelpers::VectorToString(variance).c_str());
  }
  else
  {
    this->lblPixelMean->setText("Invalid");
    this->lblPixelVariance->setText("Invalid");
  }

  // Patch display
  // If we want to use a mask, then the new method is to just call ITKHelpers::GetImageMasked
  // followed by GetQImage.
//   QImage sourcePatchImage = MaskOperations::GetQImageMasked(this->Image, this->MaskImage,
//                             patchRegion);
  QImage sourcePatchImage = ITKQtHelpers::GetQImageColor(this->Image, patchRegion);

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

void PatchInfoWidget::on_btnSavePatch_clicked()
{
  bool ok;
  QString title("Filename");
  QString label("Filename:");
  QString defaultText("patch.png");
  QString fileName = QInputDialog::getText(this, title,
                                        label, QLineEdit::Normal,
                                        defaultText, &ok);
  QMessageBox msgBox;
  if (ok && !fileName.isEmpty())
  {
    Save(fileName.toStdString());
  }
}

void PatchInfoWidget::Save(const std::string& fileName)
{
  typedef itk::RegionOfInterestImageFilter<ImageType, ImageType> RegionOfInterestImageFilterType;
  RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(this->Region);
  regionOfInterestImageFilter->SetInput(this->Image);
  regionOfInterestImageFilter->Update();

  QFileInfo fileInfo(fileName.c_str());
  
  if(fileInfo.suffix().toStdString() == "png")
  {
    ITKHelpers::WriteRGBImage(regionOfInterestImageFilter->GetOutput(), fileName);
  }
  else
  {
    ITKHelpers::WriteImage(regionOfInterestImageFilter->GetOutput(), fileName);
  }
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

bool PatchInfoWidget::eventFilter(QObject *object, QEvent *event)
{
  // When the focus leaves one of the text boxes, update the patch
  // and set the background color back to normal
  QColor normalColor = QColor(255, 255, 255);
  if(object == this->spinXCenter->findChild<QLineEdit*>() && event->type() == QEvent::FocusOut)
  {
    QPalette p = this->spinXCenter->findChild<QLineEdit*>()->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    this->spinXCenter->findChild<QLineEdit*>()->setPalette(p);
    emit signal_PatchMoved(Region);
  }

  if(object == this->spinYCenter->findChild<QLineEdit*>() && event->type() == QEvent::FocusOut)
  {
    QPalette p = this->spinYCenter->findChild<QLineEdit*>()->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    this->spinYCenter->findChild<QLineEdit*>()->setPalette(p);
    emit signal_PatchMoved(Region);
  }

  return false; // Pass the event along (don't consume it)
}

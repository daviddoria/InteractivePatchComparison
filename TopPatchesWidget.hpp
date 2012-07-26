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

#ifndef TopPatchesWidget_HPP
#define TopPatchesWidget_HPP

#include "TopPatchesWidget.h"

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

#include <QtConcurrentRun>

// Submodules
#include "Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"
#include "Mask/Mask.h"

// Custom
#include "PixmapDelegate.h"

template<typename TImage>
TopPatchesWidget<TImage>::TopPatchesWidget(QWidget* parent) : TopPatchesWidgetParent(parent),
SecondaryPatchDistanceFunctor(NULL)
{
  this->setupUi(this);

  // Make the cells fit the images (based on the sizeHint from the PixmapDelegate)
  this->tblviewTopPatches->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  this->tblviewTopPatches->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  this->TargetPatchItem = new QGraphicsPixmapItem;
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(this->TargetPatchScene);

  this->ProxyModel = new QSortFilterProxyModel;
  this->TopPatchesModel = new TableModelTopPatches<TImage>(this->TopPatchData, this);
  this->ProxyModel->setSourceModel(this->TopPatchesModel);
  this->tblviewTopPatches->setModel(ProxyModel);
  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->spinNumberOfBestPatches->value());

  std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
  this->TopPatchesModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

  connect(this->tblviewTopPatches->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(slot_SelectionChanged(const QItemSelection &, const QItemSelection &)));

  // Setup progress bar
  this->ProgressDialog = new QProgressDialog();
  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  this->ProgressDialog->hide();

  connect(&this->FutureWatcher, SIGNAL(finished()), this, SLOT(slot_Finished()));
  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));
}

template<typename TImage>
void TopPatchesWidget<TImage>::slot_Finished()
{
  std::cout << "Finshed" << std::endl;
}

template<typename TImage>
void TopPatchesWidget<TImage>::SetTargetRegion(const itk::ImageRegion<2>& targetRegion)
{
  this->TargetRegion = targetRegion;
  
  QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, targetRegion);

  QPixmap pixmap = QPixmap::fromImage(patchImage);
  //std::cout << "Set target patch display height to: "
  //          << this->gfxTargetPatch->size().height() << std::endl;
  pixmap = pixmap.scaledToHeight(this->gfxTargetPatch->size().height());

  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(TargetPatchScene);

  this->TargetPatchScene->addPixmap(pixmap);
}

template<typename TImage>
void TopPatchesWidget<TImage>::slot_SelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
  QModelIndexList indexes = this->tblviewTopPatches->selectionModel()->selection().indexes();

  std::vector<itk::ImageRegion<2> > topSourceRegions;

  for (int i = 0; i < indexes.count(); ++i)
  {
    //std::cout << "selectedIndexes: " << indexes.at(i).row() << std::endl;
    //topSourceRegions.push_back(this->TopPatchesModel->GetTopPatchData()[indexes.at(i).row()].first);
    unsigned int originalRowId = this->ProxyModel->mapToSource(indexes.at(i)).row(); // This was the row id before sorting
    topSourceRegions.push_back(this->TopPatchesModel->GetTopPatchData()[originalRowId].first);
  }

  emit signal_TopPatchesSelected(topSourceRegions);
}

// void TopPatchesWidget::slot_SingleClicked(const QModelIndex& selected)
// {
//   emit signal_TopPatchSelected(this->TopPatchesModel->GetTopPatchData()[selected.row()].first);
// }

template<typename TImage>
void TopPatchesWidget<TImage>::SetImage(TImage* const image)
{
  this->Image = image;
  this->TopPatchesModel->SetImage(this->Image);
}

template<typename TImage>
void TopPatchesWidget<TImage>::on_btnComputeSecondary_clicked()
{
  // Replace the data using the secondary distance functor
  for(int i = 0; i < this->spinNumberOfBestPatches->value(); ++i)
  {
    float secondaryDistance = this->SecondaryPatchDistanceFunctor->Distance(this->TargetRegion, this->TopPatchData[i].first);
    this->TopPatchData[i].second = secondaryDistance;
  }

  // Update the data in the model
  this->TopPatchesModel->SetTopPatchData(this->TopPatchData);
  this->TopPatchesModel->Refresh();
}

template<typename TImage>
void TopPatchesWidget<TImage>::on_btnFindTopPatches_clicked()
{
  std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
  this->TopPatchesModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());
  this->tblviewTopPatches->resizeRowsToContents();

  // Lock in the settings. That is, once "Compute" is clicked,
  // the values in the spin boxes correspond to what is being displayed.
  QColor normalColor = QColor(255, 255, 255);
  QPalette bestPatchesPalette = this->spinNumberOfBestPatches->findChild<QLineEdit*>()->palette();
  bestPatchesPalette.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->spinNumberOfBestPatches->findChild<QLineEdit*>()->setPalette(bestPatchesPalette);

  // Start the computation.
  QFuture<void> future = QtConcurrent::run(this, &TopPatchesWidget::Compute);
  this->FutureWatcher.setFuture(future);

  this->ProgressDialog->exec();
}

template<typename TImage>
void TopPatchesWidget<TImage>::Compute()
{
  this->SelfPatchCompareFunctor.SetImage(this->Image);
  //this->PatchCompare.SetMask(this->MaskImage);
  this->SelfPatchCompareFunctor.CreateFullyValidMask();

  this->SelfPatchCompareFunctor.SetTargetRegion(this->TargetRegion);

  this->SelfPatchCompareFunctor.ComputePatchScores();

  this->TopPatchData = this->SelfPatchCompareFunctor.GetPatchData();

  //std::sort(topPatchData.begin(), topPatchData.end(), Helpers::SortBySecondAccending<PatchDataType>);
  unsigned int numberOfPatches = this->spinNumberOfBestPatches->value();
//   std::partial_sort(this->TopPatchData.begin(), this->TopPatchData.begin() + numberOfPatches,
//                     this->TopPatchData.end(),
//                     Helpers::SortBySecondAccending<SelfPatchCompare<ImageType>::PatchDataType>);

  // Perform a full sort
  std::sort(this->TopPatchData.begin(), this->TopPatchData.end(),
                    Helpers::SortBySecondAccending<typename SelfPatchCompare<TImage>::PatchDataType>);

  this->TopPatchData.resize(numberOfPatches);
  std::cout << "There are " << this->TopPatchData.size() << " top patches." << std::endl;

  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->spinNumberOfBestPatches->value());
  this->TopPatchesModel->SetTopPatchData(this->TopPatchData);
  this->TopPatchesModel->Refresh();
}

template<typename TImage>
void TopPatchesWidget<TImage>::SetPatchDistanceFunctor(PatchDistance<TImage>* const patchDistanceFunctor)
{
  this->SelfPatchCompareFunctor.SetPatchDistanceFunctor(patchDistanceFunctor);
}

template<typename TImage>
bool TopPatchesWidget<TImage>::eventFilter(QObject *object, QEvent *event)
{
  // When the focus leaves one of the text boxes, update the patches
  QColor normalColor = QColor(255, 255, 255);

  if(dynamic_cast<QLineEdit*>(object) == spinNumberOfBestPatches->findChild<QLineEdit*>() &&
    event->type() == QEvent::FocusOut)
  {
    if(!this->spinNumberOfBestPatches->findChild<QLineEdit*>()->hasAcceptableInput())
    {
      std::cerr << "Invalid number of patches!" << std::endl;
      return false; // Pass the event along (don't consume it)
    }
    QPalette p = spinNumberOfBestPatches->findChild<QLineEdit*>()->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    spinNumberOfBestPatches->findChild<QLineEdit*>()->setPalette(p);
  }

  return false; // Pass the event along (don't consume it)
}

template<typename TImage>
void TopPatchesWidget<TImage>::on_spinNumberOfBestPatches_valueChanged(int value)
{
  QColor activeColor = QColor(255, 0, 0);
  QPalette p = this->spinNumberOfBestPatches->findChild<QLineEdit*>()->palette();
  p.setColor( QPalette::Normal, QPalette::Base, activeColor);
  this->spinNumberOfBestPatches->findChild<QLineEdit*>()->setPalette(p);
}

template<typename TImage>
void TopPatchesWidget<TImage>::SetSelfPatchCompareFunctor(
     const SelfPatchCompare<TImage>& selfPatchCompareFunctor)
{
  this->SelfPatchCompareFunctor = selfPatchCompareFunctor;
}

template<typename TImage>
void TopPatchesWidget<TImage>::SetSecondaryPatchDistanceFunctor
                                 (PatchDistance<TImage>* const patchDistanceFunctor)
{
  this->SecondaryPatchDistanceFunctor = patchDistanceFunctor;
}

#endif

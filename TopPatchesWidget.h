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

#ifndef TopPatchesWidget_H
#define TopPatchesWidget_H

#include "ui_TopPatchesWidget.h"

// Qt
#include <QDialog>
#include <QObject>
#include <QFutureWatcher>
class QProgressDialog;
class QSortFilterProxyModel;

// ITK
#include "itkVectorImage.h"

// Submodules
#include "PatchComparison/Mask/Mask.h"
#include "PatchComparison/PatchDistance.h"
#include "PatchComparison/SelfPatchCompare.h"
#include "PatchComparison/SelfPatchCompareLocalOptimization.h"

// Custom
#include "TableModelTopPatches.h" // Can't forward declare a class template

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class TopPatchesWidgetParent : public QWidget, public Ui::TopPatchesWidget
{
Q_OBJECT

public:

  /** Constructor. */
  TopPatchesWidgetParent(QWidget* parent = NULL) : QWidget(parent){}

  /** Set the target/query region. */
  virtual void SetTargetRegion(const itk::ImageRegion<2>& targetRegion) = 0;

public slots:

  /** When a patch (or patches) is clicked or the arrow keys are used, emit a signal. */
  virtual void slot_SelectionChanged(const QItemSelection &, const QItemSelection &) = 0;

  /** Called when the "Find Top Matches" button is clicked. */
  virtual void on_btnFindTopPatches_clicked() = 0;

  /** Called when the "compute secondary" button is clicked. */
  virtual void on_btnComputeSecondary_clicked() = 0;

  /** Called when the progress bar is complete. */
  virtual void slot_Finished() = 0;

  /** Called when the number of patches to display is changed. */
  virtual void on_spinNumberOfBestPatches_valueChanged(int) = 0;

signals:

  /** Emit a signal when a patch is selected. Note: this is not repeated in the templated
    * subclass, this signal can be used directly from there. */
  void signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >& region);

};

template <typename TImage>
class TopPatchesWidget : public TopPatchesWidgetParent
{

public:

  /** Constructor. */
  TopPatchesWidget(QWidget* parent = NULL);

  /** Set the target/query region. */
  void SetTargetRegion(const itk::ImageRegion<2>& targetRegion);

  /** Set the image to use. */
  void SetImage(TImage* const image);

  /** Set the DistanceFunctor to use in the SelfPatchCompareFunctor. */
  void SetPatchDistanceFunctor(PatchDistance<TImage>* const patchDistanceFunctor);

  /** Set the DistanceFunctor to use for secondary comparison. */
  void SetSecondaryPatchDistanceFunctor(PatchDistance<TImage>* const patchDistanceFunctor);

  /** Set the SelfPatchCompareFunctor to use. */
  void SetSelfPatchCompareFunctor(const SelfPatchCompare<TImage>& selfPatchCompareFunctor);

// public slots:

  /** When a patch (or patches) is clicked or the arrow keys are used, emit a signal. */
  void slot_SelectionChanged(const QItemSelection &, const QItemSelection &);

  /** Called when the "Find Top Patches" button is clicked. */
  void on_btnFindTopPatches_clicked();

  /** Called when the "compute secondary" button is clicked. */
  void on_btnComputeSecondary_clicked();

  /** Called when the progress bar is complete. */
  void slot_Finished();

  /** Called when the number of patches to display is changed. */
  void on_spinNumberOfBestPatches_valueChanged(int);

private:
  /** The image that the patches reference. */
  TImage* Image;

  /** Handle events (not signals) of other widgets. */
  bool eventFilter(QObject *object, QEvent *event);

  /** The main computation. */
  void Compute();

  /** The scene for the target patch. */
  QGraphicsScene* TargetPatchScene;

  /** The item for the target patch. */
  QGraphicsPixmapItem* TargetPatchItem;

  /** The table model used to display the patches. */
  TableModelTopPatches<TImage>* TopPatchesModel;

  /** The proxy model to allow the table model to be sorted. */
  QSortFilterProxyModel* ProxyModel;

  /** The query/target region that we are trying to match a patch to. */
  itk::ImageRegion<2> TargetRegion;

  /** Store the top patch data. */
  std::vector<typename SelfPatchCompare<TImage>::PatchDataType> TopPatchData;

  /** A watcher to check in on the progress of a long computation. */
  QFutureWatcher<void> FutureWatcher;

  /** The progress bar. */
  QProgressDialog* ProgressDialog;

  /** The functor to use to find the best patch. */
  SelfPatchCompare<TImage> SelfPatchCompareFunctor;
  //SelfPatchCompareLocalOptimization<TImage> SelfPatchCompareFunctor;

  PatchDistance<TImage>* SecondaryPatchDistanceFunctor;
};

#include "TopPatchesWidget.hpp"

#endif // TopPatchesWidget_H

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
#include "TableModelTopPatches.h"

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class TopPatchesWidget : public QWidget, public Ui::TopPatchesWidget
{
Q_OBJECT

public:
  typedef Eigen::MatrixXf MatrixType;
  typedef Eigen::VectorXf VectorType;

  typedef itk::VectorImage<float, 2> ImageType;

  TopPatchesWidget(QWidget* parent = NULL);

  void SetTargetRegion(const itk::ImageRegion<2>& targetRegion);

  void SetImage(ImageType* const image);

  void SetPatchDistanceFunctor(PatchDistance* const patchDistanceFunctor);

public slots:

  /** When a patch (or patches) is clicked or the arrow keys are used, emit a signal. */
  void slot_SelectionChanged(const QItemSelection &, const QItemSelection &);

  /** Called when the compute button is clicked. */
  void on_btnCompute_clicked();

  /** Called when the progress bar is complete. */
  void slot_Finished();

//   void on_txtClusters_returnPressed();
//   void on_txtClusters_textEdited();
  /** Called when the number of clusters is changed. */
  void on_spinClusters_valueChanged(int);

//   void on_txtNumberOfPatches_returnPressed();
//   void on_txtNumberOfPatches_textEdited();
  /** Called when the number of patches to display is changed. */
  void on_spinNumberOfBestPatches_valueChanged(int);

signals:

  /** Emit a signal when a patch is selected. */
  void signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >& region);

private:
  /** The image that the patches reference. */
  ImageType* Image;

  /** Handle events (not signals) of other widgets. */
  bool eventFilter(QObject *object, QEvent *event);

  /** The main computation. */
  void Compute();

  /** The scene for the target patch. */
  QGraphicsScene* TargetPatchScene;

  /** The item for the target patch. */
  QGraphicsPixmapItem* TargetPatchItem;

  /** The table model used to display the patches. */
  TableModelTopPatches* TopPatchesModel;

  /** The proxy model to allow the table model to be sorted. */
  QSortFilterProxyModel* ProxyModel;

  /** The query/target region that we are trying to match a patch to. */
  itk::ImageRegion<2> TargetRegion;

  /** Perform a clustering of the top patches. */
  void Cluster();

  /** Store the top patch data. */
  std::vector<SelfPatchCompare<ImageType>::PatchDataType> TopPatchData;

  /** A watcher to check in on the progress of a long computation. */
  QFutureWatcher<void> FutureWatcher;

  /** The progress bar. */
  QProgressDialog* ProgressDialog;

  //SelfPatchCompare SelfPatchCompareFunctor;
  /** The functor to use to find the best patch. */
  SelfPatchCompareLocalOptimization<ImageType> SelfPatchCompareFunctor;
};

#endif // TopPatchesWidget_H

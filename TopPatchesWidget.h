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
#include "Mask/Mask.h"
#include "PatchComparison/PatchDistance.h"

// Custom
#include "TableModelTopPatches.h"
#include "SelfPatchCompare.h"

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

  void on_btnCompute_clicked();

  /** Called when the progress bar is complete. */
  void slot_Finished();

signals:

  void signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >& region);

private:
  ImageType* Image;

  void Compute();
  
  QGraphicsScene* TargetPatchScene;
  QGraphicsPixmapItem* TargetPatchItem;
  TableModelTopPatches* TopPatchesModel;
  QSortFilterProxyModel* ProxyModel;
  
  itk::ImageRegion<2> TargetRegion;

  void Cluster();

  std::vector<SelfPatchCompare::PatchDataType> TopPatchData;

  /** To monitor the progress. */
  QFutureWatcher<void> FutureWatcher;
  QProgressDialog* ProgressDialog;

  SelfPatchCompare SelfPatchCompareFunctor;
};

#endif // TopPatchesWidget_H

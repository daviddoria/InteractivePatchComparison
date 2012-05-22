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

#ifndef TopPatchesWidget_H
#define TopPatchesWidget_H

#include "ui_TopPatchesWidget.h"

// Qt
#include <QDialog>
#include <QObject>

// ITK
#include "itkVectorImage.h"

// Submodules
#include "Mask/Mask.h"

// Custom
#include "TableModelTopPatches.h"

/** This class is necessary because a class template cannot have the Q_OBJECT macro directly. */
class TopPatchesWidget : public QWidget, public Ui::TopPatchesWidget
{
Q_OBJECT

public:
  typedef itk::VectorImage<float, 2> ImageType;
  
  TopPatchesWidget(QWidget* parent = NULL);

  void SetTargetRegion(const itk::ImageRegion<2>& targetRegion);

  void SetImage(ImageType* const image);
  
public slots:

  /** When a patch is clicked, emit a signal. */
  void slot_SingleClicked(const QModelIndex & index);

  void on_btnCompute_clicked();

signals:
  void signal_TopPatchSelected(const itk::ImageRegion<2>& region);

private:
  ImageType* Image;

  QGraphicsScene* TargetPatchScene;
  QGraphicsPixmapItem* TargetPatchItem;
  TableModelTopPatches* TopPatchesModel;

  itk::ImageRegion<2> TargetRegion;
};

#endif // TopPatchesWidget_H

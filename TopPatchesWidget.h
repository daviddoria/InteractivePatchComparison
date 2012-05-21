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

// VTK
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkRenderer;

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>
#include <QImage>

// Custom
#include "Types.h"
#include "SelfPatchCompare.h"

// Submodules
#include "Mask/Mask.h"

class SwitchBetweenStyle;

class TopPatchesWidget : public QWidget, public Ui::TopPatchesWidget
{
  Q_OBJECT
public:

  // Constructor/Destructor
  TopPatchesWidget(const std::string& imageFileName, const std::string& maskFileName);
  TopPatchesWidget();
  void SharedConstructor();
  ~TopPatchesWidget() {};

  void Refresh();

  const static unsigned int DisplayPatchSize = 50;

  void DisplaySourcePatches();

  void PositionTarget();

public slots:

  void PatchClickedSlot(const unsigned int);

  void on_txtPatchRadius_returnPressed();
  void on_txtNumberOfPatches_returnPressed();
  
  void on_txtTargetX_returnPressed();
  void on_txtTargetY_returnPressed();
  
  void on_btnCompute_clicked();

  void on_chkFillPatch_clicked();
  
  void RefreshSlot();

protected:

  void SetMaskedPixelsToGreen(const itk::ImageRegion<2>& targetRegion, vtkImageData* image);

  static const unsigned char Green[3];
  static const unsigned char Red[3];

  void GetPatchSize();

  void PatchesMoved();
  void SetupPatches();

  // The data that the user loads
  itk::VectorImage<float, 2>::Pointer Image;
  Mask::Pointer MaskImage;
  
  itk::Size<2> PatchSize;
  unsigned int PatchScale;
  
  QGraphicsScene* SourcePatchesScene;
  QGraphicsScene* TargetPatchScene;
  
  SelfPatchCompare PatchCompare;

  unsigned int DisplayedSourcePatch;
};

#endif // TopPatchesWidget_H

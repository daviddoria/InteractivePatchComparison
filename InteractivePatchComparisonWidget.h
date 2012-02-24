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

#ifndef InteractivePatchComparisonWidget_H
#define InteractivePatchComparisonWidget_H

#include "ui_InteractivePatchComparisonWidget.h"

// VTK
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>

// Custom
#include "Mask.h"
#include "Types.h"

class SwitchBetweenStyle;

class InteractivePatchComparisonWidget : public QMainWindow, public Ui::InteractivePatchComparisonWidget
{
Q_OBJECT
public:

  // Constructor/Destructor
  InteractivePatchComparisonWidget();
  InteractivePatchComparisonWidget(const std::string& imageFileName, const std::string& maskFileName);
  void SharedConstructor();
  ~InteractivePatchComparisonWidget() {};
  
  // These function deal with flipping the image
  void SetCameraPosition(const double leftToRight[3], const double bottomToTop[3]);
  void SetCameraPosition1();
  void SetCameraPosition2();
  
  void Refresh();
  
public slots:
  
  void on_actionOpenImage_activated();
  void on_actionOpenMask_activated();
  void on_actionOpenMaskInverted_activated();
  
  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipImage_activated();
  
  void on_txtPatchRadius_returnPressed();
  
  void on_txtSourceX_returnPressed();
  void on_txtSourceY_returnPressed();
  
  void on_txtTargetX_returnPressed();
  void on_txtTargetY_returnPressed();
  
  void on_chkShowMask_clicked();
  
  
  void RefreshSlot();
  
protected:

  void OpenImage(const std::string& filename);
  void OpenMask(const std::string& filename);
  
  void SetMaskedPixelsToGreen(const itk::ImageRegion<2>& targetRegion, vtkImageData* image);
  
  static const unsigned char Green[3];
  static const unsigned char Red[3];
  
  void GetPatchSize();
  
  void InitializePatch(vtkImageData* image, const unsigned char color[3]);
  
  void PatchesMoved();
  void SetupPatches();
  
  // Allow us to interact with the objects as we would like.
  vtkSmartPointer<SwitchBetweenStyle> InteractorStyle;
  
  // Track if the image has been flipped
  bool Flipped;

  vtkSmartPointer<vtkRenderer> Renderer;
  
  // Image display
  vtkSmartPointer<vtkImageData> VTKImage;
  vtkSmartPointer<vtkImageSlice> ImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> ImageSliceMapper;
  
  // Mask image display
  vtkSmartPointer<vtkImageData> VTKMaskImage;
  vtkSmartPointer<vtkImageSlice> MaskImageSlice;
  vtkSmartPointer<vtkImageSliceMapper> MaskImageSliceMapper;
  
  // Movable patches
  vtkSmartPointer<vtkImageData> SourcePatch;
  vtkSmartPointer<vtkImageSlice> SourcePatchSlice;
  vtkSmartPointer<vtkImageSliceMapper> SourcePatchSliceMapper;
  
  vtkSmartPointer<vtkImageData> TargetPatch;
  vtkSmartPointer<vtkImageSlice> TargetPatchSlice;
  vtkSmartPointer<vtkImageSliceMapper> TargetPatchSliceMapper;
  
  // The data that the user loads
  VectorImageType::Pointer Image;
  Mask::Pointer MaskImage;
  
  itk::Size<2> PatchSize;
  unsigned int PatchScale;
  
  QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);
};

#endif // InteractivePatchComparisonWidget_H

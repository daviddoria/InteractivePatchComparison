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

#ifndef FORM_H
#define FORM_H

#include "ui_Form.h"

// VTK
#include <vtkSmartPointer.h>
//#include <vtkSeedWidget.h>
//#include <vtkPointHandleRepresentation2D.h>

class vtkActor;
class vtkBorderWidget;
class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;
class vtkPolyData;
class vtkPolyDataMapper;

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>

// Custom
#include "Mask.h"
#include "Types.h"

class InteractorStyleImageNoLevel;

class Form : public QMainWindow, public Ui::Form
{
  Q_OBJECT
public:

  // Constructor/Destructor
  Form();
  ~Form() {};
  
  // These function deal with flipping the image
  void SetCameraPosition(double leftToRight[3], double bottomToTop[3]);
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
  
  void RefreshSlot();
  
protected:
  
  // The interactor to allow us to zoom and pan the image
  vtkSmartPointer<InteractorStyleImageNoLevel> InteractorStyle;
  
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
  
  // The data that the user loads
  FloatVectorImageType::Pointer Image;
  Mask::Pointer MaskImage;
  
};

#endif // Form_H

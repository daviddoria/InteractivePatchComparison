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

// Eigen
#include <Eigen/Dense>

// VTK
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkImageSlice;
class vtkImageSliceMapper;

// ITK
#include "itkImage.h"

// Qt
#include <QMainWindow>

// Submodules
#include "Mask/Mask.h"
#include "ITKVTKCamera/ITKVTKCamera.h"

// Custom
#include "Types.h"
#include "TopPatchesWidget.h"

class SwitchBetweenStyle;

class InteractivePatchComparisonWidget : public QMainWindow, private Ui::InteractivePatchComparisonWidget
{
Q_OBJECT
public:

  typedef itk::VectorImage<float, 2> ImageType;

  // Constructor/Destructor
  InteractivePatchComparisonWidget(QWidget* parent = 0);
  InteractivePatchComparisonWidget(const std::string& imageFileName, const std::string& maskFileName, QWidget* parent = 0);
  void SharedConstructor();

  void Refresh();

signals:
  void signal_TargetPatchMoved(const itk::ImageRegion<2>&);
  void signal_SourcePatchMoved(const itk::ImageRegion<2>&);

public slots:

  // View menu
  void on_action_View_TopPatches_activated();

  void on_actionOpenImage_activated();
  void on_actionOpenMask_activated();
  void on_actionOpenMaskInverted_activated();

  void on_btnSavePatches_clicked();
  
  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipImage_activated();
  void on_actionScreenshot_activated();
  
  void on_txtPatchRadius_returnPressed();

  void RefreshSlot();

  void slot_TargetPatchMoved(const itk::ImageRegion<2>&);
  void slot_SourcePatchMoved(const itk::ImageRegion<2>&);

private:

  unsigned int GetPatchRadius();

  Eigen::VectorXf ComputeFeatures(const itk::ImageRegion<2>& region);
  Eigen::VectorXf ComputeNormalizedFeatures(const itk::ImageRegion<2>& region);
  void ComputeFeatureMatrixStatistics();

  Eigen::VectorXf FeatureMeans;
  Eigen::VectorXf FeatureStandardDeviations;

  void PatchesMovedEventHandler();
  
  void showEvent(QShowEvent* event);
  
  void OpenImage(const std::string& filename);
  void OpenMask(const std::string& filename);

  static const unsigned char Green[3];
  static const unsigned char Red[3];
  static const unsigned char Blue[3];
  
  void GetPatchSize();
  
  void InitializePatch(vtkImageData* image, const unsigned char color[3]);
  
  void SetupPatches();
  
  // Allow us to interact with the objects as we would like.
  vtkSmartPointer<SwitchBetweenStyle> InteractorStyle;

  // Display the image appropriately
  ITKVTKCamera itkvtkCamera;

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
  ImageType::Pointer Image;
  Mask::Pointer MaskImage;

  itk::Size<2> PatchSize;

};

#endif // InteractivePatchComparisonWidget_H

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
#include "Layer.h"

class SwitchBetweenStyle;

class InteractivePatchComparisonWidget : public QMainWindow, private Ui::InteractivePatchComparisonWidget
{
Q_OBJECT
public:

  typedef itk::VectorImage<float, 2> ImageType;

  // Constructor/Destructor
  InteractivePatchComparisonWidget(QWidget* parent = 0);
  InteractivePatchComparisonWidget(const std::string& imageFileName,
                                   QWidget* parent = 0);
  InteractivePatchComparisonWidget(const std::string& imageFileName, const std::string& maskFileName,
                                   QWidget* parent = 0);
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

  void on_action_SavePatches_activated();

  void on_actionHelp_activated();
  void on_actionQuit_activated();

  void on_actionFlipImage_activated();
  void on_actionScreenshot_activated();

  void on_spinPatchRadius_valueChanged(int value);

  void RefreshSlot();

  void slot_TargetPatchMoved(const itk::ImageRegion<2>&);
  void slot_SourcePatchMoved(const itk::ImageRegion<2>&);

  void slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& );

  //void on_txtPatchRadius_textEdited();

private:

  /** Compute the difference between selected patches. */
  void UpdatePatches();

  /** When either the target patch or source patch is moved, this function should be called. */
  void PatchesMovedEventHandler(vtkObject* caller, long unsigned int eventId, void* callData);

  /** When the widget finishes loading, this function is called. */
  void showEvent(QShowEvent* event);

  /** Open an image. */
  void OpenImage(const std::string& filename);

  /** Open a mask. */
  void OpenMask(const std::string& filename);

  /** Define some colors. */
//   static const unsigned char Green[3];
//   static const unsigned char Red[3];
//   static const unsigned char Blue[3];

  /** Get the size of the patches from the line edit. */
  void GetPatchSizeFromGUI();

  /** Set the size and color of a patch. */
  void InitializePatch(vtkImageData* const image, const unsigned char color[3]);

  /** Set the sizes of the layers used to display and interact with the patches. */
  void SetupPatches();

  /** Allow us to interact with the objects as we would like. */
  vtkSmartPointer<SwitchBetweenStyle> InteractorStyle;

  /** Display the image appropriately */
  ITKVTKCamera itkvtkCamera;

  /** The main renderer */
  vtkSmartPointer<vtkRenderer> Renderer;

  /** Image display */
  Layer ImageLayer;

  /** Mask image display */
  Layer MaskImageLayer;

  /** Movable patches */
  Layer SourcePatchLayer;
  Layer TargetPatchLayer;

  /** A layer to display all of the selected source patches. */
  Layer SelectedSourcePatchesLayer;

  /** The data that the user loads */
  ImageType::Pointer Image;
  Mask::Pointer MaskImage;

  /** The size of the patches to compare. */
  itk::Size<2> PatchSize;

  /** The projection matrix to project patches to a lower dimensional space. */
  typedef Eigen::MatrixXf MatrixType;
  typedef Eigen::VectorXf VectorType;

  /** The projection matrix used project patches to a lower dimensional space. */
  MatrixType ProjectionMatrix;

  /** Compute the projection matrix to project patches to a lower dimensional space. */
  void ComputeProjectionMatrix();

  /** Handle events (not signals) so we don't have to subclass things like QLineEdit. */
  bool eventFilter(QObject *object, QEvent *event);

  std::string ImageFileName;
  std::string MaskFileName;
};

#endif // InteractivePatchComparisonWidget_H

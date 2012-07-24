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
#include "PatchComparison/Mask/Mask.h"
#include "ITKVTKCamera/ITKVTKCamera.h"
#include "PatchComparison/PatchDistance.h"

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

  // File menu
  void on_actionOpenImage_activated();
  void on_actionOpenMask_activated();
  void on_actionOpenMaskInverted_activated();
  void on_actionQuit_activated();
  void on_action_SavePatches_activated();
  void on_actionScreenshot_activated();

  // Help menu
  /** The slot to handle displaying the help text. */
  void on_actionHelp_activated();

  // View menu
  /** The slot to handle when the user wants to flip the image vertically. */
  void on_actionFlipVertically_activated();

  /** The slot to handle when patch radius is changed. */
  void on_spinPatchRadius_valueChanged(int value);

  /** Refresh. */
  void RefreshSlot();

  /** The slot to handle when the target patch is moved. */
  void slot_TargetPatchMoved(const itk::ImageRegion<2>&);

  /** The slot to handle when the source patch is moved. */
  void slot_SourcePatchMoved(const itk::ImageRegion<2>&);

  /** The slot to handle when the selected top patch is changed. */
  void slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& );

  /** The slot to handle when the user has selected the SSD distance functor. */
  void on_radDistanceSSD_clicked();

  /** The slot to handle when the user has selected the PCA distance functor. */
  void on_radDistancePCA_clicked();

  /** The slot to handle when the user has selected the LocalPCA distance functor. */
  void on_radDistanceLocalPCA_clicked();

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

  /** The movable source patch. */
  Layer SourcePatchLayer;

  /** The movable target patch. */
  Layer TargetPatchLayer;

  /** A layer to display all of the selected source patches. */
  Layer SelectedSourcePatchesLayer;

  /** The image that the user loads. */
  ImageType::Pointer Image;

  /** A blurred version of the image that the user loads. */
  ImageType::Pointer BlurredImage;

  /** The mask that the user loads. */
  Mask::Pointer MaskImage;

  /** The size of the patches to compare. */
  itk::Size<2> PatchSize;

  /** Setup the distance functors. */
  void SetupDistanceFunctors();

  /** Handle events (not signals) so we don't have to subclass things like QLineEdit. */
  bool eventFilter(QObject *object, QEvent *event);

  /** The filename of the image to operate on. */
  std::string ImageFileName;

  /** The filename of the mask. */
  std::string MaskFileName;

  /** A list of all functor to potentially use. */
  std::vector<PatchDistance*> PatchDistanceFunctors;

  /** The functor to use in the PatchInformation widgets */
  PatchDistance* CurrentDistanceFunctor;

  /** A list of all functor to potentially use. */
  std::vector<TopPatchesWidget*> TopPatchesWidgets;

};

#endif // InteractivePatchComparisonWidget_H

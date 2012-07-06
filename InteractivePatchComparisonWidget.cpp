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

#include "InteractivePatchComparisonWidget.h"

// Eigen
#include <Eigen/Dense>

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkVector.h"

// Qt
#include <QFileDialog>
#include <QIcon>
#include <QTextEdit>

// VTK
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkMath.h>
#include <vtkPNGWriter.h>
#include <vtkPointData.h>
#include <vtkProperty2D.h>
#include <vtkPolyDataMapper.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Submodules
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "Mask/ITKHelpers/Helpers/Helpers.h"
#include "Mask/Mask.h"
#include "Mask/MaskOperations.h"
#include "VTKHelpers/VTKHelpers.h"
#include "PatchProjection/EigenHelpers/EigenHelpers.h"
#include "PatchProjection/PatchProjection.h"

// Custom
#include "SwitchBetweenStyle.h"
#include "Types.h"

// Patch Comparison Submodule
#include "PatchComparison/AverageValueDifference.h"
#include "PatchComparison/DiffusionDistance.h"
#include "PatchComparison/CorrelationScore.h"
#include "PatchComparison/PixelDifferences.h"
#include "PatchComparison/SSD.h"

const unsigned char InteractivePatchComparisonWidget::Green[3] = {0,255,0};
const unsigned char InteractivePatchComparisonWidget::Red[3] = {255,0,0};
const unsigned char InteractivePatchComparisonWidget::Blue[3] = {0,0,255};

void InteractivePatchComparisonWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Interactive Patch Comparison</h1>\
  Position the two patches. <br/>\
  Their difference will be displayed.<br/>\
  Additionally you can display the top patches of the target patch.<p/>");
  help->show();
}

// Constructors
InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(const std::string& imageFileName,
                                                                   const std::string& maskFileName, QWidget* parent)
: QMainWindow(parent)
{
  SharedConstructor();
  OpenImage(imageFileName);
  OpenMask(maskFileName);
}

void InteractivePatchComparisonWidget::SharedConstructor()
{
  this->setupUi(this);

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");

  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);

  this->InteractorStyle = vtkSmartPointer<SwitchBetweenStyle>::New();

  // Setup the image display objects
  this->ImageLayer.ImageSlice->PickableOff();
  this->ImageLayer.ImageSlice->VisibilityOff(); // There are errors if this is visible and therefore displayed before it has data ("This data object does not contain the requested extent.")

  this->SelectedSourcePatchesLayer.ImageSlice->PickableOff();
  this->SelectedSourcePatchesLayer.ImageSlice->VisibilityOff();
  
  // Initialize and link the mask image display objects
  this->MaskImageLayer.ImageSlice->PickableOff();
  this->MaskImageLayer.ImageSlice->VisibilityOff();

  // Initialize patches
  this->SourcePatchLayer.ImageSlice->VisibilityOff();
  this->TargetPatchLayer.ImageSlice->VisibilityOff();

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(this->ImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->MaskImageLayer.ImageSlice);
  this->Renderer->AddViewProp(this->SourcePatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->TargetPatchLayer.ImageSlice);
  this->Renderer->AddViewProp(this->SelectedSourcePatchesLayer.ImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->Init();

  this->Image = ImageType::New();
  //this->MaskImage = Mask::New();
  
  this->MaskImage = NULL;

  SetupPatches();
  
  /** When the patches are dragged with the mouse, alert the GUI. */
  this->InteractorStyle->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this,
                                                     &InteractivePatchComparisonWidget::PatchesMovedEventHandler);

  /** Connect the PatchInfoWidgets signals (e.g. when the user changes the patch locations
   * using the text boxes). */
  connect(TargetPatchInfoWidget, SIGNAL(signal_PatchMoved(const itk::ImageRegion<2>&)),
          this, SLOT(slot_TargetPatchMoved(const itk::ImageRegion<2>&)));
  connect(SourcePatchInfoWidget, SIGNAL(signal_PatchMoved(const itk::ImageRegion<2>&)),
          this, SLOT(slot_SourcePatchMoved(const itk::ImageRegion<2>&)));

  /** Alert the PatchInfoWidgets when the patches have moved. These are typically called when the user specifies the position of the patches using the text boxes.*/
  connect(this, SIGNAL(signal_TargetPatchMoved(const itk::ImageRegion<2>&)),
          TargetPatchInfoWidget, SLOT(slot_Update(const itk::ImageRegion<2>& )));

  connect(this, SIGNAL(signal_SourcePatchMoved(const itk::ImageRegion<2>&)),
          SourcePatchInfoWidget, SLOT(slot_Update(const itk::ImageRegion<2>& )));

  /** This is used when the user clicks on a top patch in the view of the top patches. */
//   connect(this->TopPatchesPanel, SIGNAL(signal_TopPatchSelected(const itk::ImageRegion<2>&)),
//           this, SLOT(slot_SourcePatchMoved(const itk::ImageRegion<2>& )));
  connect(this->TopPatchesPanel, SIGNAL(signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >&)),
          this, SLOT(slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& )));

}
  
InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(QWidget* parent) : QMainWindow(parent)
{
  SharedConstructor();
};

void InteractivePatchComparisonWidget::on_actionQuit_activated()
{
  exit(0);
}

void InteractivePatchComparisonWidget::showEvent(QShowEvent* event)
{
  GetPatchSize();

  // Initialize
//   this->SourcePatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2,
//                                       this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);
//   this->TargetPatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2 + this->PatchSize[0],
//                                       this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);
  this->SourcePatchLayer.ImageSlice->SetPosition(0, 0, 0);
  this->TargetPatchLayer.ImageSlice->SetPosition(20, 20, 0);

  //SetupPatches();

  //PatchesMovedEventHandler();

  this->Renderer->ResetCamera();

  Refresh();
}

void InteractivePatchComparisonWidget::OpenImage(const std::string& fileName)
{
  // Set the working directory
  QFileInfo fileInfo(fileName.c_str());
  std::string workingDirectory = fileInfo.absoluteDir().absolutePath().toStdString() + "/";

  //std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->ImageLayer.ImageData);

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  TargetPatchInfoWidget->SetImage(this->Image.GetPointer());
  SourcePatchInfoWidget->SetImage(this->Image.GetPointer());

  this->TopPatchesPanel->SetImage(this->Image.GetPointer());

  this->ImageLayer.ImageSlice->VisibilityOn();
  this->SourcePatchLayer.ImageSlice->VisibilityOn();
  this->TargetPatchLayer.ImageSlice->VisibilityOn();

  ComputeProjectionMatrix();
}

void InteractivePatchComparisonWidget::OpenMask(const std::string& fileName)
{
  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  if(this->Image->GetLargestPossibleRegion() != reader->GetOutput()->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl;
    return;
    }
  this->MaskImage = Mask::New();
  ITKHelpers::DeepCopy(reader->GetOutput(), this->MaskImage.GetPointer());

  // For this program, we ALWAYS assume the hole to be filled is white, and the valid/source region is black.
  // This is not simply reversible because of some subtle erosion operations that are performed.
  // For this reason, we provide an "load inverted mask" action in the file menu.
  this->MaskImage->SetValidValue(0);
  this->MaskImage->SetHoleValue(255);

  this->MaskImage->Cleanup();

  MaskOperations::SetMaskTransparency(this->MaskImage, this->MaskImageLayer.ImageData);

  this->statusBar()->showMessage("Opened mask.");

  TargetPatchInfoWidget->SetMask(this->MaskImage);
  SourcePatchInfoWidget->SetMask(this->MaskImage);

  this->MaskImageLayer.ImageSlice->VisibilityOn();
  
  Refresh();
}

void InteractivePatchComparisonWidget::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  //std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  OpenImage(fileName.toStdString());
}

void InteractivePatchComparisonWidget::on_txtPatchRadius_returnPressed()
{
  SetupPatches();
}

unsigned int InteractivePatchComparisonWidget::GetPatchRadius()
{
  return this->txtPatchRadius->text().toUInt();
}

void InteractivePatchComparisonWidget::GetPatchSize()
{
  // The edge length of the patch is the (radius*2) + 1
  this->PatchSize[0] = GetPatchRadius() * 2 + 1;
  this->PatchSize[1] = GetPatchRadius() * 2 + 1;
}

void InteractivePatchComparisonWidget::SetupPatches()
{
  GetPatchSize();

  InitializePatch(this->SourcePatchLayer.ImageData, this->Green);
  
  InitializePatch(this->TargetPatchLayer.ImageData, this->Blue);
  
  //PatchesMovedEventHandler();
  Refresh();
}

void InteractivePatchComparisonWidget::InitializePatch(vtkImageData* image, const unsigned char color[3])
{
  // Setup and allocate the image data
  //image->SetNumberOfScalarComponents(4);
  //image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(this->PatchSize[0], this->PatchSize[1], 1);
  //image->AllocateScalars();
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 4);
  
  VTKHelpers::BlankAndOutlineImage(image,color);
}

void InteractivePatchComparisonWidget::on_actionOpenMaskInverted_activated()
{
  //std::cout << "on_actionOpenMaskInverted_activated()" << std::endl;
  on_actionOpenMask_activated();
  this->MaskImage->Invert();
  this->MaskImage->Cleanup();
  }

void InteractivePatchComparisonWidget::on_actionOpenMask_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.bmp);;Image Files(*.mha)");

  //std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  OpenMask(fileName.toStdString());
}

void InteractivePatchComparisonWidget::RefreshSlot()
{
  Refresh();
}

void InteractivePatchComparisonWidget::Refresh()
{
  //std::cout << "Refresh()" << std::endl;
  // this->MaskImageSlice->SetVisibility(this->chkShowMask->isChecked());
  
  this->qvtkWidget->GetRenderWindow()->Render();
  
}

void InteractivePatchComparisonWidget::on_actionFlipImage_activated()
{
// camera->Flip
}

void InteractivePatchComparisonWidget::slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& patches)
{
  if(patches.size() == 0)
  {
    return;
  }
  
  slot_SourcePatchMoved(patches[0]);
  
  VTKHelpers::SetImageSizeToMatch(this->ImageLayer.ImageData, this->SelectedSourcePatchesLayer.ImageData);
  VTKHelpers::ZeroImage(this->SelectedSourcePatchesLayer.ImageData, 4);
  VTKHelpers::MakeImageTransparent(this->SelectedSourcePatchesLayer.ImageData);

  const unsigned char red[3] = {255,0,0};
  for(unsigned int i = 0; i < patches.size(); ++i)
  {
    ITKVTKHelpers::OutlineRegion(this->SelectedSourcePatchesLayer.ImageData, patches[i], red);
  }
  
  this->SelectedSourcePatchesLayer.ImageSlice->VisibilityOn();
  Refresh();
}

void InteractivePatchComparisonWidget::slot_TargetPatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  //std::cout << "slot_TargetPatchMoved" << std::endl;
  
  double targetPosition[3];
  this->TargetPatchLayer.ImageSlice->GetPosition(targetPosition);

  targetPosition[0] = patchRegion.GetIndex()[0];
  targetPosition[1] = patchRegion.GetIndex()[1];
  this->TargetPatchLayer.ImageSlice->SetPosition(targetPosition);

  // Update the TopPatches widget
  this->TopPatchesPanel->SetTargetRegion(patchRegion);
  
  // Refresh
  Refresh();
  UpdatePatches();
}

void InteractivePatchComparisonWidget::slot_SourcePatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  //std::cout << "slot_SourcePatchMoved to " << patchRegion << std::endl;
  
  double sourcePosition[3];
  this->SourcePatchLayer.ImageSlice->GetPosition(sourcePosition);

  sourcePosition[0] = patchRegion.GetIndex()[0];
  sourcePosition[1] = patchRegion.GetIndex()[1];
  this->SourcePatchLayer.ImageSlice->SetPosition(sourcePosition);

  Refresh();
  UpdatePatches();
}

void InteractivePatchComparisonWidget::UpdatePatches()
{
  // Source patch
  double sourcePosition[3];
  this->SourcePatchLayer.ImageSlice->GetPosition(sourcePosition);

  itk::Index<2> sourceCorner;
  sourceCorner[0] = sourcePosition[0];
  sourceCorner[1] = sourcePosition[1];

  // Snap to grid
  sourcePosition[0] = sourceCorner[0];
  sourcePosition[1] = sourceCorner[1];
  this->SourcePatchLayer.ImageSlice->SetPosition(sourcePosition);

  itk::ImageRegion<2> sourceRegion(sourceCorner, this->PatchSize);

  // Patch 2
  double targetPosition[3];
  this->TargetPatchLayer.ImageSlice->GetPosition(targetPosition);

  itk::Index<2> targetCorner;
  targetCorner[0] = targetPosition[0];
  targetCorner[1] = targetPosition[1];

  // Snap to grid
  targetPosition[0] = targetCorner[0];
  targetPosition[1] = targetCorner[1];
  this->TargetPatchLayer.ImageSlice->SetPosition(targetPosition);

  itk::ImageRegion<2> targetRegion(targetCorner, this->PatchSize);

  // If the patch is not inside the image, don't do anything
  if(!Image->GetLargestPossibleRegion().IsInside(targetRegion))
  {
    TargetPatchInfoWidget->MakeInvalid();
  }
  else
  {
    // Set the TopPatchesWidget to use the new target patch
    this->TopPatchesPanel->SetTargetRegion(targetRegion);

    emit signal_TargetPatchMoved(targetRegion);
  }

  if(!Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    SourcePatchInfoWidget->MakeInvalid();
  }
  else
  {
    emit signal_SourcePatchMoved(sourceRegion);
  }

  // If both patches are valid, we can compute the difference
  if(Image->GetLargestPossibleRegion().IsInside(targetRegion) &&
    Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    AverageValueDifference averageValueDifferenceFunctor;
    float averageAbsPixelDifference = averageValueDifferenceFunctor(this->Image.GetPointer(),
                                                                   sourceRegion, targetRegion);

    float averageSqPixelDifference = SSD<ImageType>::Difference(this->Image.GetPointer(),
                                                 sourceRegion, targetRegion);

    VectorType vectorizedSource = PatchProjection<MatrixType, VectorType>::VectorizePatch(this->Image.GetPointer(),
                                                               sourceRegion);

    VectorType vectorizedTarget = PatchProjection<MatrixType, VectorType>::VectorizePatch(this->Image.GetPointer(),
                                                                targetRegion);

    VectorType projectedSource = this->ProjectionMatrix.transpose() * vectorizedSource;

    VectorType projectedTarget = this->ProjectionMatrix.transpose() * vectorizedTarget;

    // Compute distance between patches in PCA space
    float pcaScore = (projectedSource - projectedTarget).squaredNorm();

//     CorrelationScore correlationScoreFunctor;
//     float correlationScore = correlationScoreFunctor(this->Image.GetPointer(), sourceRegion, targetRegion);

    Refresh();

    this->lblSumSquaredPixelDifference->setNum(averageSqPixelDifference);

    this->lblSumAbsolutePixelDifference->setNum(averageAbsPixelDifference);
    // this->lblCorrelation->setNum(correlationScore);
    this->lblPCAScore->setNum(pcaScore);
  }
}

void InteractivePatchComparisonWidget::PatchesMovedEventHandler(vtkObject* caller, long unsigned int eventId,
                                                                void* callData)
{
  vtkProp* prop = static_cast<vtkProp*>(callData);
  if(prop == static_cast<vtkProp*>(TargetPatchLayer.ImageSlice) || prop == static_cast<vtkProp*>(SourcePatchLayer.ImageSlice)) // These casts are necessary because the compiler complains (warns) about mismatched pointer types
    {
    UpdatePatches();
    }
}

void InteractivePatchComparisonWidget::on_action_SavePatches_activated()
{
//   TargetPatchInfoWidget->Save("target");
//   SourcePatchInfoWidget->Save("source");

  itk::ImageRegion<2> sourceRegion = SourcePatchInfoWidget->GetRegion();
  itk::ImageRegion<2> targetRegion = TargetPatchInfoWidget->GetRegion();

  if(!this->Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    std::cerr << "Source region not inside image!" << sourceRegion << std::endl;
    return;
  }

  if(!this->Image->GetLargestPossibleRegion().IsInside(targetRegion))
  {
    std::cerr << "Target region not inside image!" << targetRegion << std::endl;
    return;
  }
  
  AverageValueDifference averageValueDifferenceFunctor;
  float averageValueDifference = averageValueDifferenceFunctor(this->Image.GetPointer(),
                                                               sourceRegion, targetRegion);

  //ITKHelpers::WriteVectorImageRegionAsRGB(this->Image.GetPointer(), sourceRegion, "SourcePatch.png");
  //Helpers::WriteRGBRegion(this->Image.GetPointer(), targetRegion, "TargetPatch.png");
  ImageType::PixelType holeColor;
  holeColor.SetSize(3);
  holeColor.Fill(0);
  MaskOperations::WriteMaskedRegionPNG(this->Image.GetPointer(), this->MaskImage.GetPointer(), targetRegion, "TargetPatch.png", holeColor);

  ITKHelpers::WriteRegion(this->MaskImage.GetPointer(), targetRegion, "MaskPatch.png");
  
  std::ofstream fout("score.txt");
  fout << averageValueDifference << std::endl;
  fout.close();
}

void InteractivePatchComparisonWidget::on_actionScreenshot_activated()
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->qvtkWidget->GetRenderWindow());
  //windowToImageFilter->SetMagnification(3); //set the resolution of the output image (3 times the current resolution of vtk render window)
  //windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName("Screenshot.png");
  writer->SetInputData(windowToImageFilter->GetOutput());
  writer->Write();
}

void InteractivePatchComparisonWidget::on_action_View_TopPatches_activated()
{
  TopPatchesPanel->setVisible(!TopPatchesPanel->isVisible());
}


void InteractivePatchComparisonWidget::ComputeProjectionMatrix()
{
  if(this->PatchSize[0]/2 == 0)
  {
    throw std::runtime_error("Must set PatchSize before calling ComputeProjectionMatrix()!");
  }

  if(this->Image->GetLargestPossibleRegion().GetSize()[0] == 0)
  {
    throw std::runtime_error("Cannot ComputeProjectionMatrix() before calling SetImage()!");
  }
  std::vector<typename VectorType::Scalar> sortedEigenvalues; // unused
  VectorType meanVector; // unused

  this->ProjectionMatrix = PatchProjection<MatrixType, VectorType>::
                              ComputeProjectionMatrixFromImagePartialMatrix(this->Image.GetPointer(),
                                                               this->PatchSize[0]/2,
                                                               meanVector,
                                                               sortedEigenvalues);
                              
//   this->ProjectionMatrix = PatchProjection<MatrixType, VectorType>::
//                               ComputeProjectionMatrixFromImageOuterProduct(this->Image.GetPointer(),
//                                                                this->PatchSize[0]/2,
//                                                                meanVector,
//                                                                sortedEigenvalues);

  // This function would be preferred, but typically the feature matrix does not fit into memory.
//   this->ProjectionMatrix = PatchProjection<MatrixType, VectorType>::
//                               ComputeProjectionMatrix_CovarianceEigen(this->Image.GetPointer(),
//                                                                       this->PatchSize[0]/2,
//                                                                       meanVector,
//                                                                       sortedEigenvalues);

  unsigned int numberOfDimensionsToProjectTo = 150;
  this->ProjectionMatrix =
         EigenHelpers::TruncateColumns(this->ProjectionMatrix, numberOfDimensionsToProjectTo);

  this->TopPatchesPanel->SetProjectionMatrix(this->ProjectionMatrix);
}

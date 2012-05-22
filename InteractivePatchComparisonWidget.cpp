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
#include "EigenHelpers/EigenHelpers.h"

// Custom
#include "SwitchBetweenStyle.h"
#include "Types.h"

// Patch Comparison Submodule
#include "PatchComparison/CorrelationScore.hpp"
#include "PatchComparison/AveragePixelDifference.hpp"
#include "PatchComparison/PixelDifferences.hpp"
#include "PatchComparison/DiffusionDistance.h"

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
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);
/*
  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);*/

  this->InteractorStyle = vtkSmartPointer<SwitchBetweenStyle>::New();

  // Initialize and link the image display objects
  this->VTKImage = vtkSmartPointer<vtkImageData>::New();
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ImageSliceMapper->BorderOn();
  this->ImageSlice->PickableOff();
  this->ImageSliceMapper->SetInputData(this->VTKImage);
  this->ImageSlice->SetMapper(this->ImageSliceMapper);
  this->ImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->ImageSlice->VisibilityOff(); // There are errors if this is visible and therefore displayed before it has data ("This data object does not contain the requested extent.")

  // Initialize and link the mask image display objects
  this->VTKMaskImage = vtkSmartPointer<vtkImageData>::New();
  this->MaskImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->MaskImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->MaskImageSlice->PickableOff();
  this->MaskImageSliceMapper->BorderOn();
  this->MaskImageSliceMapper->SetInputData(this->VTKMaskImage);
  this->MaskImageSlice->SetMapper(this->MaskImageSliceMapper);
  this->MaskImageSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->MaskImageSlice->VisibilityOff();

  // Initialize patches
  this->SourcePatch = vtkSmartPointer<vtkImageData>::New();
  this->SourcePatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->SourcePatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->SourcePatchSliceMapper->BorderOn();
  this->SourcePatchSliceMapper->SetInputData(this->SourcePatch);
  this->SourcePatchSlice->SetMapper(this->SourcePatchSliceMapper);
  this->SourcePatchSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->SourcePatchSlice->VisibilityOff();

  this->TargetPatch = vtkSmartPointer<vtkImageData>::New();
  this->TargetPatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->TargetPatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->TargetPatchSliceMapper->BorderOn();
  this->TargetPatchSliceMapper->SetInputData(this->TargetPatch);
  this->TargetPatchSlice->SetMapper(this->TargetPatchSliceMapper);
  this->TargetPatchSlice->GetProperty()->SetInterpolationTypeToNearest();
  this->TargetPatchSlice->VisibilityOff();

  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);

  this->Renderer->AddViewProp(this->ImageSlice);
  this->Renderer->AddViewProp(this->MaskImageSlice);
  this->Renderer->AddViewProp(this->SourcePatchSlice);
  this->Renderer->AddViewProp(this->TargetPatchSlice);

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
  connect(this->TopPatchesPanel, SIGNAL(signal_TopPatchSelected(const itk::ImageRegion<2>&)),
          this, SLOT(slot_SourcePatchMoved(const itk::ImageRegion<2>& )));

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
  this->SourcePatchSlice->SetPosition(0, 0, 0);
  this->TargetPatchSlice->SetPosition(20, 20, 0);

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

  std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->VTKImage);

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  TargetPatchInfoWidget->SetImage(this->Image.GetPointer());
  SourcePatchInfoWidget->SetImage(this->Image.GetPointer());

  this->TopPatchesPanel->SetImage(this->Image.GetPointer());

   this->ImageSlice->VisibilityOn();
   this->SourcePatchSlice->VisibilityOn();
   this->TargetPatchSlice->VisibilityOn();
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

  MaskOperations::SetMaskTransparency(this->MaskImage, this->VTKMaskImage);

  this->statusBar()->showMessage("Opened mask.");

  TargetPatchInfoWidget->SetMask(this->MaskImage);
  SourcePatchInfoWidget->SetMask(this->MaskImage);

  this->MaskImageSlice->VisibilityOn();
  
  Refresh();
}

void InteractivePatchComparisonWidget::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
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

  InitializePatch(this->SourcePatch, this->Green);
  
  InitializePatch(this->TargetPatch, this->Blue);
  
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
  std::cout << "on_actionOpenMaskInverted_activated()" << std::endl;
  on_actionOpenMask_activated();
  this->MaskImage->Invert();
  this->MaskImage->Cleanup();
  }

void InteractivePatchComparisonWidget::on_actionOpenMask_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.bmp);;Image Files(*.mha)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
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

void InteractivePatchComparisonWidget::slot_TargetPatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  std::cout << "slot_TargetPatchMoved" << std::endl;
  
  double targetPosition[3];
  this->TargetPatchSlice->GetPosition(targetPosition);

  targetPosition[0] = patchRegion.GetIndex()[0];
  targetPosition[1] = patchRegion.GetIndex()[1];
  this->TargetPatchSlice->SetPosition(targetPosition);

  // Update the TopPatches widget
  this->TopPatchesPanel->SetTargetRegion(patchRegion);
  
  // Refresh
  Refresh();
  UpdatePatches();
}

void InteractivePatchComparisonWidget::slot_SourcePatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  double sourcePosition[3];
  this->SourcePatchSlice->GetPosition(sourcePosition);

  sourcePosition[0] = patchRegion.GetIndex()[0];
  sourcePosition[1] = patchRegion.GetIndex()[1];
  this->SourcePatchSlice->SetPosition(sourcePosition);

  Refresh();
  UpdatePatches();
}

void InteractivePatchComparisonWidget::UpdatePatches()
{
  // Source patch
  double sourcePosition[3];
  this->SourcePatchSlice->GetPosition(sourcePosition);

  itk::Index<2> sourceCorner;
  sourceCorner[0] = sourcePosition[0];
  sourceCorner[1] = sourcePosition[1];

  // Snap to grid
  sourcePosition[0] = sourceCorner[0];
  sourcePosition[1] = sourceCorner[1];
  this->SourcePatchSlice->SetPosition(sourcePosition);

  itk::ImageRegion<2> sourceRegion(sourceCorner, this->PatchSize);

  // Patch 2
  double targetPosition[3];
  this->TargetPatchSlice->GetPosition(targetPosition);

  itk::Index<2> targetCorner;
  targetCorner[0] = targetPosition[0];
  targetCorner[1] = targetPosition[1];

  // Snap to grid
  targetPosition[0] = targetCorner[0];
  targetPosition[1] = targetCorner[1];
  this->TargetPatchSlice->SetPosition(targetPosition);

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
    AveragePixelDifference<SumOfAbsoluteDifferences> averageAbsPixelDifferenceFunctor;
    float averageAbsPixelDifference = averageAbsPixelDifferenceFunctor(this->Image.GetPointer(),
                                                                  this->MaskImage, sourceRegion, targetRegion);

    AveragePixelDifference<SumOfSquaredDifferences> averageSqPixelDifferenceFunctor;
    float averageSqPixelDifference = averageSqPixelDifferenceFunctor(this->Image.GetPointer(),
                                                                this->MaskImage, sourceRegion, targetRegion);

    CorrelationScore correlationScoreFunctor;
    float correlationScore = correlationScoreFunctor(this->Image.GetPointer(), this->MaskImage, sourceRegion, targetRegion);

    Refresh();

    this->lblSumSquaredPixelDifference->setNum(averageSqPixelDifference);

    this->lblSumAbsolutePixelDifference->setNum(averageAbsPixelDifference);
    this->lblCorrelation->setNum(correlationScore);
  }
}

void InteractivePatchComparisonWidget::PatchesMovedEventHandler(vtkObject* caller, long unsigned int eventId,
                                                                void* callData)
{
  vtkProp* prop = static_cast<vtkProp*>(callData);
  if(prop == static_cast<vtkProp*>(TargetPatchSlice) || prop == static_cast<vtkProp*>(SourcePatchSlice)) // These casts are necessary because the compiler complains (warns) about mismatched pointer types
    {
    UpdatePatches();
    }
}

void InteractivePatchComparisonWidget::on_btnSavePatches_clicked()
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
  
  AveragePixelDifference<SumOfAbsoluteDifferences> averagePixelDifferenceFunctor;
  float averagePixelDifference = averagePixelDifferenceFunctor(this->Image.GetPointer(),
                                                               this->MaskImage, sourceRegion, targetRegion);

  //ITKHelpers::WriteVectorImageRegionAsRGB(this->Image.GetPointer(), sourceRegion, "SourcePatch.png");
  //Helpers::WriteRGBRegion(this->Image.GetPointer(), targetRegion, "TargetPatch.png");
  ImageType::PixelType holeColor;
  holeColor.SetSize(3);
  holeColor.Fill(0);
  MaskOperations::WriteMaskedRegionPNG(this->Image.GetPointer(), this->MaskImage.GetPointer(), targetRegion, "TargetPatch.png", holeColor);

  ITKHelpers::WriteRegion(this->MaskImage.GetPointer(), targetRegion, "MaskPatch.png");
  
  std::ofstream fout("score.txt");
  fout << averagePixelDifference << std::endl;
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

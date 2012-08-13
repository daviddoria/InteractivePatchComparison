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
#include <QLineEdit>
#include <QTextEdit> // For the help()

// VTK
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkPNGWriter.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Submodules
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "PatchComparison/Mask/ITKHelpers/Helpers/Helpers.h"
#include "PatchComparison/Mask/Mask.h"
#include "PatchComparison/Mask/MaskOperations.h"
#include "VTKHelpers/VTKHelpers.h"
#include "PatchComparison/EigenHelpers/EigenHelpers.h"
#include "PatchComparison/PatchProjection/PatchProjection.h"
#include "QtHelpers/QtHelpers.h"

// Custom
#include "SwitchBetweenStyle.h"
#include "Types.h"
#include "OddValidator.h"

// Patch Comparison Submodule
#include "PatchComparison/AverageValueDifference.h"
#include "PatchComparison/DiffusionDistance.h"
#include "PatchComparison/CorrelationScore.h"
#include "PatchComparison/PixelDifferences.h"
#include "PatchComparison/SSD.h"
#include "PatchComparison/HistogramDistance.h"
#include "PatchComparison/ProjectedDistance.h"
#include "PatchComparison/LocalPCADistance.h"

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
InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(QWidget* parent) : QMainWindow(parent)
{
  SharedConstructor();
};

InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(const std::string& imageFileName,
                                                                   QWidget* parent)
: QMainWindow(parent)
{
  SharedConstructor();
  this->ImageFileName = imageFileName;
}

InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(const std::string& imageFileName,
                                                                   const std::string& maskFileName,
                                                                   QWidget* parent)
: QMainWindow(parent)
{
  SharedConstructor();
  this->ImageFileName = imageFileName;
  this->MaskFileName = maskFileName;
}

void InteractivePatchComparisonWidget::SharedConstructor()
{
  this->setupUi(this);

  this->TargetPatchInfoWidget = new PatchInfoWidget<ImageType>;
  this->TargetPatchInfoWidgetPlaceholder->addWidget(this->TargetPatchInfoWidget);
  this->TargetPatchInfoWidgetPlaceholder->setCurrentWidget(this->TargetPatchInfoWidget);
  
  this->SourcePatchInfoWidget = new PatchInfoWidget<ImageType>;
  this->SourcePatchInfoWidgetPlaceholder->addWidget(this->SourcePatchInfoWidget);
  this->SourcePatchInfoWidgetPlaceholder->setCurrentWidget(this->SourcePatchInfoWidget);

  OddValidator* oddValidator = new OddValidator;
  this->spinPatchRadius->findChild<QLineEdit*>()->setValidator(oddValidator);

  // Use event filters to handle focus change events
  this->spinPatchRadius->findChild<QLineEdit*>()->installEventFilter(this);

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
  // There are errors if this is visible and therefore displayed before it has data
  // ("This data object does not contain the requested extent.")
  this->ImageLayer.ImageSlice->VisibilityOff();

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

  itkvtkCamera.Initialize(this->InteractorStyle->ImageStyle, this->Renderer,
                          this->qvtkWidget->GetRenderWindow());

  // I can never decide if it is better to create these here and then check for their
  // existance with if(this->MaskImage->GetLargestPossibleRegion().GetSize()[0] > 0)
  // or to set these to NULL and create them when the are set.
  //this->Image = ImageType::New();
  //this->MaskImage = Mask::New();
  this->Image = NULL;
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

  /** Alert the PatchInfoWidgets when the patches have moved. These are typically called when the
    * user specifies the position of the patches using the text boxes.*/
  connect(this, SIGNAL(signal_TargetPatchMoved(const itk::ImageRegion<2>&)),
          TargetPatchInfoWidget, SLOT(slot_SetRegion(const itk::ImageRegion<2>& )));

  connect(this, SIGNAL(signal_SourcePatchMoved(const itk::ImageRegion<2>&)),
          SourcePatchInfoWidget, SLOT(slot_SetRegion(const itk::ImageRegion<2>& )));

}

void InteractivePatchComparisonWidget::on_actionQuit_activated()
{
  exit(0);
}

void InteractivePatchComparisonWidget::showEvent(QShowEvent* event)
{
  GetPatchSizeFromGUI();

  // Set the patches to be somewhere near the middle of the image
//   this->SourcePatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2,
//                                       this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);
//   this->TargetPatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2 +
//                                       this->PatchSize[0],
//                                       this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);

  // Set the patches to be near the bottom left of the image
  this->SourcePatchLayer.ImageSlice->SetPosition(0, 0, 0);
  this->TargetPatchLayer.ImageSlice->SetPosition(20, 20, 0);

  if(this->MaskFileName.size() > 0)
  {
    OpenMask(this->MaskFileName);
  }

  if(this->ImageFileName.size() > 0)
  {
    OpenImage(this->ImageFileName);
  }

  this->Renderer->ResetCamera();

  Refresh();

  std::cout << "InteractivePatchComparisonWidget showEvent" << std::endl;
  this->TargetPatchInfoWidget->Update();
  this->SourcePatchInfoWidget->Update();
}

void InteractivePatchComparisonWidget::OpenImage(const std::string& fileName)
{
  this->Image = ImageType::New();

  // Create a FileInfo object to get extensions, etc.
  QFileInfo fileInfo(fileName.c_str());

  // Set the working directory
//   std::string workingDirectory = fileInfo.absoluteDir().absolutePath().toStdString() + "/";
//   QDir::setCurrent(QString(workingDirectory.c_str()));
//   std::cout << "Working directory set to: " << workingDirectory << std::endl;

  typedef itk::ImageFileReader<ImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  ITKHelpers::DeepCopy(reader->GetOutput(), this->Image.GetPointer());

  ITKVTKHelpers::ITKImageToVTKRGBImage(this->Image.GetPointer(), this->ImageLayer.ImageData);

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  this->TargetPatchInfoWidget->SetImage(this->Image.GetPointer());
  this->SourcePatchInfoWidget->SetImage(this->Image.GetPointer());

  this->ImageLayer.ImageSlice->VisibilityOn();
  this->SourcePatchLayer.ImageSlice->VisibilityOn();
  this->TargetPatchLayer.ImageSlice->VisibilityOn();

  // Generate a fully valid mask if one has not been set.
  //if(this->MaskImage->GetLargestPossibleRegion().GetSize()[0] == 0)
  if(!this->MaskImage)
  {
    this->MaskImage = Mask::New();
    this->MaskImage->SetRegions(this->Image->GetLargestPossibleRegion());
    this->MaskImage->Allocate();
    ITKHelpers::SetImageToConstant(this->MaskImage.GetPointer(), this->MaskImage->GetValidValue());
    this->TargetPatchInfoWidget->SetMask(this->MaskImage);
    this->SourcePatchInfoWidget->SetMask(this->MaskImage);
  }

  SetupDistanceFunctors();

  UpdatePatches();

  if(fileInfo.suffix() == "png")
  {
    itkvtkCamera.SetCameraPositionPNG();
  }
  else if(fileInfo.suffix() == "mha")
  {
    itkvtkCamera.SetCameraPositionMHA();
  }
}

void InteractivePatchComparisonWidget::OpenMask(const std::string& fileName)
{
  this->MaskImage = Mask::New();
  this->MaskImage->Read(fileName);

  // If the image has already been loaded, make sure the image size matches the mask size
  if( (this->Image->GetLargestPossibleRegion().GetSize()[0] > 0) &&
      (this->Image->GetLargestPossibleRegion() != this->MaskImage->GetLargestPossibleRegion()) )
    {
    std::cerr << "OpenMask(): Image and mask must be the same size!" << std::endl;
    this->MaskImage = NULL;
    return;
    }

  MaskOperations::SetMaskTransparency(this->MaskImage, this->MaskImageLayer.ImageData);

  this->statusBar()->showMessage("Opened mask.");

  this->TargetPatchInfoWidget->SetMask(this->MaskImage);
  this->SourcePatchInfoWidget->SetMask(this->MaskImage);

  this->MaskImageLayer.ImageSlice->VisibilityOn();

  Refresh();
}

void InteractivePatchComparisonWidget::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".",
                                    "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  //std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  OpenImage(fileName.toStdString());
}

void InteractivePatchComparisonWidget::on_spinPatchRadius_valueChanged(int value)
{
  if(!this->spinPatchRadius->hasAcceptableInput())
  {
    std::cerr << "Invalid patch radius!" << std::endl;
    return;
  }

  QColor normalColor = QColor(255, 255, 255);
  QPalette p = this->spinPatchRadius->findChild<QLineEdit*>()->palette();
  p.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->spinPatchRadius->findChild<QLineEdit*>()->setPalette(p);

  unsigned int guiPatchRadius = this->spinPatchRadius->value();
  // If the radius has changed
  if(this->PatchSize[0] != Helpers::SideLengthFromRadius(guiPatchRadius))
  {
    GetPatchSizeFromGUI();
    SetupPatches();
    SetupDistanceFunctors();
    UpdatePatches();
  }
}

void InteractivePatchComparisonWidget::GetPatchSizeFromGUI()
{
  unsigned int patchRadius = this->spinPatchRadius->value();

  this->PatchSize[0] = Helpers::SideLengthFromRadius(patchRadius);
  this->PatchSize[1] = Helpers::SideLengthFromRadius(patchRadius);
}

void InteractivePatchComparisonWidget::SetupPatches()
{
  GetPatchSizeFromGUI();

  QPalette sourcePalette = this->lblSourcePatch->palette();
  QColor qsourceColor = sourcePalette.color( QPalette::Normal, QPalette::WindowText);
  //std::cout << qsourceColor << std::endl;
  unsigned char sourceColor[3];
  QtHelpers::QColorToUCharColor(qsourceColor, sourceColor);
  InitializePatch(this->SourcePatchLayer.ImageData, sourceColor);

  QPalette targetPalette = this->lblTargetPatch->palette();
  QColor qtargetColor = targetPalette.color( QPalette::Normal, QPalette::WindowText);
  unsigned char targetColor[3];
  QtHelpers::QColorToUCharColor(qtargetColor, targetColor);
  InitializePatch(this->TargetPatchLayer.ImageData, targetColor);

  Refresh();
}

void InteractivePatchComparisonWidget::InitializePatch(vtkImageData* const image,
                                                       const unsigned char color[3])
{
  // Setup and allocate the image data
  image->SetDimensions(this->PatchSize[0], this->PatchSize[1], 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

  VTKHelpers::BlankAndOutlineImage(image, color);
}

void InteractivePatchComparisonWidget::on_actionOpenMaskInverted_activated()
{
  //std::cout << "on_actionOpenMaskInverted_activated()" << std::endl;
  on_actionOpenMask_activated();
  this->MaskImage->InvertData();
  this->MaskImage->Cleanup();
  }

void InteractivePatchComparisonWidget::on_actionOpenMask_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Mask Files (*.mask)");

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
  this->qvtkWidget->GetRenderWindow()->Render();
}

void InteractivePatchComparisonWidget::on_actionFlipVertically_activated()
{
  itkvtkCamera.FlipVertically();
}

void InteractivePatchComparisonWidget::slot_SelectedPatchesChanged(
     const std::vector<itk::ImageRegion<2> >& patches)
{
  if(patches.size() == 0)
  {
    return;
  }

  slot_SourcePatchMoved(patches[0]);

  VTKHelpers::SetImageSizeToMatch(this->ImageLayer.ImageData, this->SelectedSourcePatchesLayer.ImageData);
  VTKHelpers::ZeroImage(this->SelectedSourcePatchesLayer.ImageData, 4);
  VTKHelpers::MakeImageTransparent(this->SelectedSourcePatchesLayer.ImageData);

  //const unsigned char outlineColor[3] = {255,0,0}; // red
  const unsigned char outlineColor[3] = {0,0,255}; // blue
  
  for(unsigned int i = 0; i < patches.size(); ++i)
  {
    ITKVTKHelpers::OutlineRegion(this->SelectedSourcePatchesLayer.ImageData, patches[i], outlineColor);
  }

  this->SelectedSourcePatchesLayer.ImageSlice->VisibilityOn();
  Refresh();
}

void InteractivePatchComparisonWidget::slot_TargetPatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  //std::cout << "slot_TargetPatchMoved" << std::endl;

  if(!this->Image->GetLargestPossibleRegion().IsInside(patchRegion))
  {
    std::cerr << "Invalid patch position specified!" << std::endl;
    return;
  }

  double targetPosition[3];
  this->TargetPatchLayer.ImageSlice->GetPosition(targetPosition);

  targetPosition[0] = patchRegion.GetIndex()[0];
  targetPosition[1] = patchRegion.GetIndex()[1];
  this->TargetPatchLayer.ImageSlice->SetPosition(targetPosition);

  // Update the TopPatches widgets
  for(unsigned int i = 0; i < this->TopPatchesWidgets.size(); ++i)
  {
    this->TopPatchesWidgets[i]->SetTargetRegion(patchRegion);
  }

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
    // Set the TopPatchesWidgets to use the new target patch
    for(unsigned int i = 0; i < this->TopPatchesWidgets.size(); ++i)
    {
      this->TopPatchesWidgets[i]->SetTargetRegion(targetRegion);
    }

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
  if(this->Image->GetLargestPossibleRegion().IsInside(targetRegion) &&
     this->Image->GetLargestPossibleRegion().IsInside(sourceRegion))
  {
    for(size_t i = 0; i < this->DistanceFunctors.size(); ++i)
    {
      float distance = this->DistanceFunctors[i]->Distance(sourceRegion, targetRegion);
      std::stringstream ss;
      ss << this->DistanceFunctors[i]->GetDistanceName() << ": " << distance;
      this->ScoreDisplayMap[this->DistanceFunctors[i]]->setText(ss.str().c_str());
    }

    Refresh();
  }
}

void InteractivePatchComparisonWidget::PatchesMovedEventHandler(vtkObject* caller, long unsigned int eventId,
                                                                void* callData)
{
  vtkProp* prop = static_cast<vtkProp*>(callData);
  // These casts are necessary because the compiler complains (warns) about mismatched pointer types
  if(prop == static_cast<vtkProp*>(TargetPatchLayer.ImageSlice) ||
     prop == static_cast<vtkProp*>(SourcePatchLayer.ImageSlice)) 
    {
    UpdatePatches();
    }
}

void InteractivePatchComparisonWidget::on_action_SavePatches_activated()
{
  this->TargetPatchInfoWidget->Save("target.png");
  this->SourcePatchInfoWidget->Save("source.png");
}

void InteractivePatchComparisonWidget::on_actionScreenshot_activated()
{
  vtkSmartPointer<vtkWindowToImageFilter> windowToImageFilter =
    vtkSmartPointer<vtkWindowToImageFilter>::New();
  windowToImageFilter->SetInput(this->qvtkWidget->GetRenderWindow());
  // Set the resolution of the output image (3 times the current resolution of vtk render window)
  //windowToImageFilter->SetMagnification(3); 
  //windowToImageFilter->SetInputBufferTypeToRGBA(); //also record the alpha (transparency) channel
  windowToImageFilter->Update();

  vtkSmartPointer<vtkPNGWriter> writer = vtkSmartPointer<vtkPNGWriter>::New();
  writer->SetFileName("Screenshot.png");
  writer->SetInputData(windowToImageFilter->GetOutput());
  writer->Write();
}

void InteractivePatchComparisonWidget::SetupDistanceFunctors()
{
  if(this->PatchSize[0]/2 == 0)
  {
    throw std::runtime_error("Must set PatchSize before calling SetupDistanceFunctors()!");
  }

  if(this->Image->GetLargestPossibleRegion().GetSize()[0] == 0)
  {
    throw std::runtime_error("Cannot SetupDistanceFunctors() before calling SetImage()!");
  }

  this->TopPatchesWidgets.clear();

  ////////////////// Setup the normal top patches widget //////////////////
  SSD<ImageType>* ssdDistanceFunctor = new SSD<ImageType>;
  ssdDistanceFunctor->SetImage(this->Image);
  this->DistanceFunctors.push_back(ssdDistanceFunctor);

  QLabel* ssdLabel = new QLabel;
  this->layoutScores->addWidget(ssdLabel);
  this->ScoreDisplayMap[ssdDistanceFunctor] = ssdLabel;

  TopPatchesWidget<ImageType>* ssdTopPatchesWidget = new TopPatchesWidget<ImageType>;
  ssdTopPatchesWidget->SetPatchDistanceFunctor(ssdDistanceFunctor);
  ssdTopPatchesWidget->SetImage(this->Image);
  ssdTopPatchesWidget->setWindowTitle("SSD");
  this->TopPatchesWidgets.push_back(ssdTopPatchesWidget);
  ssdTopPatchesWidget->show();

  // This is used when the user clicks on a top patch in the view of the top patches.
  connect(ssdTopPatchesWidget,
          SIGNAL(signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >&)),
          this, SLOT(slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& )));

  ////////////////// Setup the histogram top patches widget //////////////////
  // RGB Histogram
//   HistogramDistance<ImageType>* histogramDistanceFunctor = new HistogramDistance<ImageType>;
//   histogramDistanceFunctor->SetImage(this->Image);

  typedef itk::VectorImage<float, 2> HSVImageType;
  HSVImageType::Pointer hsvImage = HSVImageType::New();
  ITKHelpers::ITKImageToHSVImage(this->Image.GetPointer(), hsvImage.GetPointer());

  ITKHelpers::ScaleAllChannelsTo255(hsvImage.GetPointer());
  this->HSVImage = ImageType::New();
  ITKHelpers::CastImage(hsvImage.GetPointer(), this->HSVImage.GetPointer());

  // Convert back to an uchar image so our distance functor vector can hold the object
  HistogramDistance<ImageType>* histogramDistanceFunctor = new HistogramDistance<ImageType>;
  histogramDistanceFunctor->SetImage(this->HSVImage);
  histogramDistanceFunctor->SetDistanceNameModifier("HSV");
  this->DistanceFunctors.push_back(histogramDistanceFunctor);

  QLabel* histogramDistanceLabel = new QLabel;
  this->layoutScores->addWidget(histogramDistanceLabel);
  this->ScoreDisplayMap[histogramDistanceFunctor] = histogramDistanceLabel;
  //ssdTopPatchesWidget->SetSecondaryPatchDistanceFunctor(histogramDistanceFunctor);

  // It is much too slow to compare histograms for every source patch
//   TopPatchesWidget<ImageType>* histogramTopPatchesWidget = new TopPatchesWidget<ImageType>;
//   histogramTopPatchesWidget->SetPatchDistanceFunctor(histogramDistanceFunctor);
//   histogramTopPatchesWidget->SetImage(this->Image);
//   histogramTopPatchesWidget->setWindowTitle("Histogram Distance");
//   this->TopPatchesWidgets.push_back(histogramTopPatchesWidget);
//   histogramTopPatchesWidget->show();
// 
//   // This is used when the user clicks on a top patch in the view of the top patches.
//   connect(histogramTopPatchesWidget,
//           SIGNAL(signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >&)),
//           this, SLOT(slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& )));

  //////////////// Setup the blurred top patches widget //////////////////
//   this->BlurredImage = ImageType::New();
//   //float sigma = 2.0f;
//   //ITKHelpers::BlurAllChannels(this->Image.GetPointer(), this->BlurredImage.GetPointer(), sigma);
// 
//   float domainSigma = 5.0f;
//   float rangeSigma = 50.0f;
//   ITKHelpers::BilateralFilterAllChannels(this->Image.GetPointer(), this->BlurredImage.GetPointer(), domainSigma, rangeSigma);
//   ITKHelpers::WriteRGBImage(this->BlurredImage.GetPointer(), "blurred.png");
// 
//   SSD<ImageType>* blurredSSDDistanceFunctor = new SSD<ImageType>;
//   blurredSSDDistanceFunctor->SetImage(this->BlurredImage);
// 
//   TopPatchesWidget<ImageType>* blurredTopPatchesWidget = new TopPatchesWidget<ImageType>;
//   blurredTopPatchesWidget->setWindowTitle("Blurred SSD");
//   //blurredTopPatchesWidget->SetImage(this->Image);
//   blurredTopPatchesWidget->SetImage(this->BlurredImage);
//   blurredTopPatchesWidget->SetPatchDistanceFunctor(blurredSSDDistanceFunctor);
//   this->TopPatchesWidgets.push_back(blurredTopPatchesWidget);
//   blurredTopPatchesWidget->show();
// 
//   // This is used when the user clicks on a top patch in the view of the top patches.
//   connect(blurredTopPatchesWidget,
//           SIGNAL(signal_TopPatchesSelected(const std::vector<itk::ImageRegion<2> >&)),
//           this, SLOT(slot_SelectedPatchesChanged(const std::vector<itk::ImageRegion<2> >& )));

}

bool InteractivePatchComparisonWidget::eventFilter(QObject *object, QEvent *event)
{
  // When the focus leaves one of the text boxes, update the patches
  QColor normalColor = QColor(255, 255, 255);
  if(object == spinPatchRadius->findChild<QLineEdit*>() && event->type() == QEvent::FocusOut)
  {
    if(!this->spinPatchRadius->hasAcceptableInput())
    {
      std::cerr << "Invalid patch radius!" << std::endl;
      return false; // Pass the event along (don't consume it)
    }
    unsigned int guiPatchRadius = this->spinPatchRadius->value();
    // If the radius has changed
    if(this->PatchSize[0] != Helpers::SideLengthFromRadius(guiPatchRadius))
    {
      GetPatchSizeFromGUI();
      SetupPatches();
      SetupDistanceFunctors();
      UpdatePatches();
    }
    QPalette p = spinPatchRadius->findChild<QLineEdit*>()->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    spinPatchRadius->findChild<QLineEdit*>()->setPalette(p);
  }

  return false; // Pass the event along (don't consume it)
}

void InteractivePatchComparisonWidget::closeEvent(QCloseEvent* event)
{
  QApplication::quit();
}

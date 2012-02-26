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

#include "ui_InteractivePatchComparisonWidget.h"
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
#include <vtkImageSliceMapper.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLImageDataWriter.h> // For debugging only

// Custom
#include "Helpers.h"
#include "SwitchBetweenStyle.h"
#include "Mask.h"
#include "Types.h"
#include "CorrelationScore.hpp"
#include "AveragePixelDifference.hpp"
#include "PixelDifferences.hpp"
#include "DiffusionDistance.h"

const unsigned char InteractivePatchComparisonWidget::Green[3] = {0,255,0};
const unsigned char InteractivePatchComparisonWidget::Red[3] = {255,0,0};
const unsigned char InteractivePatchComparisonWidget::Blue[3] = {0,0,255};

void InteractivePatchComparisonWidget::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Interactive Patch Comparison</h1>\
  Position the two patches. <br/>\
  Their difference will be displayed.<br/> <p/>");
  help->show();
}

// Constructors
InteractivePatchComparisonWidget::InteractivePatchComparisonWidget(const std::string& imageFileName,
                                                                   const std::string& maskFileName)
{
  SharedConstructor();
  OpenImage(imageFileName);
  OpenMask(maskFileName);

  // ComputeFeatureMatrixStatistics();
  FeatureMeans.resize(6);
  FeatureMeans[0] = 84.5227;
  FeatureMeans[1] = 83.7242;
  FeatureMeans[2] = 66.7671;
  FeatureMeans[3] = 124.39;
  FeatureMeans[4] = 124.645;
  FeatureMeans[5] = 126.41;

  FeatureStandardDeviations.resize(6);
  FeatureStandardDeviations[0] = 26.4461;
  FeatureStandardDeviations[1] = 25.3215;
  FeatureStandardDeviations[2] = 22.657;
  FeatureStandardDeviations[3] = 74.04;
  FeatureStandardDeviations[4] = 73.3509;
  FeatureStandardDeviations[5] = 72.8899;

}

void InteractivePatchComparisonWidget::SharedConstructor()
{
  this->setupUi(this);

  this->PatchScale = 5;

  // Setup icons
  QIcon openIcon = QIcon::fromTheme("document-open");
  QIcon saveIcon = QIcon::fromTheme("document-save");

  // Setup toolbar
  actionOpenImage->setIcon(openIcon);
  this->toolBar->addAction(actionOpenImage);

  actionOpenMask->setIcon(openIcon);
  this->toolBar->addAction(actionOpenMask);
  actionOpenMask->setEnabled(false);

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->Flipped = false;

  this->InteractorStyle = vtkSmartPointer<SwitchBetweenStyle>::New();

  // Initialize and link the image display objects
  this->VTKImage = vtkSmartPointer<vtkImageData>::New();
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ImageSliceMapper->BorderOn();
  this->ImageSlice->PickableOff();
  this->ImageSliceMapper->SetInputConnection(this->VTKImage->GetProducerPort());
  this->ImageSlice->SetMapper(this->ImageSliceMapper);
  this->ImageSlice->GetProperty()->SetInterpolationTypeToNearest();

  // Initialize and link the mask image display objects
  this->VTKMaskImage = vtkSmartPointer<vtkImageData>::New();
  this->MaskImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->MaskImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->MaskImageSlice->PickableOff();
  this->MaskImageSliceMapper->BorderOn();
  this->MaskImageSliceMapper->SetInputConnection(this->VTKMaskImage->GetProducerPort());
  this->MaskImageSlice->SetMapper(this->MaskImageSliceMapper);
  this->MaskImageSlice->GetProperty()->SetInterpolationTypeToNearest();

  // Initialize patches
  this->SourcePatch = vtkSmartPointer<vtkImageData>::New();
  this->SourcePatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->SourcePatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->SourcePatchSliceMapper->BorderOn();
  this->SourcePatchSliceMapper->SetInputConnection(this->SourcePatch->GetProducerPort());
  this->SourcePatchSlice->SetMapper(this->SourcePatchSliceMapper);
  this->SourcePatchSlice->GetProperty()->SetInterpolationTypeToNearest();

  this->TargetPatch = vtkSmartPointer<vtkImageData>::New();
  this->TargetPatchSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->TargetPatchSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->TargetPatchSliceMapper->BorderOn();
  this->TargetPatchSliceMapper->SetInputConnection(this->TargetPatch->GetProducerPort());
  this->TargetPatchSlice->SetMapper(this->TargetPatchSliceMapper);
  this->TargetPatchSlice->GetProperty()->SetInterpolationTypeToNearest();

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

  //this->Image = FloatVectorImageType::New();
  //this->MaskImage = Mask::New();
  this->Image = NULL;
  this->MaskImage = NULL;

  this->InteractorStyle->TrackballStyle->AddObserver(CustomTrackballStyle::PatchesMovedEvent, this,
                                                     &InteractivePatchComparisonWidget::PatchesMovedEventHandler);

  connect(TargetPatchInfoWidget, SIGNAL(signal_PatchMoved(const itk::ImageRegion<2>&)),
          this, SLOT(slot_TargetPatchMoved(const itk::ImageRegion<2>&)));
  connect(SourcePatchInfoWidget, SIGNAL(signal_PatchMoved(const itk::ImageRegion<2>&)),
          this, SLOT(slot_SourcePatchMoved(const itk::ImageRegion<2>&)));

  connect(this, SIGNAL(signal_TargetPatchMoved(const itk::ImageRegion<2>&)),
          TargetPatchInfoWidget, SLOT(slot_Update(const itk::ImageRegion<2>& )));

  connect(this, SIGNAL(signal_SourcePatchMoved(const itk::ImageRegion<2>&)),
          SourcePatchInfoWidget, SLOT(slot_Update(const itk::ImageRegion<2>& )));

}
  
InteractivePatchComparisonWidget::InteractivePatchComparisonWidget()
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
  this->SourcePatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2,
                                      this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);
  this->TargetPatchSlice->SetPosition(this->Image->GetLargestPossibleRegion().GetSize()[0]/2 + this->PatchSize[0],
                                      this->Image->GetLargestPossibleRegion().GetSize()[1]/2, 0);

  SetupPatches();

  PatchesMovedEventHandler();

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

  typedef itk::ImageFileReader<VectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName);
  reader->Update();

  //this->Image = reader->GetOutput();
  this->Image = VectorImageType::New();
  Helpers::DeepCopyVectorImage<VectorImageType>(reader->GetOutput(), this->Image);

  Helpers::ITKImagetoVTKImage(this->Image, this->VTKImage);

  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);

  TargetPatchInfoWidget->SetImage(this->Image);
  SourcePatchInfoWidget->SetImage(this->Image);

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
  Helpers::DeepCopy<Mask>(reader->GetOutput(), this->MaskImage);

  // For this program, we ALWAYS assume the hole to be filled is white, and the valid/source region is black.
  // This is not simply reversible because of some subtle erosion operations that are performed.
  // For this reason, we provide an "load inverted mask" action in the file menu.
  this->MaskImage->SetValidValue(0);
  this->MaskImage->SetHoleValue(255);

  this->MaskImage->Cleanup();

  Helpers::SetMaskTransparency(this->MaskImage, this->VTKMaskImage);

  this->statusBar()->showMessage("Opened mask.");

  TargetPatchInfoWidget->SetMask(this->MaskImage);
  SourcePatchInfoWidget->SetMask(this->MaskImage);
  
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
  
  PatchesMovedEventHandler();
  Refresh();
}

void InteractivePatchComparisonWidget::InitializePatch(vtkImageData* image, const unsigned char color[3])
{
  // Setup and allocate the image data
  image->SetNumberOfScalarComponents(4);
  image->SetScalarTypeToUnsignedChar();
  image->SetDimensions(this->PatchSize[0], this->PatchSize[1], 1);
  image->AllocateScalars();
  
  Helpers::BlankAndOutlineImage(image,color);
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

void InteractivePatchComparisonWidget::SetCameraPosition1()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,1,0};
  SetCameraPosition(leftToRight, bottomToTop);
}

void InteractivePatchComparisonWidget::SetCameraPosition2()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,-1,0};

  SetCameraPosition(leftToRight, bottomToTop);
}

void InteractivePatchComparisonWidget::SetCameraPosition(const double leftToRight[3], const double bottomToTop[3])
{
  this->InteractorStyle->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void InteractivePatchComparisonWidget::on_actionFlipImage_activated()
{
  if(this->Flipped)
    {
    SetCameraPosition1();
    }
  else
    {
    SetCameraPosition2();
    }
  this->Flipped = !this->Flipped;
}

void InteractivePatchComparisonWidget::slot_TargetPatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  double targetPosition[3];
  this->TargetPatchSlice->GetPosition(targetPosition);

  targetPosition[0] = patchRegion.GetIndex()[0];
  targetPosition[1] = patchRegion.GetIndex()[1];
  this->TargetPatchSlice->SetPosition(targetPosition);

  Refresh();
}

void InteractivePatchComparisonWidget::slot_SourcePatchMoved(const itk::ImageRegion<2>& patchRegion)
{
  double sourcePosition[3];
  this->SourcePatchSlice->GetPosition(sourcePosition);

  sourcePosition[0] = patchRegion.GetIndex()[0];
  sourcePosition[1] = patchRegion.GetIndex()[1];
  this->SourcePatchSlice->SetPosition(sourcePosition);

  Refresh();
}
  
void InteractivePatchComparisonWidget::PatchesMovedEventHandler()
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

  emit signal_SourcePatchMoved(sourceRegion);

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

  emit signal_TargetPatchMoved(targetRegion);

  if(!(Image->GetLargestPossibleRegion().IsInside(targetRegion) &&
    Image->GetLargestPossibleRegion().IsInside(sourceRegion)))
  {
    return;
  }

  AveragePixelDifference<SumOfAbsoluteDifferences> averagePixelDifferenceFunctor;
  float averagePixelDifference = averagePixelDifferenceFunctor(this->Image.GetPointer(),
                                                               this->MaskImage, sourceRegion, targetRegion);

  CorrelationScore correlationScoreFunctor;
  float correlationScore = correlationScoreFunctor(this->Image.GetPointer(), this->MaskImage, sourceRegion, targetRegion);

  Eigen::VectorXf sourceFeatures = ComputeNormalizedFeatures(sourceRegion);
  std::cout << "sourceFeatures " << std::endl << sourceFeatures << std::endl;
  
  Eigen::VectorXf targetFeatures = ComputeNormalizedFeatures(targetRegion);
  std::cout << "targetFeatures " << std::endl << targetFeatures << std::endl;

  float featuresDifference = Helpers::SumOfAbsoluteDifferences(sourceFeatures, targetFeatures);
  std::cout << "Features difference: " << featuresDifference << std::endl;

  // Diffusion distance of feature vectors
//   std::vector<Eigen::VectorXf> allPoints;
//   itk::Index<2> targetCenter = Helpers::GetRegionCenter(targetRegion);
//   itk::ImageRegion<2> neighborhoodRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, 4);
//   itk::ImageRegionConstIterator<Mask> neighborhoodIterator(MaskImage, neighborhoodRegion);
// 
//   while(!neighborhoodIterator.IsAtEnd())
//     {
//     itk::ImageRegion<2> nearbyRegion = Helpers::GetRegionInRadiusAroundPixel(neighborhoodIterator.GetIndex(), GetPatchRadius());
//     Eigen::VectorXf nearbyFeatures = ComputeNormalizedFeatures(nearbyRegion);
//     allPoints.push_back(nearbyFeatures);
//     ++neighborhoodIterator;
//     }
// 
//   allPoints.push_back(sourceFeatures);
// 
//   DiffusionDistance diffusionDistanceFunctor;
//   float diffusionDistance = diffusionDistanceFunctor(targetFeatures, sourceFeatures, allPoints);
//   std::cout << "diffusionDistance: " << diffusionDistance << std::endl;

  // Diffusion distance of pixel values directly
  std::vector<Eigen::VectorXf> allPoints;
  itk::Index<2> targetCenter = Helpers::GetRegionCenter(targetRegion);
  itk::ImageRegion<2> neighborhoodRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, 4);
  itk::ImageRegionConstIterator<Mask> neighborhoodIterator(MaskImage, neighborhoodRegion);

  while(!neighborhoodIterator.IsAtEnd())
    {
    itk::ImageRegion<2> nearbyRegion = Helpers::GetRegionInRadiusAroundPixel(neighborhoodIterator.GetIndex(), GetPatchRadius());
    Eigen::VectorXf nearbyFeatures = Helpers::GetRegionAsVector(Image.GetPointer(), nearbyRegion);
    allPoints.push_back(nearbyFeatures);
    ++neighborhoodIterator;
    }
  Eigen::VectorXf sourceRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), sourceRegion);
  Eigen::VectorXf targetRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), targetRegion);
  allPoints.push_back(sourceRegionVectorized);

  DiffusionDistance diffusionDistanceFunctor;
  float diffusionDistance = diffusionDistanceFunctor(sourceRegionVectorized, targetRegionVectorized, allPoints);
  std::cout << "diffusionDistance: " << diffusionDistance << std::endl;

  Refresh();

  this->lblSumAbsolutePixelDifference->setNum(averagePixelDifference);
  this->lblCorrelation->setNum(correlationScore);
}

void InteractivePatchComparisonWidget::on_btnSavePatches_clicked()
{
  TargetPatchInfoWidget->Save("target");
  SourcePatchInfoWidget->Save("source");

  itk::ImageRegion<2> sourceRegion = SourcePatchInfoWidget->GetRegion();
  itk::ImageRegion<2> targetRegion = TargetPatchInfoWidget->GetRegion();
  
  AveragePixelDifference<SumOfAbsoluteDifferences> averagePixelDifferenceFunctor;
  float averagePixelDifference = averagePixelDifferenceFunctor(this->Image.GetPointer(),
                                                               this->MaskImage, sourceRegion, targetRegion);

  std::ofstream fout("score.txt");
  fout << averagePixelDifference << std::endl;
  fout.close();
}

Eigen::VectorXf InteractivePatchComparisonWidget::ComputeNormalizedFeatures(const itk::ImageRegion<2>& region)
{
  Eigen::VectorXf features = ComputeFeatures(region);
  for(unsigned int i = 0; i < static_cast<unsigned int>(features.size()); ++i)
  {
    features[i] -= FeatureMeans[i];
    features[i] /= FeatureStandardDeviations[i];
  }

  return features;
}

Eigen::VectorXf InteractivePatchComparisonWidget::ComputeFeatures(const itk::ImageRegion<2>& region)
{
  // Compute average (N components), variance (N components)

  unsigned int numberOfImageComponents = Image->GetNumberOfComponentsPerPixel();

  Eigen::VectorXf feature(numberOfImageComponents * 2);
  
  VectorImageType::PixelType pixelAverage = Helpers::AverageInRegion(Image.GetPointer(), region);
  for(unsigned int component = 0; component < numberOfImageComponents; ++component)
    {
    feature[component] = pixelAverage[component];
    }

  VectorImageType::PixelType pixelVariance = Helpers::VarianceInRegion(Image.GetPointer(), region);
  for(unsigned int component = 0; component < numberOfImageComponents; ++component)
    {
    feature[numberOfImageComponents + component] = pixelVariance[component];
    }

  return feature;
}

void InteractivePatchComparisonWidget::ComputeFeatureMatrixStatistics()
{
  //Eigen::MatrixXf m(3*N,3*N);

  // Count valid patches
  unsigned int numberOfValidPatches = Helpers::CountValidPatches(MaskImage, GetPatchRadius());

  itk::ImageRegion<2> firstValidRegion = Helpers::FindFirstValidPatch(MaskImage, GetPatchRadius());

  Eigen::VectorXf testFeature = ComputeFeatures(firstValidRegion);
  
  unsigned int numberOfFeatures = testFeature.size();
  
  Eigen::MatrixXf featureMatrix(numberOfValidPatches, numberOfFeatures);
  
  itk::ImageRegionConstIteratorWithIndex<VectorImageType> imageIterator(Image, Image->GetLargestPossibleRegion());

  unsigned int rowCounter = 0;
  while(!imageIterator.IsAtEnd())
    {
    if(rowCounter % 10000 == 0)
    {
      std::cout << "Computed " << rowCounter << " out of " << Image->GetLargestPossibleRegion().GetNumberOfPixels()
                << " features." << std::endl;
    }

    itk::ImageRegion<2> region = Helpers::GetRegionInRadiusAroundPixel(imageIterator.GetIndex(), GetPatchRadius());
    if(MaskImage->IsValid(region))
    {
      Eigen::VectorXf feature = ComputeFeatures(region);
      for(unsigned int component = 0; component < numberOfFeatures; ++component)
        {
        featureMatrix(rowCounter, component) = feature[component];
        }

      rowCounter++;
    }

    ++imageIterator;
    }

  // Normalize feature matrix
  FeatureMeans.resize(numberOfFeatures);
  FeatureStandardDeviations.resize(numberOfFeatures);
  
  // Extract a column at a time (each column consists of the same computed value)
  for(unsigned int feature = 0; feature < numberOfFeatures; ++feature)
  {
    Eigen::VectorXf eigenFeatures = featureMatrix.col(feature);
    std::vector<float> stdFeatures = Helpers::EigenVectorToSTDVector(eigenFeatures);
    float featureAverage = Statistics::Average(stdFeatures);
    FeatureMeans[feature] = featureAverage;
    
    float featureStandardDeviation = sqrt(Statistics::Variance(stdFeatures));
    FeatureStandardDeviations[feature] = featureStandardDeviation;
  }

  std::cout << "featureMeans: " << std::endl <<  FeatureMeans << std::endl;
  std::cout << "featureStandardDeviations: " << std::endl << FeatureStandardDeviations << std::endl;
}

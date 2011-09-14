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

#include "ui_Form.h"
#include "Form.h"

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
#include "InteractorStyleImageNoLevel.h"
#include "Mask.h"
#include "Types.h"

void Form::on_actionHelp_activated()
{
  QTextEdit* help=new QTextEdit();
  
  help->setReadOnly(true);
  help->append("<h1>Interactive Patch Comparison</h1>\
  Position the two patches. <br/>\
  Their difference will be displayed.<br/> <p/>"\
  );
  help->show();
}

// Constructor
Form::Form()
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

  actionSaveResult->setIcon(saveIcon);
  this->toolBar->addAction(actionSaveResult);

  this->Flipped = false;
  
  //this->InteractorStyle = vtkSmartPointer<vtkInteractorStyleImage>::New();
  this->InteractorStyle = vtkSmartPointer<InteractorStyleImageNoLevel>::New();
  
  // Initialize and link the image display objects
  this->VTKImage = vtkSmartPointer<vtkImageData>::New();
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->ImageSliceMapper->SetInputConnection(this->VTKImage->GetProducerPort());
  this->ImageSlice->SetMapper(this->ImageSliceMapper);
  
  // Initialize and link the mask image display objects
  this->VTKMaskImage = vtkSmartPointer<vtkImageData>::New();
  this->MaskImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->MaskImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->MaskImageSliceMapper->SetInputConnection(this->VTKMaskImage->GetProducerPort());
  this->MaskImageSlice->SetMapper(this->MaskImageSliceMapper);
  
  // Add objects to the renderer
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);
  
  this->Renderer->AddViewProp(this->ImageSlice);
  this->Renderer->AddViewProp(this->MaskImageSlice);

  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->qvtkWidget->GetRenderWindow()->GetInteractor()->SetInteractorStyle(this->InteractorStyle);
  
  //this->Image = NULL;
  //this->Mask = NULL;
  
  this->Image = FloatVectorImageType::New();
  this->MaskImage = Mask::New();
  
};

void Form::on_actionQuit_activated()
{
  exit(0);
}

void Form::on_actionOpenImage_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.jpg *.jpeg *.bmp *.png *.mha);;PNG Files (*.png)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  // Set the working directory
  QFileInfo fileInfo(fileName);
  std::string workingDirectory = fileInfo.absoluteDir().absolutePath().toStdString() + "/";
  std::cout << "Working directory set to: " << workingDirectory << std::endl;
  QDir::setCurrent(QString(workingDirectory.c_str()));
    
  typedef itk::ImageFileReader<FloatVectorImageType> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();

  //this->Image = reader->GetOutput();
  Helpers::DeepCopyVectorImage<FloatVectorImageType>(reader->GetOutput(), this->Image);
  
  Helpers::ITKImagetoVTKImage(this->Image, this->VTKImage);
  
  this->Renderer->ResetCamera();
  this->qvtkWidget->GetRenderWindow()->Render();
  
  this->statusBar()->showMessage("Opened image.");
  actionOpenMask->setEnabled(true);
}


void Form::on_actionOpenMaskInverted_activated()
{
  std::cout << "on_actionOpenMaskInverted_activated()" << std::endl;
  on_actionOpenMask_activated();
  this->MaskImage->Invert();
  this->MaskImage->Cleanup();
  }

void Form::on_actionOpenMask_activated()
{
  // Get a filename to open
  QString fileName = QFileDialog::getOpenFileName(this, "Open File", ".", "Image Files (*.png *.bmp)");

  std::cout << "Got filename: " << fileName.toStdString() << std::endl;
  if(fileName.toStdString().empty())
    {
    std::cout << "Filename was empty." << std::endl;
    return;
    }

  typedef itk::ImageFileReader<Mask> ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(fileName.toStdString());
  reader->Update();
  
  if(this->Image->GetLargestPossibleRegion() != reader->GetOutput()->GetLargestPossibleRegion())
    {
    std::cerr << "Image and mask must be the same size!" << std::endl;
    return;
    }

  Helpers::DeepCopy<Mask>(reader->GetOutput(), this->MaskImage);
  
  // For this program, we ALWAYS assume the hole to be filled is white, and the valid/source region is black.
  // This is not simply reversible because of some subtle erosion operations that are performed.
  // For this reason, we provide an "load inverted mask" action in the file menu.
  this->MaskImage->SetValidValue(0);
  this->MaskImage->SetHoleValue(255);
  
  this->MaskImage->Cleanup();
  
  Helpers::SetMaskTransparency(this->MaskImage, this->VTKMaskImage);

  this->statusBar()->showMessage("Opened mask.");
}

void Form::RefreshSlot()
{
  Refresh();
}

void Form::Refresh()
{
  std::cout << "Refresh()" << std::endl;
  
  this->qvtkWidget->GetRenderWindow()->Render();
  //this->Renderer->Render();
  
}

void Form::SetCameraPosition1()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,1,0};
  SetCameraPosition(leftToRight, bottomToTop);
}

void Form::SetCameraPosition2()
{
  double leftToRight[3] = {-1,0,0};
  double bottomToTop[3] = {0,-1,0};

  SetCameraPosition(leftToRight, bottomToTop);
}

void Form::SetCameraPosition(double leftToRight[3], double bottomToTop[3])
{
  this->InteractorStyle->SetImageOrientation(leftToRight, bottomToTop);

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->qvtkWidget->GetRenderWindow()->Render();
}


void Form::on_actionFlipImage_activated()
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

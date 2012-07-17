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

#include "Layer.h"

#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>

Layer::Layer()
{
  this->ImageData = vtkSmartPointer<vtkImageData>::New();
  this->ImageData->AllocateScalars(VTK_UNSIGNED_CHAR, 4); // RGBA

  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  this->ImageSlice->GetProperty()->SetInterpolationTypeToNearest(); // Don't blur, but rather show pixels

  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  this->ImageSliceMapper->BorderOn(); // Draw the full border pixels.
  this->ImageSlice->SetMapper(this->ImageSliceMapper);

  this->ImageSlice->GetProperty()->SetInterpolationTypeToNearest(); // Don't blur pixels
  
  Setup();
}

void Layer::Setup()
{
  // This function should be called if ImageData has been replaced.
  this->ImageSliceMapper->SetInputData(this->ImageData);
}

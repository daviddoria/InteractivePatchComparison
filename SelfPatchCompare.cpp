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

#include "SelfPatchCompare.h"

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKHelpers/ITKHelpers.h"

// Custom
#include "Types.h"

// STL
#include <algorithm> // for sort()

SelfPatchCompare::SelfPatchCompare() : Image(NULL), MaskImage(NULL)
{
  this->FullyValidMask = Mask::New();
}

void SelfPatchCompare::SetImage(itk::VectorImage<float, 2>* const image)
{
  this->Image = image;

  this->FullyValidMask->SetRegions(this->Image->GetLargestPossibleRegion());
  this->FullyValidMask->Allocate();
  ITKHelpers::SetImageToConstant(this->FullyValidMask.GetPointer(), this->FullyValidMask->GetValidValue());
}

void SelfPatchCompare::SetMask(Mask* const mask)
{
  this->MaskImage = mask;
}

void SelfPatchCompare::SetTargetRegion(const itk::ImageRegion<2>& region)
{
  this->TargetRegion = region;
}

void SelfPatchCompare::ComputePatchScores()
{
  this->PatchData.clear();
  
  std::vector<itk::ImageRegion<2> > fullSourcePatches = FindFullSourcePatches();
  
  for(unsigned int i = 0; i < fullSourcePatches.size(); ++i)
    {
    //std::cout << "Comparing " << this->TargetRegion << " to " << fullSourcePatches[i] << std::endl;
    SSD<ImageType> differenceFunctor;
    float averageAbsoluteScore = differenceFunctor(this->TargetRegion, fullSourcePatches[i]);

    //std::cout << "score: " << averageAbsoluteScore << std::endl;
    PatchDataType patchData;
    patchData.first = fullSourcePatches[i];
    patchData.second = averageAbsoluteScore;
    this->PatchData.push_back(patchData);
    }
}

std::vector<SelfPatchCompare::PatchDataType> SelfPatchCompare::GetPatchData()
{
  return this->PatchData;
}

std::vector<itk::ImageRegion<2> > SelfPatchCompare::FindFullSourcePatches()
{
  // Find all full patches that are entirely Valid
  std::cout << "Finding patches for TargetRegion " << this->TargetRegion << std::endl;
  
  std::vector<itk::ImageRegion<2> > fullSourcePatches;

  itk::ImageRegionConstIterator<ImageType> imageIterator(this->Image, this->Image->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = imageIterator.GetIndex();
    itk::ImageRegion<2> region = ITKHelpers::GetRegionInRadiusAroundPixel(currentPixel, this->TargetRegion.GetSize()[0]/2);

    if(this->Image->GetLargestPossibleRegion().IsInside(region))
      {
      if(!this->MaskImage)
        {
        fullSourcePatches.push_back(region);
        }
      else if(this->MaskImage->IsValid(region))
        {
        fullSourcePatches.push_back(region);
        }
      }

    ++imageIterator;
    }

  return fullSourcePatches;
}

void SelfPatchCompare::SetProjectionMatrix(const MatrixType& projectionMatrix)
{
  this->ProjectionMatrix = projectionMatrix;
}

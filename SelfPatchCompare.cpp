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

float SelfPatchCompare::PixelSquaredDifference(const VectorType &a, const VectorType &b)
{
  float difference = 0;
  
  float diff = 0;
  
  for(unsigned int i = 0; i < this->Image->GetNumberOfComponentsPerPixel(); ++i)
    {
    diff = a[i] - b[i];
    difference += diff * diff;
    }
  return difference;
}


float SelfPatchCompare::PixelDifference(const VectorType &a, const VectorType &b)
{
  float difference = 0;
  
  for(unsigned int i = 0; i < this->Image->GetNumberOfComponentsPerPixel(); ++i)
    {
    difference += fabs(a[i] - b[i]);
    }
  return difference;
}


float SelfPatchCompare::SlowTotalAbsoluteDifference(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.
  
  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).
  itk::ImageRegionConstIterator<ImageType> sourcePatchIterator(this->Image, sourceRegion);
  itk::ImageRegionConstIterator<ImageType> targetPatchIterator(this->Image, this->TargetRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  float sumDifferences = 0;

  float difference = 0;
  
  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(this->MaskImage->IsValid(currentPixel))
      {
      //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      ImageType::PixelType sourcePixel = sourcePatchIterator.Get();
      ImageType::PixelType targetPixel = targetPatchIterator.Get();
            
      difference = PixelDifference(sourcePixel, targetPixel);
      sumDifferences +=  difference;
      }

    ++sourcePatchIterator;
    ++targetPatchIterator;
    ++maskIterator;
    } // end while iterate over sourcePatch

  return sumDifferences;
}


float SelfPatchCompare::SlowTotalSquaredDifference(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.

  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).

  typedef itk::VectorImage<float, 2> FloatVectorImageType;
  itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, sourceRegion);
  itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, this->TargetRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  float sumSquaredDifferences = 0;

  float squaredDifference = 0;

  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(this->MaskImage->IsValid(currentPixel))
      {
      //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
      FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();

      squaredDifference = PixelSquaredDifference(sourcePixel, targetPixel);

      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << "Difference: " << difference << " squaredDifference: " << squaredDifference << std::endl;

      sumSquaredDifferences +=  squaredDifference;
      }

    ++sourcePatchIterator;
    ++targetPatchIterator;
    ++maskIterator;
    } // end while iterate over sourcePatch

  return sumSquaredDifferences;
}


float SelfPatchCompare::SlowAverageAbsoluteDifference(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.

  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).

  if(!this->MaskImage)
    {
    this->MaskImage = this->FullyValidMask.GetPointer();
    }

  typedef itk::VectorImage<float, 2> FloatVectorImageType;
  itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, sourceRegion);
  itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, this->TargetRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  float sumDifferences = 0;

  float difference = 0;
  unsigned int numberOfPixelsCompared = 0;
  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(this->MaskImage->IsValid(currentPixel))
      {
      //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
      FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();

      difference = PixelDifference(sourcePixel, targetPixel);
      sumDifferences +=  difference;

      numberOfPixelsCompared++;
      }

    ++sourcePatchIterator;
    ++targetPatchIterator;
    ++maskIterator;
    } // end while iterate over sourcePatch

  float averageDifferences = 0.0f;
  if(numberOfPixelsCompared == 0)
  {
    averageDifferences = 0.0f;
  }
  else
  {
    averageDifferences= sumDifferences / static_cast<float>(numberOfPixelsCompared);
  }
  return averageDifferences;
}


float SelfPatchCompare::SlowAverageSquaredDifference(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.

  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).
  typedef itk::VectorImage<float, 2> FloatVectorImageType;
  itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, sourceRegion);
  itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, this->TargetRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  float sumSquaredDifferences = 0;

  float squaredDifference = 0;
  unsigned int numberOfPixelsCompared = 0;
  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(!this->MaskImage || this->MaskImage->IsValid(currentPixel))
      {
      //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
      FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();

      squaredDifference = PixelSquaredDifference(sourcePixel, targetPixel);

      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << "Difference: " << difference << " squaredDifference: " << squaredDifference << std::endl;

      sumSquaredDifferences +=  squaredDifference;
      numberOfPixelsCompared++;
      }

    ++sourcePatchIterator;
    ++targetPatchIterator;
    ++maskIterator;
    } // end while iterate over sourcePatch

  float averageSquaredDifferences = sumSquaredDifferences / static_cast<float>(numberOfPixelsCompared);
  return averageSquaredDifferences;
}

void SelfPatchCompare::ComputePatchScores()
{
  this->PatchData.clear();
  
  std::vector<itk::ImageRegion<2> > fullSourcePatches = FindFullSourcePatches();
  
  for(unsigned int i = 0; i < fullSourcePatches.size(); ++i)
    {
    //std::cout << "Comparing " << this->TargetRegion << " to " << fullSourcePatches[i] << std::endl;
    float averageAbsoluteScore = SlowAverageAbsoluteDifference(fullSourcePatches[i]);
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

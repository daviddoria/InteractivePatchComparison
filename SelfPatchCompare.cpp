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

#include "Helpers.h"
#include "Types.h"

SelfPatchCompare::SelfPatchCompare(const unsigned int components)
{
  this->NumberOfComponentsPerPixel = components;
}

bool SelfPatchCompare::IsReady()
{
  if(this->Image && this->MaskImage)
    {
    return true;
    }
  else
    {
    return false;
    }
}

void SelfPatchCompare::SetImage(FloatVectorImageType::Pointer image)
{
  this->Image = image;
}

void SelfPatchCompare::SetMask(Mask::Pointer mask)
{
  this->MaskImage = mask;
}

void SelfPatchCompare::SetTargetRegion(const itk::ImageRegion<2>& region)
{
  this->TargetRegion = region;
}

void SelfPatchCompare::SetSourceRegion(const itk::ImageRegion<2>& region)
{
  this->SourceRegion = region;
}

#if 0
void SelfPatchCompare::ComputeOffsets()
{
  try
  {
    // Iterate over the target region of the mask. Add the linear offset of valid pixels to the offsets to be used later in the comparison.
    itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    while(!maskIterator.IsAtEnd())
      {
      if(this->MaskImage->IsValid(maskIterator.GetIndex()))
	{
	FloatVectorImageType::OffsetValueType offset = this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
	//std::cout << "Using offset: " << offset << std::endl;
	this->ValidOffsets.push_back(offset); // We have to multiply the linear offset by the number of components per pixel for the VectorImage type
	}

      ++maskIterator;
      }
    //std::cout << "Number of valid offsets: " << this->ValidOffsets.size() << std::endl;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in ComputeOffsets!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif

float SelfPatchCompare::SlowDifference()
{
  // This function assumes that all pixels in the source region are unmasked.
  
  // This method uses 3 iterators - one for the mask, and one for each image patch.
  // The entire mask is traversed looking for valid pixels, and then comparing the image pixels.
  // This is very inefficient because, since the target region stays constant for many thousands of patch
  // comparisons, the mask need only be traversed once. This method is performed by ComputeOffsets()
  // and PatchDifference*(). This function is only here for comparison purposes (to ensure the result of the other functions
  // is correct).

  itk::ImageRegionConstIterator<FloatVectorImageType> sourcePatchIterator(this->Image, this->SourceRegion);
  itk::ImageRegionConstIterator<FloatVectorImageType> targetPatchIterator(this->Image, this->TargetRegion);
  itk::ImageRegionConstIterator<Mask> maskIterator(this->MaskImage, this->TargetRegion);

  float sum = 0;
  unsigned int validPixelCounter = 0;
  
  while(!sourcePatchIterator.IsAtEnd())
    {
    itk::Index<2> currentPixel = maskIterator.GetIndex();
    if(this->MaskImage->IsValid(currentPixel))
      {
      //std::cout << "Offset from iterator: " << this->Image->ComputeOffset(maskIterator.GetIndex()) * componentsPerPixel;
      FloatVectorImageType::PixelType sourcePixel = sourcePatchIterator.Get();
      FloatVectorImageType::PixelType targetPixel = targetPatchIterator.Get();
      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
      //float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
      float difference = PixelDifference(sourcePixel, targetPixel);
      sum +=  difference;
      validPixelCounter++;
      }

    ++sourcePatchIterator;
    ++targetPatchIterator;
    ++maskIterator;
    } // end while iterate over sourcePatch

  //std::cout << "totalDifference: " << sum << std::endl;
  //std::cout << "Valid pixels: " << validPixelCounter << std::endl;

  if(validPixelCounter == 0)
    {
    return 0;
    }
  float averageDifference = sum/static_cast<float>(validPixelCounter);
  return averageDifference;

}

#if 0
float SelfPatchCompare::PatchDifferenceManual(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->TargetRegion.GetIndex())
                                    - this->Image->ComputeOffset(sourceRegion.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      /*
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }
      //std::cout << "Source pixel: " << sourcePixel << " target pixel: " << targetPixel << std::endl;
      float difference = Helpers::PixelSquaredDifference(sourcePixel, targetPixel);
      */
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
      
	difference += (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]) * 
		      (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
        }
      //std::cout << "difference: " << difference << std::endl;
      totalDifference += difference;
      }
    //std::cout << "totalDifference: " << totalDifference << std::endl;
    /*
    if(validPixelCounter == 0)
      {
      std::cerr << "Zero valid pixels in PatchDifference." << std::endl;
      std::cerr << "Source region: " << sourceRegion << std::endl;
      std::cerr << "Target region: " << targetRegion << std::endl;
      std::cerr << "New source region: " << newSourceRegion << std::endl;
      std::cerr << "New target region: " << newTargetRegion << std::endl;
      exit(-1);
      }
    */
    totalDifference *= totalDifference;
    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif

#if 0
float SelfPatchCompare::PatchDifferenceExternal(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->TargetRegion.GetIndex())
                                    - this->Image->ComputeOffset(sourceRegion.GetIndex())) * componentsPerPixel;

    float difference = 0;
    
    FloatVectorImageType::PixelType sourcePixel;
    sourcePixel.SetSize(componentsPerPixel);
    
    FloatVectorImageType::PixelType targetPixel;
    targetPixel.SetSize(componentsPerPixel);
    
    FloatVectorImageType::PixelType differencePixel;
    differencePixel.SetSize(componentsPerPixel);
    
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	sourcePixel[i] = buffptr[this->ValidOffsets[pixelId] + i];
        targetPixel[i] = buffptr[this->ValidOffsets[pixelId] - offsetDifference + i];
        }
    
      
      difference = PixelDifference(sourcePixel, targetPixel); // This call seems to make it very slow?
      //difference = NonVirtualPixelDifference(sourcePixel, targetPixel); // This call seems to make it very slow?
      //difference = (sourcePixel-targetPixel).GetSquaredNorm(); // horribly slow
      
      //differencePixel = sourcePixel-targetPixel;
      //difference = differencePixel.GetSquaredNorm();
      
//       difference = 0;
//       for(unsigned int i = 0; i < componentsPerPixel; ++i)
//         {
// 	difference += (sourcePixel[i] - targetPixel[i]) * 
// 		      (sourcePixel[i] - targetPixel[i]);
// 	}

      totalDifference += difference;
      }

    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif

#if 0
float SelfPatchCompare::PatchDifferenceBoundary(const itk::ImageRegion<2>& sourceRegion)
{
  // This function assumes that all pixels in the source region are unmasked.
  try
  {
    assert(this->Image->GetLargestPossibleRegion().IsInside(sourceRegion));

    itk::ImageRegion<2> newSourceRegion = sourceRegion;

    // Move the source region to the target region. We move this way because we want to iterate over the mask in the target region.
    itk::Offset<2> sourceTargetOffset = this->TargetRegion.GetIndex() - sourceRegion.GetIndex();

    newSourceRegion.SetIndex(sourceRegion.GetIndex() + sourceTargetOffset);

    // Force the source region to be entirely inside the image
    newSourceRegion.Crop(this->Image->GetLargestPossibleRegion());

    // Move the source region back to its original position
    newSourceRegion.SetIndex(newSourceRegion.GetIndex() - sourceTargetOffset);

    //std::cout << "New source region: " << newSourceRegion << std::endl;
    //std::cout << "New target region: " << newTargetRegion << std::endl;
    
    float totalDifference = 0;

    unsigned int componentsPerPixel = this->Image->GetNumberOfComponentsPerPixel();
    
    FloatVectorImageType::InternalPixelType *buffptr = this->Image->GetBufferPointer();
    unsigned int offsetDifference = (this->Image->ComputeOffset(this->TargetRegion.GetIndex())
                                    - this->Image->ComputeOffset(newSourceRegion.GetIndex())) * componentsPerPixel;

    float difference = 0;
    for(unsigned int pixelId = 0; pixelId < this->ValidOffsets.size(); ++pixelId)
      {
      difference = 0;
      for(unsigned int i = 0; i < componentsPerPixel; ++i)
        {
	//std::cout << "component " << i << ": " << buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i] << std::endl;
        //difference += fabs(buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
	difference += (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]) * 
		      (buffptr[this->ValidOffsets[pixelId] + i] - buffptr[this->ValidOffsets[pixelId] - offsetDifference + i]);
        }
      totalDifference += difference;
      }

    float averageDifference = totalDifference/static_cast<float>(this->ValidOffsets.size());
    return averageDifference;
  } //end try
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in PatchDifference!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif

#if 0
unsigned int SelfPatchCompare::FindBestPatch()
{
  try
  {
    float minDistance = std::numeric_limits<float>::infinity();
    unsigned int bestMatchId = 0;
    if(!this->Image->GetLargestPossibleRegion().IsInside(this->TargetRegion))
      {  
      // Force the target region to be entirely inside the image
      this->TargetRegion.Crop(this->Image->GetLargestPossibleRegion());
    
      ComputeOffsets();
    
      for(unsigned int i = 0; i < this->SourceRegions.size(); ++i)
	{
	float distance = PatchDifferenceBoundary(this->SourceRegions[i]);
	//std::cout << "Patch " << i << " distance " << distance << std::endl;
	if(distance < minDistance)
	  {
	  minDistance = distance;
	  bestMatchId = i;
	  }
	}
      }
    else // The target patch is entirely inside the image
      {
      ComputeOffsets();
      for(unsigned int i = 0; i < this->SourceRegions.size(); ++i)
	{
	float distance = PatchDifferenceExternal(this->SourceRegions[i]);
	//float distance = PatchDifferenceManual(this->SourceRegions[i]);
	//std::cout << "Patch " << i << " distance " << distance << std::endl;
	if(distance < minDistance)
	  {
	  minDistance = distance;
	  bestMatchId = i;
	  }
	}
      }

    return bestMatchId;
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in FindBestPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}
#endif

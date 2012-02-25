#ifndef AverageScore_HPP
#define AverageScore_HPP

// Custom
#include "Helpers.h"
/**

 */
struct AverageScore
{
  template <typename TImage>
  float operator()(const TImage* const image, const Mask* const mask,
                   const itk::ImageRegion<2>& sourceRegion, const itk::ImageRegion<2>& targetRegion)
  {
    std::vector<itk::Offset<2> > validOffsets = mask->GetValidOffsetsInRegion(targetRegion);

    if(validOffsets.size() == 0)
    {
      return 0;
    }
    else
    {
      std::cout << "AverageScore with " << validOffsets.size() << " pixels." << std::endl;
    }

    std::vector<itk::Index<2> > validIndices = Helpers::OffsetsToIndices(validOffsets, targetRegion.GetIndex());

    Helpers::AverageFunctor averageFunctor;

    /////////// Target region //////////
    std::vector<typename TImage::PixelType> validPixelsTargetRegion = Helpers::GetPixelValues(image, validIndices);
    std::cout << "pixel 0: " << static_cast<int>(validPixelsTargetRegion[0][0]) << std::endl;

    typename TypeTraits<typename TImage::PixelType>::LargerType targetAverage = averageFunctor(validPixelsTargetRegion);

    std::cout << "targetAverage: " << targetAverage << std::endl;

    float variance = 0.0f;
    for(unsigned int i = 0; i < targetAverage.GetSize(); ++i)
    {
      variance += targetAverage[i];
    }
    return variance;
  }

};

#endif

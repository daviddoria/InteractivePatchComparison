#include "Mask.h"


std::vector<itk::Offset<2> > Mask::GetValidOffsetsInRegion(itk::ImageRegion<2> region) const
{
  region.Crop(this->GetLargestPossibleRegion());

  std::vector<itk::Offset<2> > validOffsets;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsValid(iterator.GetIndex()))
      {
      validOffsets.push_back(iterator.GetIndex() - region.GetIndex());
      }

    ++iterator;
    }
  return validOffsets;
}

unsigned int Mask::CountMaskedPixels(const itk::ImageRegion<2>& region)
{
  unsigned int numberOfMaskedPixels = 0;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsHole(iterator.GetIndex()))
      {
      numberOfMaskedPixels++;
      }

    ++iterator;
    }
  return numberOfMaskedPixels;
}

unsigned int Mask::CountValidPixels(const itk::ImageRegion<2>& region)
{
  unsigned int numberOfValidPixels = 0;

  itk::ImageRegionConstIterator<Mask> iterator(this, region);

  while(!iterator.IsAtEnd())
    {
    if(this->IsValid(iterator.GetIndex()))
      {
      numberOfValidPixels++;
      }

    ++iterator;
    }
  return numberOfValidPixels;
}

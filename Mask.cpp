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
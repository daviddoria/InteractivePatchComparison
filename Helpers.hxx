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

#include "itkCastImageFilter.h"

// STL
#include <iomanip> // for setfill()

// Qt
#include <QColor>

// VTK
#include <vtkImageData.h>

// Custom
#include "Mask.h"
#include "TypeTraits.h"
#include "Statistics.h"

namespace Helpers
{

template<typename TImage>
void DeepCopyInRegion(const TImage* input, const itk::ImageRegion<2>& region, TImage* output)
{
  // This function assumes that the size of input and output are the same.

  itk::ImageRegionConstIterator<TImage> inputIterator(input, region);
  itk::ImageRegionIterator<TImage> outputIterator(output, region);

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

// Index functions
template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T&>::type index(T& t, size_t)
{
  return t;
}

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T>::type index(const T& t, size_t)
{
  return t;
}

template<typename T>
typename std::enable_if<!std::is_fundamental<T>::value, typename T::value_type&>::type index(T& v, size_t i)
{
  return v[i];
}

template<typename T>
typename std::enable_if<!std::is_fundamental<T>::value, typename T::value_type>::type index(const T& v, size_t i)
{
  return v[i];
}

// Length functions
template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, unsigned int>::type length(const T& t)
{
  return 1;
}

template<typename T>
unsigned int length(const std::vector<T>& v)
{
  return v.size();
}

template<typename T>
unsigned int length(const itk::VariableLengthVector<T>& v)
{
  return v.GetSize();
}

template<typename T>
T& index(itk::VariableLengthVector<T>& v, size_t i)
{
  return v[i];
}

template<typename T>
T index(const itk::VariableLengthVector<T>& v, size_t i)
{
  return v[i];
}

template<typename T>
void SetObjectToZero(T& object)
{
  for(unsigned int i = 0; i < Helpers::length(object); ++i)
    {
    Helpers::index(object, i) = 0;
    }
}

template<typename T>
itk::Index<2> CreateIndex(const T& v)
{
  itk::Index<2> index = {{v[0], v[1]}};
  return index;
}

template<typename TImage>
std::vector<typename TImage::PixelType> GetPixelValues(const TImage* const image, const std::vector<itk::Index<2> >& indices)
{
  std::vector<typename TImage::PixelType> values;
  for(std::vector<itk::Index<2> >::const_iterator iter = indices.begin(); iter != indices.end(); ++iter)
  {
    values.push_back(image->GetPixel(*iter));
  }

  return values;
}

struct AverageFunctor
{
  template <typename TPixel>
  typename TypeTraits<TPixel>::LargerType operator()(const std::vector<TPixel>& pixels) const
  {
    typename TypeTraits<TPixel>::LargerType allChannelsAverage = Statistics::Average(pixels);
    // std::cout << "AverageFunctor() : allChannelsAverage " << allChannelsAverage << std::endl;

    return allChannelsAverage;
  }
};

struct VarianceFunctor
{
  template <typename TPixel>
  typename TypeTraits<TPixel>::LargerType operator()(const std::vector<TPixel>& pixels) const
  {
    assert(pixels.size() > 0);
//     if(pixels.size() <= 0)
//     {
//       throw std::runtime_error("Must have more than 0 items to use VarianceFunctor!");
//     }
    typename TypeTraits<TPixel>::LargerType allChannelsVariance = Statistics::Variance(pixels);
    return allChannelsVariance;
  }
};

template<typename TImage>
void ExtractRegion(const TImage* const image, const itk::ImageRegion<2>& region,
                   TImage* const output)
{
  typedef itk::RegionOfInterestImageFilter<TImage, TImage> ExtractFilterType;

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(region);
  extractFilter->SetInput(image);
  extractFilter->Update();

  DeepCopy(extractFilter->GetOutput(), output);
}

/** Copy the input to the output*/
template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
}

/** Copy the input to the output*/
template<typename TImage>
void DeepCopyVectorImage(const TImage* const input, TImage* const output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->SetNumberOfComponentsPerPixel(input->GetNumberOfComponentsPerPixel());
  output->Allocate();

  itk::ImageRegionConstIterator<TImage> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<TImage> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    outputIterator.Set(inputIterator.Get());
    ++inputIterator;
    ++outputIterator;
    }
    
}


template <typename TImage>
QImage GetQImageMasked(const TImage* image, const Mask* const mask, const itk::ImageRegion<2>& region, const QColor& color)
{
  // Call the stronger function with the same 'region' for both the image and mask.
  return GetQImageMasked(image, region, mask, region, color);
}

template <typename TImage>
QImage GetQImageMasked(const TImage* image, const itk::ImageRegion<2>& imageRegion, const Mask* const mask,
                       const itk::ImageRegion<2>& maskRegion, const QColor& holeColor)
{
  assert(imageRegion.GetSize() == maskRegion.GetSize());

  QImage qimage(imageRegion.GetSize()[0], imageRegion.GetSize()[1], QImage::Format_RGB888);

  typedef itk::RegionOfInterestImageFilter< TImage, TImage > RegionOfInterestImageFilterType;
  typename RegionOfInterestImageFilterType::Pointer regionOfInterestImageFilter = RegionOfInterestImageFilterType::New();
  regionOfInterestImageFilter->SetRegionOfInterest(imageRegion);
  regionOfInterestImageFilter->SetInput(image);
  regionOfInterestImageFilter->Update();

  typedef itk::RegionOfInterestImageFilter< Mask, Mask> RegionOfInterestMaskFilterType;
  typename RegionOfInterestMaskFilterType::Pointer regionOfInterestMaskFilter = RegionOfInterestMaskFilterType::New();
  regionOfInterestMaskFilter->SetRegionOfInterest(maskRegion);
  regionOfInterestMaskFilter->SetInput(mask);
  regionOfInterestMaskFilter->Update();

  itk::ImageRegionIterator<TImage> imageIterator(regionOfInterestImageFilter->GetOutput(),
                                                 regionOfInterestImageFilter->GetOutput()->GetLargestPossibleRegion());

  unsigned int numberOfHolePixels = 0;
  while(!imageIterator.IsAtEnd())
    {
    typename TImage::PixelType pixel = imageIterator.Get();

    itk::Index<2> index = imageIterator.GetIndex();

    if(regionOfInterestMaskFilter->GetOutput()->IsHole(index))
      {
      qimage.setPixel(index[0], index[1], holeColor.rgb());
      numberOfHolePixels++;
      }
    else
      {
      QColor pixelColor(static_cast<int>(pixel[0]), static_cast<int>(pixel[1]), static_cast<int>(pixel[2]));
      qimage.setPixel(index[0], index[1], pixelColor.rgb());
      }

    ++imageIterator;
    }

  // std::cout << "There were " << numberOfHolePixels << " hole pixels." << std::endl;

  //return qimage; // The actual image region
  return qimage.mirrored(false, true); // The flipped image region
}

template<typename T>
void ReplaceValue(const T* const image, const typename T::PixelType queryValue, const typename T::PixelType replacementValue)
{
  // This function replaces all pixels in 'image' equal to 'queryValue' with 'replacementValue'
  itk::ImageRegionIterator<T> imageIterator(image, image->GetLargestPossibleRegion());
  imageIterator.GoToBegin();
  while(!imageIterator.IsAtEnd())
    {
    if(imageIterator.Get() == queryValue)
      {
      imageIterator.Set(replacementValue);
      }
    ++imageIterator;
    }
}

template<typename T>
void WriteImage(const T* const image, std::string filename)
{
  // This is a convenience function so that images can be written in 1 line instead of 4.
  typename itk::ImageFileWriter<T>::Pointer writer = itk::ImageFileWriter<T>::New();
  writer->SetFileName(filename);
  writer->SetInput(image);
  writer->Update();
}


template<typename T>
void WriteRGBImage(const T* const input, const std::string filename)
{
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> RGBImageType;

  RGBImageType::Pointer output = RGBImageType::New();
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();

  itk::ImageRegionConstIterator<T> inputIterator(input, input->GetLargestPossibleRegion());
  itk::ImageRegionIterator<RGBImageType> outputIterator(output, output->GetLargestPossibleRegion());

  while(!inputIterator.IsAtEnd())
    {
    itk::CovariantVector<unsigned char, 3> pixel;
    for(unsigned int i = 0; i < 3; ++i)
      {
      pixel[i] = inputIterator.Get()[i];
      }
    outputIterator.Set(pixel);
    ++inputIterator;
    ++outputIterator;
    }

  typename itk::ImageFileWriter<RGBImageType>::Pointer writer = itk::ImageFileWriter<RGBImageType>::New();
  writer->SetFileName(filename);
  writer->SetInput(output);
  writer->Update();

}

template <class T>
void CreateBlankPatch(T* const patch, const unsigned int radius)
{
  CreateConstantPatch<T>(patch, itk::NumericTraits< typename T::PixelType >::Zero, radius);
}

template <class T>
void CreateConstantPatch(T* const patch, typename T::PixelType value, unsigned int radius)
{
  try
  {
  typename T::IndexType start;
  start.Fill(0);

  typename T::SizeType size;
  size.Fill(radius*2 + 1);

  typename T::RegionType region(start, size);

  patch->SetRegions(region);
  patch->Allocate();

  itk::ImageRegionIterator<T> imageIterator(patch, patch->GetLargestPossibleRegion());

  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CreateConstantPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
float MaxValue(const T* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMaximum();
}

template <class T>
float MaxValueLocation(const T* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMaximum();
}

template <class T>
float MinValue(const T* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetMinimum();
}

template <class T>
itk::Index<2> MinValueLocation(const T* const image)
{
  typedef typename itk::MinimumMaximumImageCalculator<T>
          ImageCalculatorFilterType;

  typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
          = ImageCalculatorFilterType::New ();
  imageCalculatorFilter->SetImage(image);
  imageCalculatorFilter->Compute();

  return imageCalculatorFilter->GetIndexOfMinimum();
}

template <class T>
void CopyPatchIntoImage(const T* const patch, T* const image, Mask* const mask, const itk::Index<2> position)
{
  try
  {
  // This function copies 'patch' into 'image' centered at 'position' only where the 'mask' is non-zero

  // 'Mask' must be the same size as 'image'
  if(mask->GetLargestPossibleRegion().GetSize() != image->GetLargestPossibleRegion().GetSize())
    {
    std::cerr << "mask and image must be the same size!" << std::endl;
    exit(-1);
    }

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  itk::ImageRegion<2> region = GetRegionInRadiusAroundPixel(position, patch->GetLargestPossibleRegion().GetSize()[0]/2);

  itk::ImageRegionConstIterator<T> patchIterator(patch,patch->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<Mask> maskIterator(mask,region);
  itk::ImageRegionIterator<T> imageIterator(image, region);

  while(!patchIterator.IsAtEnd())
    {
    if(mask->IsHole(maskIterator.GetIndex())) // we are in the target region
      {
      imageIterator.Set(patchIterator.Get());
      }
    ++imageIterator;
    ++maskIterator;
    ++patchIterator;
    }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatchIntoImage(patch, image, mask, position)!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}



template <class T>
void CopySelfPatchIntoValidRegion(T* const image, const Mask* const mask,
                                  const itk::ImageRegion<2> sourceRegion, const itk::ImageRegion<2> destinationRegion)
{
  try
  {
    assert(image->GetLargestPossibleRegion().IsInside(sourceRegion));
    assert(mask->IsValid(sourceRegion));
    assert(sourceRegion.GetSize() == destinationRegion.GetSize());

    // Move the source region to the desintation region
    itk::Offset<2> offset = destinationRegion.GetIndex() - sourceRegion.GetIndex();
    sourceRegion.SetIndex(sourceRegion.GetIndex() + offset);

    // Make the destination be entirely inside the image
    destinationRegion.Crop(image->GetLargestPossibleRegion());
    sourceRegion.Crop(image->GetLargestPossibleRegion());

    // Move the source region back
    sourceRegion.SetIndex(sourceRegion.GetIndex() - offset);

    itk::ImageRegionConstIterator<T> sourceIterator(image, sourceRegion);
    itk::ImageRegionIterator<T> destinationIterator(image, destinationRegion);
    itk::ImageRegionConstIterator<Mask> maskIterator(mask, destinationRegion);

    while(!sourceIterator.IsAtEnd())
      {
      if(mask->IsHole(maskIterator.GetIndex())) // we are in the target region
	{
	destinationIterator.Set(sourceIterator.Get());
	}
      ++sourceIterator;
      ++maskIterator;
      ++destinationIterator;
      }

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopySelfPatchIntoValidRegion!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
void CopyPatchIntoImage(const T* const patch, T* const image, const itk::Index<2> position)
{
  try
  {
  // This function copies 'patch' into 'image' centered at 'position'.

  // The PasteFilter expects the lower left corner of the destination position, but we have passed the center pixel.
  position[0] -= patch->GetLargestPossibleRegion().GetSize()[0]/2;
  position[1] -= patch->GetLargestPossibleRegion().GetSize()[1]/2;

  typedef itk::PasteImageFilter <T, T> PasteImageFilterType;

  typename PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New();
  pasteFilter->SetInput(0, image);
  pasteFilter->SetInput(1, patch);
  pasteFilter->SetSourceRegion(patch->GetLargestPossibleRegion());
  pasteFilter->SetDestinationIndex(position);
  pasteFilter->InPlaceOn();
  pasteFilter->Update();

  image->Graft(pasteFilter->GetOutput());

  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatchIntoImage(patch, image, position)!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}

template <class T>
void CopyPatch(const T* const sourceImage, T* const targetImage,
               const itk::Index<2> sourcePosition, const itk::Index<2> targetPosition, const unsigned int radius)
{
  try
  {
  // Copy a patch of radius 'radius' centered at 'sourcePosition' from 'sourceImage' to 'targetImage' centered at 'targetPosition'
  typedef itk::RegionOfInterestImageFilter<T,T> ExtractFilterType;

  typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
  extractFilter->SetRegionOfInterest(GetRegionInRadiusAroundPixel(sourcePosition, radius));
  extractFilter->SetInput(sourceImage);
  extractFilter->Update();

  CopyPatchIntoImage<T>(extractFilter->GetOutput(), targetImage, targetPosition);
  }
  catch( itk::ExceptionObject & err )
  {
    std::cerr << "ExceptionObject caught in CopyPatch!" << std::endl;
    std::cerr << err << std::endl;
    exit(-1);
  }
}


template <class T>
void WriteScaledScalarImage(const T* const image, const std::string filename)
{
  if(T::PixelType::Dimension > 1)
    {
    std::cerr << "Cannot write scaled scalar image with vector image input!" << std::endl;
    return;
    }
  typedef itk::RescaleIntensityImageFilter<T, UnsignedCharScalarImageType> RescaleFilterType; // expected ';' before rescaleFilter

  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetInput(image);
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->Update();

  typedef itk::ImageFileWriter<UnsignedCharScalarImageType> WriterType;
  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(filename);
  writer->SetInput(rescaleFilter->GetOutput());
  writer->Update();
}


template <typename TImage>
void ColorToGrayscale(const TImage* const colorImage, UnsignedCharScalarImageType* const grayscaleImage)
{
  grayscaleImage->SetRegions(colorImage->GetLargestPossibleRegion());
  grayscaleImage->Allocate();

  itk::ImageRegionConstIterator<TImage> colorImageIterator(colorImage, colorImage->GetLargestPossibleRegion());
  itk::ImageRegionIterator<UnsignedCharScalarImageType> grayscaleImageIterator(grayscaleImage, grayscaleImage->GetLargestPossibleRegion());

  typename TImage::PixelType largestPixel;
  largestPixel.Fill(255);

  float largestNorm = largestPixel.GetNorm();

  while(!colorImageIterator.IsAtEnd())
    {
    grayscaleImageIterator.Set(colorImageIterator.Get().GetNorm()*(255./largestNorm));

    ++colorImageIterator;
    ++grayscaleImageIterator;
    }
}

template <typename TImageType>
void DebugWriteSequentialImage(const TImageType* const image, const std::string& filePrefix, const unsigned int iteration)
{
  std::stringstream padded;
  padded << "Debug/" << filePrefix << "_" << std::setfill('0') << std::setw(4) << iteration << ".mha";
  Helpers::WriteImage<TImageType>(image, padded.str());
}

template <typename TImageType>
void DebugWriteImageConditional(typename TImageType::Pointer image, const std::string& fileName, const bool condition)
{
  if(condition)
    {
    WriteImage<TImageType>(image, fileName);
    }
}


template <typename TImage>
void ITKScalarImageToScaledVTKImage(const TImage* const image, vtkImageData* outputImage)
{
  //std::cout << "ITKScalarImagetoVTKImage()" << std::endl;
  
  // Rescale and cast for display
  typedef itk::RescaleIntensityImageFilter<TImage, UnsignedCharScalarImageType > RescaleFilterType;
  typename RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);
  rescaleFilter->SetInput(image);
  rescaleFilter->Update();

  // Setup and allocate the VTK image
  outputImage->SetNumberOfScalarComponents(1);
  outputImage->SetScalarTypeToUnsignedChar();
  outputImage->SetDimensions(image->GetLargestPossibleRegion().GetSize()[0],
                             image->GetLargestPossibleRegion().GetSize()[1],
                             1);

  outputImage->AllocateScalars();

  // Copy all of the scaled magnitudes to the output image
  itk::ImageRegionConstIteratorWithIndex<UnsignedCharScalarImageType> imageIterator(rescaleFilter->GetOutput(), rescaleFilter->GetOutput()->GetLargestPossibleRegion());
  imageIterator.GoToBegin();

  while(!imageIterator.IsAtEnd())
    {
    unsigned char* pixel = static_cast<unsigned char*>(outputImage->GetScalarPointer(imageIterator.GetIndex()[0],
                                                                                     imageIterator.GetIndex()[1],0));
    pixel[0] = imageIterator.Get();

    ++imageIterator;
    }
    
  outputImage->Modified();
}


template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageInRegion(const TImage* const image,
                                                                            const itk::ImageRegion<2>& region)
{
  typename itk::ImageRegionConstIterator<TImage> imageIterator(image, region);
  std::vector<typename TImage::PixelType> pixels;
  while(!imageIterator.IsAtEnd())
    {
    pixels.push_back(imageIterator.Get());
    ++imageIterator;
    }

  using Statistics::Average;
  //using ITKHelpers::Average;
  return Average(pixels);
}

template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceInRegion(const TImage* const image,
                                                                             const itk::ImageRegion<2>& region)
{
  typename itk::ImageRegionConstIterator<TImage> imageIterator(image, region);
  std::vector<typename TImage::PixelType> pixels;
  while(!imageIterator.IsAtEnd())
    {
    pixels.push_back(imageIterator.Get());
    ++imageIterator;
    }

  return Statistics::Variance(pixels);
}


/** Compute the average of all unmasked pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageInRegionMasked(const TImage* const image,
                                                                                  const Mask* const mask,
                                                                            const itk::ImageRegion<2>& region)
{
  typename itk::ImageRegionConstIteratorWithIndex<Mask> maskIterator(mask, region);
  std::vector<typename TImage::PixelType> pixels;
  while(!maskIterator.IsAtEnd())
    {
    if(mask->IsValid(maskIterator.GetIndex()))
      {
      pixels.push_back(image->GetPixel(maskIterator.GetIndex()));
      }
    ++maskIterator;
    }

  using Statistics::Average;
  //using ITKHelpers::Average;
  return Average(pixels);
}

/** Compute the average of all unmasked pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceInRegionMasked(const TImage* const image,
                                                                                   const Mask* const mask,
                                                                             const itk::ImageRegion<2>& region)
{
  typename itk::ImageRegionConstIterator<Mask> maskIterator(mask, region);
  std::vector<typename TImage::PixelType> pixels;
  while(!maskIterator.IsAtEnd())
    {
    if(mask->IsValid(maskIterator.GetIndex()))
      {
      pixels.push_back(image->GetPixel(maskIterator.GetIndex()));
      }
    ++maskIterator;
    }

  return Statistics::Variance(pixels);
}

template<typename TImage>
void SetImageToConstant(TImage* const image, const typename TImage::PixelType& value)
{
  typename itk::ImageRegionIterator<TImage> imageIterator(image, image->GetLargestPossibleRegion());
  
  while(!imageIterator.IsAtEnd())
    {
    imageIterator.Set(value);
    ++imageIterator;
    }
}

}// end namespace

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

#ifndef HELPERS_H
#define HELPERS_H

// Custom
#include "Mask.h"
#include "Types.h"
#include "TypeTraits.h"

// ITK
#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileWriter.h"
#include "itkNeighborhoodIterator.h"
#include "itkPasteImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

// VTK
class vtkImageData;
class vtkImageSlice;

// Qt
#include <QImage>
#include <QGraphicsView>

// Eigen
#include <Eigen/Dense>

namespace Helpers
{

std::vector<float> EigenVectorToSTDVector(const Eigen::VectorXd& vec);

float SumOfAbsoluteDifferences(const Eigen::VectorXd& a, const Eigen::VectorXd& b);
  
unsigned int CountValidPatches(const Mask* const mask, const unsigned int patchRadius);

itk::ImageRegion<2> FindFirstValidPatch(const Mask* const mask, const unsigned int patchRadius);

QImage FitToGraphicsView(const QImage qimage, const QGraphicsView* gfx);

template<typename TImage>
void DeepCopy(const TImage* const input, TImage* const output);

/** Extract a region of an image. */
template<typename TImage>
void ExtractRegion(const TImage* const image, const itk::ImageRegion<2>& region,
                   TImage* const output);

std::string VectorToString(const VectorImageType::PixelType& vec);

void RGBImageToCIELabImage(RGBImageType* const rgbImage, VectorImageType* const cielabImage);
  
void NormalizeVectorImage(FloatVector2ImageType* const image);


void SetMaskTransparency(Mask* const input, vtkImageData* outputImage);

void ITKRegionToVTKImage(VectorImageType* const image, const itk::ImageRegion<2>& region, vtkImageData* outputImage);

template<typename TImage>
void DeepCopyVectorImage(const TImage* const input, TImage* const output);

void ITKImagetoVTKImage(const VectorImageType* const image, vtkImageData* outputImage); // This function simply drives ITKImagetoVTKRGBImage or ITKImagetoVTKMagnitudeImage
void ITKImagetoVTKRGBImage(const VectorImageType* const image, vtkImageData* outputImage);
void ITKImagetoVTKMagnitudeImage(const VectorImageType* const image, vtkImageData* outputImage);

void ITKImagetoVTKVectorFieldImage(const FloatVector2ImageType* const image, vtkImageData* outputImage);

void VectorImageToRGBImage(const VectorImageType* const image, RGBImageType* const rgbImage);

template <typename TImage>
void ITKScalarImageToScaledVTKImage(TImage* const image, vtkImageData* outputImage);

itk::Index<2> GetRegionCenter(const itk::ImageRegion<2> region);

template <typename TDebugImageType>
void DebugWriteSequentialImage(TDebugImageType* const image, const std::string& filePrefix, const unsigned int iteration);

template <typename TDebugImageType>
void DebugWriteImageConditional(TDebugImageType* const image, const std::string& fileName, const bool condition);

template <class TImage>
void WriteScaledScalarImage(const TImage* const image, const std::string filename);

template <class TImage>
void CopyPatch(const TImage* const sourceImage, TImage* const targetImage, const itk::Index<2> sourcePosition,
               const itk::Index<2> targetPosition, const unsigned int radius);

template <class T>
void CreateConstantPatch(T* const patch, T* const value, const unsigned int radius);

template<typename T>
void ReplaceValue(T* const image, const T* const queryValue, const T* const replacementValue);

template<typename TImage>
void WriteImage(const TImage* const image, const std::string filename);

template <class T>
void CopyPatchIntoImage(T* const patch, T* const image, const itk::Index<2> position);

template <class T>
void CreateBlankPatch(T* const patch, const unsigned int radius);

template <class TImage>
void CopySelfPatchIntoValidRegion(TImage* const image, const UnsignedCharScalarImageType* const mask,
                                  const itk::ImageRegion<2> sourceRegion, const itk::ImageRegion<2> destinationRegion);

// Non template function declarations
itk::ImageRegion<2> GetRegionInRadiusAroundPixel(const itk::Index<2> pixel, const unsigned int radius);

itk::Index<2> GetIndexFromImageSlice(vtkImageSlice*);

void BlankAndOutlineImage(vtkImageData*, const unsigned char color[3]);

void OutlineImage(vtkImageData*, const unsigned char color[3]);

QImage ITKImageToQImage(const VectorImageType* const itkimage);

QImage ITKImageToQImage(const VectorImageType* const itkimage, const itk::ImageRegion<2>& region);

template<typename T>
void SetObjectToZero(T& object);

/** Convert an image to a QImage, but changed the corresponding masked pixels to the specified 'color'.*/
template <typename TImage>
QImage GetQImageMasked(const TImage* const image, const Mask* const mask,
                       const itk::ImageRegion<2>& region, const QColor& color = QColor(0, 255, 0));

template <typename TImage>
QImage GetQImageMasked(const TImage* image, const itk::ImageRegion<2>& imageRegion, const Mask* const mask,
                       const itk::ImageRegion<2>& maskRegion, const QColor& holeColor);

template<typename T>
unsigned int length(const std::vector<T>& v);

template<typename T>
unsigned int length(const itk::VariableLengthVector<T>& v);

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, unsigned int>::type length(const T& t);

std::vector<itk::Index<2> > OffsetsToIndices(const std::vector<itk::Offset<2> >& offsets);

std::vector<itk::Index<2> > OffsetsToIndices(const std::vector<itk::Offset<2> >& offsets, const itk::Index<2>& index);

template<typename TImage>
void SetImageToConstant(TImage* const image, const typename TImage::PixelType& value);

template<typename T>
T& index(itk::VariableLengthVector<T>& v, size_t i);

template<typename T>
T index(const itk::VariableLengthVector<T>& v, size_t i);

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T&>::type index(T& t, size_t);

template<typename T>
typename std::enable_if<std::is_fundamental<T>::value, T>::type index(const T& t, size_t);

template<typename T>
typename std::enable_if<!std::is_fundamental<T>::value, typename T::value_type&>::type index(T& v, size_t i);

template<typename T>
typename std::enable_if<!std::is_fundamental<T>::value, typename T::value_type>::type index(const T& v, size_t i);

template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageInRegion(const TImage* const image,
                                                                            const itk::ImageRegion<2>& region);

template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceInRegion(const TImage* const image,
                                                                             const itk::ImageRegion<2>& region);

/** Compute the average of all unmasked pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType AverageInRegionMasked(const TImage* const image,
                                                                                  const Mask* const mask,
                                                                            const itk::ImageRegion<2>& region);

/** Compute the average of all unmasked pixels in a region.*/
template<typename TImage>
typename TypeTraits<typename TImage::PixelType>::LargerType VarianceInRegionMasked(const TImage* const image,
                                                                                   const Mask* const mask,
                                                                             const itk::ImageRegion<2>& region);

}// end namespace

#include "Helpers.hxx"

#endif
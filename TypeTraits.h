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

#ifndef TypeTraits_H
#define TypeTraits_H

// ITK
#include "itkVariableLengthVector.h"

// STL
#include <vector>

/** For generic types (assume they are scalars). */
template <class T>
struct TypeTraits
{
  typedef T LargerType;
  typedef T LargerComponentType;
  typedef T ComponentType;
};

/** For unsigned char, use float as the LargerType */
template <>
struct TypeTraits<unsigned char>
{
  typedef float LargerType;
  typedef float LargerComponentType;
  typedef unsigned char ComponentType;
};

/** For generic std::vector. */
template <typename T>
struct TypeTraits<std::vector<T> >
{
  typedef std::vector<T> LargerType;
  typedef typename TypeTraits<T>::LargerType LargerComponentType;
  typedef T ComponentType;
};

/** For generic itk::VariableLengthVector, use the same type as the LargerType */
template <typename T>
struct TypeTraits<itk::VariableLengthVector<T> >
{
  typedef itk::VariableLengthVector<T> LargerType;
  typedef typename TypeTraits<T>::LargerType LargerComponentType;
  typedef T ComponentType;
};

/** For generic itk::VariableLengthVector<unsigned char>, use itk::VariableLengthVector<float> as the LargerType */
template <>
struct TypeTraits<itk::VariableLengthVector<unsigned char> >
{
  typedef itk::VariableLengthVector<float> LargerType;
  typedef float LargerComponentType;
  typedef unsigned char ComponentType;
};

#endif

#ifndef PixelDifferences_H
#define PixelDifferences_H

struct SumOfSquaredDifferences
{
  template <typename TPixel>
  float operator()(const VectorType &a, const VectorType &b)
  {
    float difference = 0;

    float diff = 0;
    for(unsigned int i = 0; i < a.GetSize(); ++i)
      {
      diff = a[i] - b[i];
      difference += diff * diff;
      }
    return difference;
  }
};


struct SumOfAbsoluteDifferences
{
  float operator()(const VectorType &a, const VectorType &b)
  {
    float difference = 0;

    for(unsigned int i = 0; i < a.GetSize(); ++i)
      {
      difference += fabs(a[i] - b[i]);
      }
    return difference;
  }
};

#endif

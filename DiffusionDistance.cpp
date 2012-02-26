#include "DiffusionDistance.h"

#include "Helpers.h"

float DiffusionDistance::SumOfAbsoluteDifference(const std::vector<float>& a, const std::vector<float>& b)
{
  float sum = 0.0f;

  for(unsigned int i = 0; i < a.size(); ++i)
  {
    sum += fabs(a[i] - b[i]);
  }
  return sum;
}

float DiffusionDistance::SumOfAbsoluteDifference(const Eigen::VectorXf& a, const Eigen::VectorXf& b)
{
  float sum = 0.0f;

  for(unsigned int i = 0; i < static_cast<unsigned int>(a.size()); ++i)
  {
    sum += fabs(a[i] - b[i]);
  }
  return sum;
}

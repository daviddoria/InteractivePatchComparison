#ifndef DiffusionDistance_HPP
#define DiffusionDistance_HPP

// Eigen
#include <Eigen/Dense>


struct DiffusionDistance
{
  float operator()(const std::vector<float>& a, const std::vector<float>& b,
                   const std::vector<std::vector<float> > allPoints);

  float SumOfAbsoluteDifference(const std::vector<float>& a, const std::vector<float>& b);

  float SumOfAbsoluteDifference(const Eigen::VectorXf& a, const Eigen::VectorXf& b);
};

#endif

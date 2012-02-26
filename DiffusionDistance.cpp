#include "DiffusionDistance.h"

#include "Helpers.h"

template <typename TPair>
bool SortByFirst(TPair object1, TPair object2)
{
  return object1.first < object2.first;
}

float DiffusionDistance::operator()(const std::vector<float>& a, const std::vector<float>& b,
                 const std::vector<std::vector<float> > allPoints)
{
  Eigen::MatrixXf L(allPoints.size(), allPoints.size());

  for(unsigned int i = 0; i < allPoints.size(); ++i)
  {
    for(unsigned int j = 0; j < allPoints.size(); ++j)
    {
      float difference = SumOfAbsoluteDifference(allPoints[i], allPoints[j]);
      L(i,j) = difference;
      //L(j,i) = difference; // We could only iterate over half of the matrix and do this.
    }
  }

  Eigen::MatrixXf D(allPoints.size(), allPoints.size());
  D.setIdentity();

  // Sum the rows of L, and set the diagonal entries of D to these sums
  for(unsigned int row = 0; row < allPoints.size(); ++row)
  {
    float sumOfRow = Helpers::SumOfRow(L, row);
    D(row, row) = sumOfRow;
  }

  // Compute M = D^-1 L
  Eigen::MatrixXf M = D.inverse() * L;
  
  // Compute the eigenvectors of M
  
  Eigen::SelfAdjointEigenSolver<Eigen::MatrixXf> eigensolver(M);
  if (eigensolver.info() != Eigen::Success)
  {
    throw std::runtime_error("Computing eigenvectors failed!");
  }
  
  Eigen::VectorXf eigenvalues = eigensolver.eigenvalues();
  Eigen::MatrixXf eigenvectorMatrix = eigensolver.eigenvectors();

  // Sort by increating magnitude
  typedef std::pair<float, Eigen::VectorXf> PairType;
  std::vector<PairType> pairs;
  for(unsigned int i = 0; i < static_cast<unsigned int>(eigenvalues.size()); ++i)
  {
    PairType pair;
    pair.first = eigenvalues[i];
    pair.second = eigenvectorMatrix.col(i);
    pairs.push_back(pair);
  }

  std::sort(pairs.begin(), pairs.end(), SortByFirst<PairType>);

  // Keep only the eigenvectors with the largest eigenvalues ("the size of a" of them)
  Eigen::MatrixXf sortedEigenvectorMatrix(eigenvalues.size(), a.size());
  for(unsigned int i = 0; i < static_cast<unsigned int>(a.size()); ++i)
  {
    sortedEigenvectorMatrix.col(i) = pairs[i].second;
  }

  // Conver the inputs to Eigen3 structures
  Eigen::VectorXf a_eigen = Helpers::STDVectorToEigenVector(a);
  Eigen::VectorXf b_eigen = Helpers::STDVectorToEigenVector(b);

  std::cout << "a_eigen: " << a_eigen << std::endl;
  //std::cout << "sortedEigenvectorMatrix: " << sortedEigenvectorMatrix << std::endl;
  std::cout << "sortedEigenvectorMatrix size: " << sortedEigenvectorMatrix.rows() << " "
            << sortedEigenvectorMatrix.cols() << std::endl;
  
  Eigen::VectorXf a_transformed = sortedEigenvectorMatrix * a_eigen;

  Eigen::VectorXf b_transformed = sortedEigenvectorMatrix * b_eigen;

  float finalDistance = SumOfAbsoluteDifference(a_transformed, b_transformed);

  return finalDistance;
}

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

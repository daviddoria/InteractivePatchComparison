
  // Diffusion distance of feature vectors
//   std::vector<Eigen::VectorXf> allPoints;
//   itk::Index<2> targetCenter = Helpers::GetRegionCenter(targetRegion);
//   itk::ImageRegion<2> neighborhoodRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, 4);
//   itk::ImageRegionConstIterator<Mask> neighborhoodIterator(MaskImage, neighborhoodRegion);
//
//   while(!neighborhoodIterator.IsAtEnd())
//     {
//     itk::ImageRegion<2> nearbyRegion = Helpers::GetRegionInRadiusAroundPixel(neighborhoodIterator.GetIndex(),
//                                                                              GetPatchRadius());
//     Eigen::VectorXf nearbyFeatures = ComputeNormalizedFeatures(nearbyRegion);
//     allPoints.push_back(nearbyFeatures);
//     ++neighborhoodIterator;
//     }
//
//   allPoints.push_back(sourceFeatures);
//
//   DiffusionDistance diffusionDistanceFunctor;
//   float diffusionDistance = diffusionDistanceFunctor(targetFeatures, sourceFeatures, allPoints);
//   std::cout << "diffusionDistance: " << diffusionDistance << std::endl;

  // Diffusion distance of pixel values directly
  {
//   std::vector<Eigen::VectorXf> allPoints;
//   itk::Index<2> targetCenter = Helpers::GetRegionCenter(targetRegion);
//   itk::ImageRegion<2> neighborhoodRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, 4);
//   itk::ImageRegionConstIterator<Mask> neighborhoodIterator(MaskImage, neighborhoodRegion);
//
//   Eigen::VectorXf sourceRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), sourceRegion);
//   Eigen::VectorXf targetRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), targetRegion);
//
//   std::cout << "Features have " << targetRegionVectorized.size() << " components." << std::endl;
//   while(!neighborhoodIterator.IsAtEnd())
//     {
//     itk::ImageRegion<2> nearbyRegion = Helpers::GetRegionInRadiusAroundPixel(neighborhoodIterator.GetIndex(), GetPatchRadius());
//     Eigen::VectorXf nearbyFeatures = Helpers::GetRegionAsVector(Image.GetPointer(), nearbyRegion);
//     allPoints.push_back(nearbyFeatures);
//     ++neighborhoodIterator;
//     }
//
//   while(allPoints.size() < static_cast<unsigned int>(sourceRegionVectorized.size()))
//   {
// //     Eigen::VectorXf emptyFeature(sourceRegionVectorized.size());
// //     for(unsigned int i = 0; i < static_cast<unsigned int>(emptyFeature.size()); ++i)
// //       {
// //       emptyFeature[i] = 0.0f;
// //       }
// //     allPoints.push_back(emptyFeature);
//     allPoints.push_back(targetRegionVectorized);
//   }
//
//   allPoints.push_back(sourceRegionVectorized);
//
//   std::cout << "There are " << allPoints.size() << " points." << std::endl;
//
//   DiffusionDistance diffusionDistanceFunctor;
//   float diffusionDistance = diffusionDistanceFunctor(sourceRegionVectorized, targetRegionVectorized, allPoints);
//   std::cout << "diffusionDistance: " << diffusionDistance << std::endl;
  }

  // Diffusion distance of pixel values directly (smaller patches)
//   {
//   unsigned int smallPatchRadius = 5;
//   std::vector<Eigen::VectorXf> allPoints;
//   itk::Index<2> targetCenter = Helpers::GetRegionCenter(targetRegion);
//   itk::Index<2> sourceCenter = Helpers::GetRegionCenter(sourceRegion);
//   itk::ImageRegion<2> neighborhoodRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, 4);
//   itk::ImageRegionConstIterator<Mask> neighborhoodIterator(MaskImage, neighborhoodRegion);
//
//   itk::ImageRegion<2> smallSourceRegion = Helpers::GetRegionInRadiusAroundPixel(sourceCenter, smallPatchRadius);
//   itk::ImageRegion<2> smallTargetRegion = Helpers::GetRegionInRadiusAroundPixel(targetCenter, smallPatchRadius);
//
//   Eigen::VectorXf sourceRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), smallSourceRegion);
//   Eigen::VectorXf targetRegionVectorized = Helpers::GetRegionAsVector(Image.GetPointer(), smallTargetRegion);
//
//   std::cout << "Features have " << targetRegionVectorized.size() << " components." << std::endl;
//   while(!neighborhoodIterator.IsAtEnd())
//     {
//     itk::ImageRegion<2> nearbyRegion = Helpers::GetRegionInRadiusAroundPixel(neighborhoodIterator.GetIndex(), smallPatchRadius);
//     Eigen::VectorXf nearbyFeatures = Helpers::GetRegionAsVector(Image.GetPointer(), nearbyRegion);
//     allPoints.push_back(nearbyFeatures);
//     ++neighborhoodIterator;
//     }
//
//   while(allPoints.size() < static_cast<unsigned int>(sourceRegionVectorized.size()))
//   {
// //     Eigen::VectorXf emptyFeature(sourceRegionVectorized.size());
// //     for(unsigned int i = 0; i < static_cast<unsigned int>(emptyFeature.size()); ++i)
// //       {
// //       emptyFeature[i] = 0.0f;
// //       }
// //     allPoints.push_back(emptyFeature);
//     allPoints.push_back(targetRegionVectorized);
//   }
//
//   allPoints.push_back(sourceRegionVectorized);
//
//   std::cout << "There are " << allPoints.size() << " points." << std::endl;
//
//   DiffusionDistance diffusionDistanceFunctor;
//   float diffusionDistance = diffusionDistanceFunctor(sourceRegionVectorized, targetRegionVectorized, allPoints);
//   std::cout << "diffusionDistance: " << diffusionDistance << std::endl;
//   }
  
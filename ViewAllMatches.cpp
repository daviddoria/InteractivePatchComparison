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

#include <QApplication>
#include <QCleanlooksStyle>

#include "ViewAllMatchesWidget.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  QApplication::setStyle(new QCleanlooksStyle);

  ViewAllMatchesWidget* viewAllMatchesWidget = NULL;

  if(argc == 1)
  {
    std::cout << "Default" << std::endl;
    viewAllMatchesWidget = new ViewAllMatchesWidget;
  }
  else if(argc == 4)
  {
    std::cout << "Full" << std::endl;
    std::stringstream ss;
    for(int i = 1; i < argc; ++i)
    {
      ss << argv[i] << " ";
    }

    std::string imageFileName;
    std::string matchFileName;
    unsigned int patchRadius;

    ss >> imageFileName >> matchFileName >> patchRadius;

    std::cout << "imageFileName: " << imageFileName << std::endl;
    std::cout << "matchFileName: " << matchFileName << std::endl;
    std::cout << "patchRadius: " << patchRadius << std::endl;

    viewAllMatchesWidget = new ViewAllMatchesWidget(imageFileName, matchFileName, patchRadius);
  }
  else
  {
    std::cout << "argc is " << argc << std::endl;
    std::cerr << "Invalid input arguments!" << std::endl;
    for(int i = 1; i < argc; ++i)
    {
      //std::cerr << argv[i] << " ";
      std::cout << argv[i] << " ";
    }
    return EXIT_FAILURE;
  }

  viewAllMatchesWidget->show();

  return app.exec();
}

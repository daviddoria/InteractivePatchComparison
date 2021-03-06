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

#include "InteractivePatchComparisonWidget.h"

int main( int argc, char** argv )
{
  QApplication app( argc, argv );

  QApplication::setStyle(new QCleanlooksStyle);

  InteractivePatchComparisonWidget* interactivePatchComparisonWidget = NULL;

  if(argc == 1)
  {
    interactivePatchComparisonWidget = new InteractivePatchComparisonWidget;
    std::cout << "Called with no arguments. Potential arguments are: image.png" << std::endl;
  }
  else if(argc == 2)
  {
    std::string imageFileName = argv[1];

    std::cout << "imageFileName: " << imageFileName << std::endl;

    interactivePatchComparisonWidget = new InteractivePatchComparisonWidget(imageFileName);
  }
  else if(argc == 3)
  {
    std::string imageFileName = argv[1];
    std::string maskFileName = argv[2];

    std::cout << "imageFileName: " << imageFileName << std::endl;
    std::cout << "maskFileName: " << maskFileName << std::endl;

    interactivePatchComparisonWidget = new InteractivePatchComparisonWidget(imageFileName, maskFileName);
  }
  else
  {
    std::cerr << "Input arguments invalid!" << std::endl;
    return EXIT_FAILURE;
  }
  
  interactivePatchComparisonWidget->show();

  return app.exec();
}

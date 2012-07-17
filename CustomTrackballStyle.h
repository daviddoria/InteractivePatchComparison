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

#ifndef CustomTrackballStyle_H
#define CustomTrackballStyle_H

#include "CustomTrackballStyle.h"

#include <vtkCommand.h>
#include <vtkInteractorStyleTrackballActor.h>

//#include "CustomImageStyle.h"
class CustomImageStyle;

// Define interaction style
class CustomTrackballStyle : public vtkInteractorStyleTrackballActor
{
public:
  const static unsigned int PatchesMovedEvent = vtkCommand::UserEvent + 1;

  static CustomTrackballStyle* New();
  
  vtkTypeMacro(CustomTrackballStyle,vtkInteractorStyleTrackballActor);

  void OnLeftButtonDown();

  void OnLeftButtonUp();

  void OnMiddleButtonDown();

  void OnRightButtonDown();

  void SetOtherStyle(CustomImageStyle* style);

private:
  CustomImageStyle* OtherStyle;
  
};

#endif

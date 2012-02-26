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

#ifndef PatchInfoWidget_H
#define PatchInfoWidget_H

#include "ui_PatchInfoWidget.h"

// Qt
#include <QWidget>

// Custom
#include "Mask.h"
#include "Types.h"

class SwitchBetweenStyle;

class PatchInfoWidget : public QWidget, public Ui::PatchInfoWidget
{
Q_OBJECT
public:

  PatchInfoWidget(QWidget* parent);

  void SetImage(VectorImageType* const image);
  void SetMask(Mask* const mask);

  void Save(const std::string& prefix);

  itk::ImageRegion<2> GetRegion() const;
  
signals:
  void signal_PatchMoved(const itk::ImageRegion<2>& patchRegion);
  
public slots:

  void on_txtXCenter_returnPressed();
  void on_txtYCenter_returnPressed();

  void slot_Update(const itk::ImageRegion<2>& patchRegion);

private:

  unsigned int GetRadius();
  VectorImageType::Pointer Image;
  Mask::Pointer MaskImage;

  itk::ImageRegion<2> Region;
};

#endif

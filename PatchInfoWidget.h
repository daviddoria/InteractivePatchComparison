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

#ifndef PatchInfoWidget_H
#define PatchInfoWidget_H

#include "ui_PatchInfoWidget.h"

// Qt
#include <QWidget>

// Custom
#include "PatchComparison/Mask/Mask.h"
#include "Types.h"

class SwitchBetweenStyle;


class PatchInfoWidgetParent : public QWidget, public Ui::PatchInfoWidget
{
Q_OBJECT
public:

  PatchInfoWidgetParent(QWidget* parent) : QWidget(parent){}

signals:
  void signal_PatchMoved(const itk::ImageRegion<2>& patchRegion);

public slots:

//   virtual void on_btnSavePatch_clicked() = 0;
// 
//   virtual void on_spinXCenter_valueChanged(int) = 0;
//   virtual void on_spinYCenter_valueChanged(int) = 0;
// 
//   virtual void slot_Update(const itk::ImageRegion<2>& patchRegion) = 0;

// Do not make these pure virtual, because then the QWidget promoted to a PatchInfoWidgetParent in QtDesigner cannot be instantiated
  virtual void on_btnSavePatch_clicked(){};

  virtual void on_spinXCenter_valueChanged(int){};
  virtual void on_spinYCenter_valueChanged(int){};

  virtual void slot_Update(const itk::ImageRegion<2>& patchRegion){};

// Non slots - need these because Designer instantiates the PatchInfoWidget as PatchInfoWidgetParent, so PatchInfoWidgetParent->SetImage, etc needs to exist
  virtual void SetImage(TImage* const image) {};

  virtual void SetMask(Mask* const mask) {};

  virtual void Save(const std::string& prefix) {};

  virtual itk::ImageRegion<2> GetRegion() const {};

  virtual void MakeInvalid() {};
};

template<typename TImage>
class PatchInfoWidget : public PatchInfoWidgetParent
{
public:

  PatchInfoWidget(QWidget* parent);

  void SetImage(TImage* const image);
  void SetMask(Mask* const mask);

  void Save(const std::string& prefix);

  itk::ImageRegion<2> GetRegion() const;

  void MakeInvalid();

//public slots:

  void on_btnSavePatch_clicked();

  void on_spinXCenter_valueChanged(int);
  void on_spinYCenter_valueChanged(int);

  void slot_Update(const itk::ImageRegion<2>& patchRegion);

private:

  unsigned int GetRadius();
  TImage* Image;
  Mask* MaskImage;

  itk::ImageRegion<2> Region;

  bool eventFilter(QObject *object, QEvent *event);
};

#include "PatchInfoWidget.hpp"

#endif

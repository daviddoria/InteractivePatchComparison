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

/** This class exists only because Qt does not allow templated widgets.
  * The function descriptions are provided in the child class, PatchInfoWidget
  */
class PatchInfoWidgetParent : public QWidget, public Ui::PatchInfoWidget
{
Q_OBJECT
public:

  PatchInfoWidgetParent(QWidget* parent) : QWidget(parent){}

signals:
  void signal_PatchMoved(const itk::ImageRegion<2>& patchRegion);

public slots:

  virtual void on_btnSavePatch_clicked() = 0;

  virtual void on_spinXCenter_valueChanged(int) = 0;
  virtual void on_spinYCenter_valueChanged(int) = 0;

  virtual void on_chkFlip_stateChanged(int) = 0;

  virtual void slot_SetRegion(const itk::ImageRegion<2>& patchRegion) = 0;

private:

  virtual void showEvent(QShowEvent* event) = 0;

  virtual void resizeEvent(QResizeEvent* event) = 0;
};

template<typename TImage>
class PatchInfoWidget : public PatchInfoWidgetParent
{
public:

  /** Constructor. */
  PatchInfoWidget(QWidget* parent = NULL);

  /** Set the image to take the patch from. */
  void SetImage(TImage* const image);

  /** Set the mask that specifies valid regions of the image. */
  void SetMask(Mask* const mask);

  /** Save the selected patch. */
  void Save(const std::string& prefix);

  /** Get the region of the image that the widget is displaying. */
  itk::ImageRegion<2> GetRegion() const;

  /** If an invalid region is selected, make it clear that the selected patch is not valid. */
  void MakeInvalid();

  /** Update the widget.*/
  void Update();

//public slots:

  /** Called when "Save Patch" is clicked. */
  void on_btnSavePatch_clicked();

  /** Called when the value of the X coordinate is changed. */
  void on_spinXCenter_valueChanged(int);

  /** Called when the value of the Y coordinate is changed. */
  void on_spinYCenter_valueChanged(int);

  /** Set the region that the widget displays. */
  void slot_SetRegion(const itk::ImageRegion<2>& patchRegion);

  /** Update the widget when the Flip checkbox is toggled. */
  void on_chkFlip_stateChanged(int);
private:

  /** When the widget finishes loading, this function is called. */
  void showEvent(QShowEvent* event);

  /** When the widget is resized, this function is called. */
  void resizeEvent(QResizeEvent* event);

  /** Get the radius of the patch. */
  unsigned int GetRadius();

  /** The image from which to take patches. */
  TImage* Image;

  /** The mask indicating image validity. */
  Mask* MaskImage;

  /** The region to display. */
  itk::ImageRegion<2> Region;

  /** Handle events (not signals) of other widgets. */
  bool eventFilter(QObject *object, QEvent *event);
};

#include "PatchInfoWidget.hpp"

#endif

#include "CustomTrackballStyle.h"

#include "CustomImageStyle.h"

#include <vtkObjectFactory.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(CustomTrackballStyle);

void CustomTrackballStyle::OnLeftButtonDown()
{
  //std::cout << "TrackballStyle::OnLeftButtonDown()" << std::endl;
  // Behave like the middle button instead
  vtkInteractorStyleTrackballActor::OnMiddleButtonDown();
}

void CustomTrackballStyle::OnLeftButtonUp()
{
  // Behave like the middle button instead
  vtkInteractorStyleTrackballActor::OnMiddleButtonUp();
  this->InvokeEvent(this->PatchesMovedEvent, InteractionProp);
}

void CustomTrackballStyle::OnMiddleButtonDown()
{
  this->Interactor->SetInteractorStyle(this->OtherStyle);
  this->OtherStyle->OnMiddleButtonDown();
}

void CustomTrackballStyle::OnRightButtonDown()
{
  this->Interactor->SetInteractorStyle(this->OtherStyle);
  this->OtherStyle->OnRightButtonDown();
}

void CustomTrackballStyle::SetOtherStyle(CustomImageStyle* style)
{
  this->OtherStyle = style;
}

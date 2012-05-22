#include "TopPatchesWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>

// Custom
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "Mask/Mask.h"
#include "PixmapDelegate.h"

TopPatchesWidget::TopPatchesWidget(QWidget* parent) : QWidget(parent)
{
  this->setupUi(this);

  if(parent)
  {
    this->setGeometry(QRect(parent->pos().x() + parent->width(), parent->pos().y(), this->width(), this->height()));
  }
//   if(image->GetNumberOfComponentsPerPixel() == 3)
//   {
//     // assume the image is RGB, and use it directly
//     ITKHelpers::DeepCopy(image, this->Image);
//   }

  this->TargetPatchItem = new QGraphicsPixmapItem;
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(this->TargetPatchScene);

  this->TopPatchesModel = new TableModelTopPatches(this);
  this->tblviewTopPatches->setModel(TopPatchesModel);

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

  connect(this->tblviewTopPatches, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_SingleClicked(const QModelIndex&)));
}

void TopPatchesWidget::SetTargetRegion(const itk::ImageRegion<2>& targetRegion)
{
  this->TargetRegion = targetRegion;
  
  //QImage maskedQueryPatch = MaskOperations::GetQImageMasked(Image, MaskImage, targetRegion);
  //MaskedQueryPatchItem = this->QueryPatchScene->addPixmap(QPixmap::fromImage(maskedQueryPatch));

  // We do this here because we could potentially call SetQueryNode after the widget is constructed as well.
  //gfxQueryPatch->fitInView(MaskedQueryPatchItem);
}

void TopPatchesWidget::slot_SingleClicked(const QModelIndex& selected)
{
  emit TopPatchSelected(selected);
}

void TopPatchesWidget::SetImage(ImageType* const image)
{
  this->Image = image;
}

#include "TopPatchesWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"
#include "Mask/Mask.h"

// Custom
#include "PixmapDelegate.h"
#include "SelfPatchCompare.h"

TopPatchesWidget::TopPatchesWidget(QWidget* parent) : QWidget(parent)
{
  this->setupUi(this);

  if(parent)
  {
    this->setGeometry(QRect(parent->pos().x() + parent->width(), parent->pos().y(), this->width(), this->height()));
  }

  this->TargetPatchItem = new QGraphicsPixmapItem;
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(this->TargetPatchScene);

  this->TopPatchesModel = new TableModelTopPatches(this);
  this->tblviewTopPatches->setModel(TopPatchesModel);
  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->txtNumberOfPatches->text().toInt());

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

  connect(this->tblviewTopPatches, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_SingleClicked(const QModelIndex&)));
}

void TopPatchesWidget::SetTargetRegion(const itk::ImageRegion<2>& targetRegion)
{
  this->TargetRegion = targetRegion;
  
  QImage patchImage = ITKQtHelpers::GetQImageColor(this->Image, targetRegion);

  QPixmap pixmap = QPixmap::fromImage(patchImage);

  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(TargetPatchScene);

  this->TargetPatchScene->addPixmap(pixmap);
}

void TopPatchesWidget::slot_SingleClicked(const QModelIndex& selected)
{
  emit signal_TopPatchSelected(this->TopPatchesModel->GetTopPatchData()[selected.row()].first);
}

void TopPatchesWidget::SetImage(ImageType* const image)
{
  this->Image = image;
  this->TopPatchesModel->SetImage(this->Image);
}

void TopPatchesWidget::on_btnCompute_clicked()
{
  SelfPatchCompare patchCompare;
  patchCompare.SetImage(this->Image);
  //this->PatchCompare.SetMask(this->MaskImage);

  patchCompare.SetTargetRegion(this->TargetRegion);

  patchCompare.ComputePatchScores();

  std::vector<SelfPatchCompare::PatchDataType> topPatchData = patchCompare.GetPatchData();

  //std::sort(topPatchData.begin(), topPatchData.end(), Helpers::SortBySecondAccending<PatchDataType>);
  std::partial_sort(topPatchData.begin(), topPatchData.begin() + this->txtNumberOfPatches->text().toInt(),
                    topPatchData.end(), Helpers::SortBySecondAccending<SelfPatchCompare::PatchDataType>);
  
  std::cout << "There are " << topPatchData.size() << " top patches." << std::endl;

  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->txtNumberOfPatches->text().toInt());
  this->TopPatchesModel->SetTopPatchData(topPatchData);
  this->TopPatchesModel->Refresh();
}

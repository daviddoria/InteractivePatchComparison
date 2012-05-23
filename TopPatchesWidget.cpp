#include "TopPatchesWidget.h"

// ITK
#include "itkCastImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkMaskImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"

// Qt
#include <QGraphicsPixmapItem>
#include <QProgressDialog>
#include <QtConcurrentRun>

// Submodules
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"
#include "ITKVTKHelpers/ITKVTKHelpers.h"
#include "QtHelpers/QtHelpers.h"
#include "ITKQtHelpers/ITKQtHelpers.h"
#include "Mask/Mask.h"
#include "PatchClustering/PatchClustering.h"
#include "PatchClustering/KMeansClustering/KMeansClustering.h"

// Custom
#include "PixmapDelegate.h"
#include "SelfPatchCompare.h"

TopPatchesWidget::TopPatchesWidget(QWidget* parent) : QWidget(parent)
{
  this->setupUi(this);

//   if(parent)
//   {
//     this->setGeometry(QRect(parent->pos().x() + parent->width(), parent->pos().y(), this->width(), this->height()));
//   }

  this->TargetPatchItem = new QGraphicsPixmapItem;
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(this->TargetPatchScene);

  this->TopPatchesModel = new TableModelTopPatches(this->TopPatchData, this);
  this->tblviewTopPatches->setModel(TopPatchesModel);
  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->txtNumberOfPatches->text().toInt());

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

  //connect(this->tblviewTopPatches, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slot_SingleClicked(const QModelIndex&)));

  connect(this->tblviewTopPatches->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(slot_SelectionChanged(const QItemSelection &, const QItemSelection &)));

  // Setup progress bar
  this->ProgressDialog = new QProgressDialog();
  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->hide();

  connect(&this->FutureWatcher, SIGNAL(finished()), this, SLOT(slot_Finished()));
  connect(&this->FutureWatcher, SIGNAL(finished()), this->ProgressDialog , SLOT(cancel()));
}

void TopPatchesWidget::slot_Finished()
{
  std::cout << "Finshed" << std::endl;
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

void TopPatchesWidget::slot_SelectionChanged(const QItemSelection &, const QItemSelection &)
{
  QModelIndexList indexes = this->tblviewTopPatches->selectionModel()->selection().indexes();

  std::vector<itk::ImageRegion<2> > topSourceRegions;

  for (int i = 0; i < indexes.count(); ++i)
  {
    //std::cout << "selectedIndexes: " << indexes.at(i).row() << std::endl;
    topSourceRegions.push_back(this->TopPatchesModel->GetTopPatchData()[indexes.at(i).row()].first);
  }

  emit signal_TopPatchesSelected(topSourceRegions);
}

// void TopPatchesWidget::slot_SingleClicked(const QModelIndex& selected)
// {
//   emit signal_TopPatchSelected(this->TopPatchesModel->GetTopPatchData()[selected.row()].first);
// }

void TopPatchesWidget::SetImage(ImageType* const image)
{
  this->Image = image;
  this->TopPatchesModel->SetImage(this->Image);
}

void TopPatchesWidget::on_btnCompute_clicked()
{
  // Start the computation.
  QFuture<void> future = QtConcurrent::run(this, &TopPatchesWidget::Compute);
  this->FutureWatcher.setFuture(future);

  this->ProgressDialog->setMinimum(0);
  this->ProgressDialog->setMaximum(0);
  this->ProgressDialog->setWindowModality(Qt::WindowModal);
  this->ProgressDialog->exec();

}

void TopPatchesWidget::Compute()
{
  SelfPatchCompare patchCompare;
  patchCompare.SetImage(this->Image);
  //this->PatchCompare.SetMask(this->MaskImage);

  patchCompare.SetTargetRegion(this->TargetRegion);

  patchCompare.ComputePatchScores();

  this->TopPatchData = patchCompare.GetPatchData();

  //std::sort(topPatchData.begin(), topPatchData.end(), Helpers::SortBySecondAccending<PatchDataType>);
  unsigned int numberOfPatches = this->txtNumberOfPatches->text().toInt();
  std::partial_sort(this->TopPatchData.begin(), this->TopPatchData.begin() + numberOfPatches,
                    this->TopPatchData.end(), Helpers::SortBySecondAccending<SelfPatchCompare::PatchDataType>);

  this->TopPatchData.resize(numberOfPatches);
  std::cout << "There are " << this->TopPatchData.size() << " top patches." << std::endl;

  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->txtNumberOfPatches->text().toInt());
  this->TopPatchesModel->SetTopPatchData(this->TopPatchData);
  this->TopPatchesModel->Refresh();

  Cluster();
}

void TopPatchesWidget::Cluster()
{
  EigenHelpers::VectorOfVectors vectors(this->TopPatchData.size());
  for(unsigned int i = 0; i < this->TopPatchData.size(); ++i)
    {
    Eigen::VectorXf v = PatchClustering::VectorizePatch(this->Image, this->TopPatchData[i].first);
    vectors[i] = v;
    }

  unsigned int numberOfClusters = this->txtClusters->text().toInt();
  KMeansClustering kmeans;
  kmeans.SetK(numberOfClusters);
  kmeans.SetPoints(vectors);
  kmeans.SetInitMethod(KMeansClustering::KMEANSPP);
  //kmeans.SetRandom(false); // for repeatable results
  kmeans.SetRandom(true); // for real, random results
  kmeans.Cluster();

  std::vector<unsigned int> labels = kmeans.GetLabels();

  this->TopPatchesModel->SetClusterIDs(labels);
}

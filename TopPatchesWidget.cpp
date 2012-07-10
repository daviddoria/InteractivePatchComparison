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
#include <QSortFilterProxyModel>

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

TopPatchesWidget::TopPatchesWidget(QWidget* parent) : QWidget(parent)
{
  this->setupUi(this);

  this->txtNumberOfPatches->installEventFilter(this);
  this->txtClusters->installEventFilter(this);

  QIntValidator* clusterValidator = new QIntValidator(1, 5);
  this->txtClusters->setValidator(clusterValidator);

  QIntValidator* numberOfPatchesValidator = new QIntValidator(1, 5000);
  this->txtNumberOfPatches->setValidator(numberOfPatchesValidator);
  
  // Make the cells fit the images (based on the sizeHint from the PixmapDelegate)
  this->tblviewTopPatches->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  this->tblviewTopPatches->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  this->TargetPatchItem = new QGraphicsPixmapItem;
  this->TargetPatchScene = new QGraphicsScene();
  this->gfxTargetPatch->setScene(this->TargetPatchScene);

  this->ProxyModel = new QSortFilterProxyModel;
  this->TopPatchesModel = new TableModelTopPatches(this->TopPatchData, this);
  this->ProxyModel->setSourceModel(this->TopPatchesModel);
  this->tblviewTopPatches->setModel(ProxyModel);
  this->TopPatchesModel->SetMaxTopPatchesToDisplay(this->txtNumberOfPatches->text().toInt());

  std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
  this->TopPatchesModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());

  PixmapDelegate* pixmapDelegate = new PixmapDelegate;

  this->tblviewTopPatches->setItemDelegate(pixmapDelegate);

  connect(this->tblviewTopPatches->selectionModel(),
          SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
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
  //std::cout << "Set target patch display height to: "
  //          << this->gfxTargetPatch->size().height() << std::endl;
  pixmap = pixmap.scaledToHeight(this->gfxTargetPatch->size().height());

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
    //topSourceRegions.push_back(this->TopPatchesModel->GetTopPatchData()[indexes.at(i).row()].first);
    unsigned int originalRowId = this->ProxyModel->mapToSource(indexes.at(i)).row(); // This was the row id before sorting
    topSourceRegions.push_back(this->TopPatchesModel->GetTopPatchData()[originalRowId].first);
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
  std::cout << "Set patch display size to: " << this->gfxTargetPatch->size().height() << std::endl;
  this->TopPatchesModel->SetPatchDisplaySize(this->gfxTargetPatch->size().height());
  this->tblviewTopPatches->resizeRowsToContents(); 
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
  this->SelfPatchCompareFunctor.SetImage(this->Image);
  //this->PatchCompare.SetMask(this->MaskImage);
  this->SelfPatchCompareFunctor.CreateFullyValidMask();

  this->SelfPatchCompareFunctor.SetTargetRegion(this->TargetRegion);

  this->SelfPatchCompareFunctor.ComputePatchScores();

  this->TopPatchData = this->SelfPatchCompareFunctor.GetPatchData();

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
  EigenHelpers::VectorOfFloatVectors vectors(this->TopPatchData.size());
  for(unsigned int i = 0; i < this->TopPatchData.size(); ++i)
    {
    VectorType v = PatchClustering::VectorizePatch(this->Image, this->TopPatchData[i].first);
    vectors[i] = v;
    }

  unsigned int numberOfClusters = this->txtClusters->text().toInt();
  KMeansClustering kmeans;
  kmeans.SetK(numberOfClusters);
  kmeans.SetPoints(vectors);
  kmeans.SetInitMethod(KMeansClustering::KMEANSPP);
  kmeans.SetRandom(true);
  kmeans.Cluster();

  std::vector<unsigned int> labels = kmeans.GetLabels();

  this->TopPatchesModel->SetClusterIDs(labels);
}

void TopPatchesWidget::SetPatchDistanceFunctor(PatchDistance* const patchDistanceFunctor)
{
  this->SelfPatchCompareFunctor.SetPatchDistanceFunctor(patchDistanceFunctor);
}

bool TopPatchesWidget::eventFilter(QObject *object, QEvent *event)
{
  // When the focus leaves one of the text boxes, update the patches
  QColor normalColor = QColor(255, 255, 255);
  if(object == txtClusters && event->type() == QEvent::FocusOut)
  {
    if(!this->txtClusters->hasAcceptableInput())
    {
      std::cerr << "Invalid number of clusters!" << std::endl;
      return false; // Pass the event along (don't consume it)
    }
    QPalette p = txtClusters->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    txtClusters->setPalette(p);
  }

  if(object == txtNumberOfPatches && event->type() == QEvent::FocusOut)
  {
    if(!this->txtNumberOfPatches->hasAcceptableInput())
    {
      std::cerr << "Invalid number of patches!" << std::endl;
      return false; // Pass the event along (don't consume it)
    }
    QPalette p = txtNumberOfPatches->palette();
    p.setColor( QPalette::Normal, QPalette::Base, normalColor);
    txtNumberOfPatches->setPalette(p);
  }

  return false; // Pass the event along (don't consume it)
}

void TopPatchesWidget::on_txtClusters_returnPressed()
{
  if(!this->txtClusters->hasAcceptableInput())
  {
    std::cerr << "Invalid patch radius!" << std::endl;
    return;
  }

  QColor normalColor = QColor(255, 255, 255);
  QPalette p = this->txtClusters->palette();
  p.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->txtClusters->setPalette(p);
}

void TopPatchesWidget::on_txtNumberOfPatches_returnPressed()
{
  if(!this->txtNumberOfPatches->hasAcceptableInput())
  {
    std::cerr << "Invalid patch radius!" << std::endl;
    return;
  }
  
  QColor normalColor = QColor(255, 255, 255);
  QPalette p = this->txtNumberOfPatches->palette();
  p.setColor( QPalette::Normal, QPalette::Base, normalColor);
  this->txtNumberOfPatches->setPalette(p);
}

void TopPatchesWidget::on_txtClusters_textEdited()
{
  QColor activeColor = QColor(255, 0, 0);
  QPalette p = this->txtClusters->palette();
  p.setColor( QPalette::Normal, QPalette::Base, activeColor);
  this->txtClusters->setPalette(p);
}

void TopPatchesWidget::on_txtNumberOfPatches_textEdited()
{
  QColor activeColor = QColor(255, 0, 0);
  QPalette p = this->txtNumberOfPatches->palette();
  p.setColor( QPalette::Normal, QPalette::Base, activeColor);
  this->txtNumberOfPatches->setPalette(p);
}

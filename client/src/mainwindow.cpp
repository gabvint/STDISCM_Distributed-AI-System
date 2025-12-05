#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      table_(new QTableWidget(this)),
      progressBar_(new QProgressBar(this)),
      addImagesButton_(new QPushButton("Add images", this))
{

    std::string serverAddress = "192.168.100.7:50051";
    grpcClient_ = std::make_unique<GrpcClient>(serverAddress);

    setupUi();

    connect(addImagesButton_, &QPushButton::clicked,
            this, &MainWindow::onAddImagesClicked);
}

void MainWindow::setupUi()
{
    auto central = new QWidget(this);
    auto vbox = new QVBoxLayout(central);

    auto hboxTop = new QHBoxLayout();
    hboxTop->addWidget(addImagesButton_);
    hboxTop->addStretch();

    table_->setColumnCount(4);
    QStringList headers;
    headers << "Image ID" << "File" << "Status" << "Text (preview)";
    table_->setHorizontalHeaderLabels(headers);

    auto header = table_->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Image ID
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents); // File
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Status
    header->setSectionResizeMode(3, QHeaderView::Stretch);          // Text

    table_->setSelectionMode(QAbstractItemView::NoSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->setFormat("%p%");

    progressLabel_ = new QLabel(this);
    progressLabel_->setText("Progress: 0% (0/0)");

    vbox->addLayout(hboxTop);
    vbox->addWidget(table_);
    vbox->addWidget(progressLabel_);
    vbox->addWidget(progressBar_);

    setCentralWidget(central);
    setWindowTitle("Distributed OCR Client");
    resize(900, 600);
}

void MainWindow::onAddImagesClicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select images",
        QString(),
        "Images (*.png *.jpg *.jpeg *.bmp)");

    if (files.isEmpty())
    {
        return;
    }

    // If previous batch already finished, start a new batch (clear UI + state)
    if (totalCount_ > 0 && doneCount_ == totalCount_)
    {
        table_->setRowCount(0);
        items_.clear();
        totalCount_ = 0;
        doneCount_ = 0;
        currentBatchId_.clear();
        errorShownForCurrentBatch_ = false;
    }

    if (currentBatchId_.isEmpty())
    {
        currentBatchId_ = QString("batch-%1").arg(++batchCounter_);
    }

    // Add all selected files to the current batch
    for (const QString &filePath : files)
    {
        ImageItem item;
        item.batchId = currentBatchId_;
        item.imageId = QString("img-%1").arg(items_.size() + 1);
        item.filePath = filePath;
        item.status = ImageItem::Pending;
        item.text.clear();

        // Insert row in the table
        int row = table_->rowCount();
        table_->insertRow(row);
        item.row = row;

        table_->setItem(row, 0, new QTableWidgetItem(item.imageId));
        table_->setItem(row, 1, new QTableWidgetItem(item.filePath));
        table_->setItem(row, 2, new QTableWidgetItem("Pending"));
        table_->setItem(row, 3, new QTableWidgetItem(""));

        // Append to items_
        items_.push_back(item);
        totalCount_++;

        // Start OCR for this new index
        int index = items_.size() - 1;
        startOcrForIndex(index);
    }

    updateProgress();
}

void MainWindow::startOcrForIndex(int idx)
{
    if (idx < 0 || idx >= items_.size())
        return;

    // Mark in-progress and update the row
    items_[idx].status = ImageItem::InProgress;
    updateRow(idx);

    // Capture everything needed by value for the worker
    QString batchId = items_[idx].batchId;
    QString imageId = items_[idx].imageId;
    QString filePath = items_[idx].filePath;

    auto watcher = new QFutureWatcher<GrpcClient::Result>(this);

    // Run gRPC call in a background thread
    QFuture<GrpcClient::Result> future = QtConcurrent::run(
        [this, batchId, imageId, filePath]()
        {
            return grpcClient_->processImage(
                batchId.toStdString(),
                imageId.toStdString(),
                filePath.toStdString());
        });

    connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher, idx]()
            {
        GrpcClient::Result res = watcher->result();
        watcher->deleteLater();

        if (idx < 0 || idx >= items_.size()) {
            return;
        }

                auto& item = items_[idx];
        if (res.ok) {
            item.status = ImageItem::Done;
            item.text = QString::fromStdString(res.text);
        } else {
            item.status = ImageItem::Error;
            item.text = QString::fromStdString(res.error);

        
            if (!errorShownForCurrentBatch_) {
                errorShownForCurrentBatch_ = true;
                QMessageBox::warning(
                    this,
                    "Connection / OCR error",
                    "Failed to process one or more images.\n\nDetails:\n"
                    + item.text
                );
            }
        }


        doneCount_++;
        updateRow(idx);
        updateProgress(); });

    watcher->setFuture(future);
}

void MainWindow::updateRow(int idx)
{
    if (idx < 0 || idx >= items_.size())
        return;
    const auto &item = items_[idx];

    int row = item.row;
    if (row < 0 || row >= table_->rowCount())
        return;

    QString statusStr;
    switch (item.status)
    {
    case ImageItem::Pending:
        statusStr = "Pending";
        break;
    case ImageItem::InProgress:
        statusStr = "In progress";
        break;
    case ImageItem::Done:
        statusStr = "Done";
        break;
    case ImageItem::Error:
        statusStr = "Error";
        break;
    }

    table_->item(row, 2)->setText(statusStr);
    table_->item(row, 3)->setText(item.text);
}

void MainWindow::updateProgress()
{
    if (totalCount_ == 0)
    {
        progressBar_->setValue(0);
        if (progressLabel_)
        {
            progressLabel_->setText("Progress: 0% (0/0)");
        }
        return;
    }

    int value = static_cast<int>((100.0 * doneCount_) / totalCount_);
    progressBar_->setValue(value);

    if (progressLabel_)
    {
        progressLabel_->setText(
            QString("Progress: %1% (%2/%3 images)")
                .arg(value)
                .arg(doneCount_)
                .arg(totalCount_));
    }
}

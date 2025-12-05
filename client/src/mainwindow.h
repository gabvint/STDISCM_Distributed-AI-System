#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>        
#include <QVector>
#include <memory>

#include "grpc_client.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void onAddImagesClicked();

private:
    struct ImageItem {
        QString batchId;
        QString imageId;
        QString filePath;
        enum Status { Pending, InProgress, Done, Error } status;
        QString text;
        int row = -1;
    };

    void setupUi();
    void startOcrForIndex(int idx);
    void updateRow(int idx);
    void updateProgress();

    QTableWidget* table_;
    QProgressBar* progressBar_;
    QLabel*       progressLabel_;   
    QPushButton*  addImagesButton_;

    std::unique_ptr<GrpcClient> grpcClient_;

    QVector<ImageItem> items_;
    QString currentBatchId_;
    int batchCounter_ = 0;
    int totalCount_ = 0;
    int doneCount_ = 0;
    bool errorShownForCurrentBatch_ = false; 
};

#include "StatisticAnalysis.h"
#include <iostream>
#include <fstream>

StatisticAnalysis::StatisticAnalysis(QWidget* parent)
    : QWidget(parent), mpTableWidget(new QTableWidget())
{
    mUi.setupUi(this);

    setWindowTitle("Statistic Analysis");
    setWindowIcon(QIcon(":/login/res/statisticAnalysis.png"));

    // Connect signals to slots
    connect(mUi.openFile, &QPushButton::clicked, this, &StatisticAnalysis::onOpenFileButtonClicked);
    connect(mUi.saveFile, &QPushButton::clicked, this, &StatisticAnalysis::onSaveFileButtonClicked);
    connect(mUi.begin, &QPushButton::clicked, this, &StatisticAnalysis::beginClicked);

    // Configure mpTableWidget
    mpTableWidget->setVisible(false);  // Initially hide the mpTableWidget
}

StatisticAnalysis::~StatisticAnalysis()
{
    // Clean up mpTableWidget
    mpTableWidget->clearContents();
    mpTableWidget->setRowCount(0);
    delete mpTableWidget;
}

//打开文件
void StatisticAnalysis::onOpenFileButtonClicked() {
    mstrFileName = QFileDialog::getOpenFileName(this, "Open File", "", "Shapefile (*.shp)");

    if (mstrFileName.isEmpty()) QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));

    QFileInfo fileInfo(mstrFileName);
    QString basePath = fileInfo.fileName();
    mUi.lineEditVector->setText(basePath);
}

//保存文件
void StatisticAnalysis::onSaveFileButtonClicked() {
    mstrSaveFileName = QFileDialog::getSaveFileName(this, "Save File", "", "CSV (*.csv)");

    if (mstrSaveFileName.isEmpty()) return;

    mUi.lineEditResult->setText(mstrSaveFileName);

}

//开始分析
void StatisticAnalysis::processFile() {
    if (mstrFileName.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrSaveFileName.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No output file selected."));
        return;
    }

    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();
    CPLErrorReset();
    QString filePath = mstrFileName;
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(filePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == NULL) {
        std::cerr << "GDAL Error: " << CPLGetLastErrorMsg() << std::endl;
        emit progressUpdated(10);

        QFile file(filePath);
        if (!file.exists()) {
            QMessageBox::critical(this, "Error", "The file does not exist: " + filePath);
            return;
        }

        if (!(file.permissions() & QFileDevice::ReadUser)) {
            QMessageBox::critical(this, "Error", "You do not have read permission for the file: " + filePath);
            return;
        }
        emit progressUpdated(25);

        QMessageBox::critical(this, "Error", "Failed to open file: " + filePath);
        return;
    }

    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == NULL) {
        QMessageBox::critical(this, "Error", "Failed to find layer");
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(40);

    // Clear any existing rows and set column count
    mpTableWidget->clear();
    mpTableWidget->setColumnCount(6);

    int row = 0; // Start row index from 0
    OGRFeature* poFeature;
    poLayer->ResetReading();
    emit progressUpdated(50);
    while ((poFeature = poLayer->GetNextFeature()) != NULL) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL) {
            if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
                OGRPolygon* poPolygon = (OGRPolygon*)poGeometry;
                double area = poPolygon->get_Area();
                double perimeter = poPolygon->getExteriorRing()->get_Length();

                mpTableWidget->insertRow(row);
                mpTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // ID starts from 1
                mpTableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(perimeter)));
                mpTableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(area)));
                mpTableWidget->setItem(row, 3, new QTableWidgetItem(""));
                mpTableWidget->setItem(row, 4, new QTableWidgetItem(""));
                mpTableWidget->setItem(row, 5, new QTableWidgetItem(""));
            }
            else if (wkbFlatten(poGeometry->getGeometryType()) == wkbLineString) {
                OGRLineString* poLineString = (OGRLineString*)poGeometry;
                double length = poLineString->get_Length();

                mpTableWidget->insertRow(row);
                mpTableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1))); // ID starts from 1
                mpTableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(length)));
                mpTableWidget->setItem(row, 2, new QTableWidgetItem(""));
                mpTableWidget->setItem(row, 3, new QTableWidgetItem(""));
                mpTableWidget->setItem(row, 4, new QTableWidgetItem(""));
                mpTableWidget->setItem(row, 5, new QTableWidgetItem(""));
            }
            row++;
        }

        OGRFeature::DestroyFeature(poFeature);
        emit progressUpdated(75);
    }

    // Display total number of features
    int featureCount = poLayer->GetFeatureCount();
    mpTableWidget->insertRow(0);
    mpTableWidget->setItem(0, 0, new QTableWidgetItem("ID"));
    mpTableWidget->setItem(0, 1, new QTableWidgetItem("Perimeter/Length"));
    mpTableWidget->setItem(0, 2, new QTableWidgetItem("Area"));
    mpTableWidget->setItem(0, 3, new QTableWidgetItem(""));
    mpTableWidget->setItem(0, 4, new QTableWidgetItem("VectorNumber"));
    mpTableWidget->setItem(0, 5, new QTableWidgetItem(QString::number(featureCount)));

    GDALClose(poDS);

    // Save results to CSV file after processing
    saveResultsToCSV(mstrSaveFileName);

    emit progressUpdated(100);

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Vector Statistic Analysis runs successfully."));
}

//将结果保存成CSV
void StatisticAnalysis::saveResultsToCSV(const QString& filePath) {
    std::ofstream file(filePath.toStdString());

    if (!file.is_open()) {
        QMessageBox::critical(this, "Error", "Failed to open file for writing: " + filePath);
        return;
    }

    int rows = mpTableWidget->rowCount();
    int columns = mpTableWidget->columnCount();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < columns; ++j) {
            QTableWidgetItem* item = mpTableWidget->item(i, j);
            if (item) {
                file << item->text().toStdString();
            }
            if (j < columns - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
    QMessageBox::information(this, "Information", "Results have been successfully saved to CSV file: " + filePath);
}




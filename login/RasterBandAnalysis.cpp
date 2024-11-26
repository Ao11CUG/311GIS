#include "RasterBandAnalysis.h"
#include <QFileDialog>
#include <QPainter>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <QMessageBox>

RasterBandAnalysis::RasterBandAnalysis(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("Raster Band Analysis");
    setWindowIcon(QIcon(":/login/res/rasterBand.png"));

    connect(mUi.openFile, &QPushButton::clicked, this, &RasterBandAnalysis::openFile);
    connect(mUi.begin, &QPushButton::clicked, this, &RasterBandAnalysis::beginClicked);
    connect(mUi.outputRasterButton1, &QPushButton::clicked, this, &RasterBandAnalysis::saveTrueColor);
    connect(mUi.outputRasterButton2, &QPushButton::clicked, this, &RasterBandAnalysis::saveFalseColor);
    connect(mUi.outputRasterButton3, &QPushButton::clicked, this, &RasterBandAnalysis::saveGrayHistogram);
    connect(mUi.outputRasterButton4, &QPushButton::clicked, this, &RasterBandAnalysis::saveEnhancedHistogram);
}

RasterBandAnalysis::~RasterBandAnalysis()
{}

//打开文件
void RasterBandAnalysis::openFile() {
    mstrRasterFilePath = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("TIFF (*.tif)"));

    if (mstrRasterFilePath.isEmpty())
        return;

    QFileInfo fileInfo(mstrRasterFilePath);
    QString basePath = fileInfo.fileName();
    mUi.lineEditGrid->setText(basePath);
}

//保存真彩色结果
void RasterBandAnalysis::saveTrueColor() {
    mstrTrueColorPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrTrueColorPath.isEmpty()) return;

    mUi.lineEditOutputRaster1->setText(mstrTrueColorPath);
}

//保存假彩色结果
void RasterBandAnalysis::saveFalseColor() {
    mstrFalseColorPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrFalseColorPath.isEmpty()) return;

    mUi.lineEditOutputRaster2->setText(mstrFalseColorPath);
}

//保存灰度直方图结果
void RasterBandAnalysis::saveGrayHistogram() {
    mstrGrayHistogramPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrGrayHistogramPath.isEmpty()) return;

    mUi.lineEditOutputRaster3->setText(mstrGrayHistogramPath);
}

//保存直方图均衡化结果
void RasterBandAnalysis::saveEnhancedHistogram() {
    mstrEnhancedHistogramPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrEnhancedHistogramPath.isEmpty()) return;

    mUi.lineEditOutputRaster4->setText(mstrEnhancedHistogramPath);
}

//多线程
void RasterBandAnalysis::processRasterData() {
    if (mstrRasterFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrTrueColorPath.isEmpty() && mstrFalseColorPath.isEmpty() && mstrGrayHistogramPath.isEmpty() && mstrEnhancedHistogramPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No any output file selected."));
        return;
    }

    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    emit progressUpdated(10);

    std::vector<std::thread> threads;

    GDALDataset* dataset = (GDALDataset*)GDALOpen(mstrRasterFilePath.toStdString().c_str(), GA_ReadOnly);
    if (dataset == nullptr) {
        return; // 打开失败，直接返回
    }

    emit progressUpdated(20);

    // 保存真彩色显示
    if (!mstrTrueColorPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrTrueColorPath, 1, 2, 3);
            });
    }

    emit progressUpdated(30);

    // 保存假彩色显示
    if (!mstrFalseColorPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrFalseColorPath, 1, 3, 2, false, false, true);
            });
    }

    emit progressUpdated(40);

    // 保存灰度直方图
    if (!mstrGrayHistogramPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrGrayHistogramPath, 1, 1, 1, true);
            });
    }

    emit progressUpdated(50);

    // 保存直方图均衡化后的图像
    if (!mstrEnhancedHistogramPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrEnhancedHistogramPath, 1, 1, 1, false, true);
            });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    emit progressUpdated(80);

    GDALClose(dataset);

    emit progressUpdated(100);

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Raster Band Analysis runs successfully."));
}

//转换为Tiff保存
void RasterBandAnalysis::saveRasterAsTiff(GDALDataset* inputDataset, const QString& outputFilePath, int bandRed, int bandGreen, int bandBlue, bool isGray, bool isEqualized, bool isFalseColor) {
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) {
        qDebug() << "GTiff driver not available.";
        return;
    }

    int width = inputDataset->GetRasterXSize();
    int height = inputDataset->GetRasterYSize();

    // 判断输出波段数
    int numBands = (isGray || isEqualized) ? 1 : (isFalseColor ? 3 : 3);

    // 创建输出数据集
    GDALDataset* outputDataset = driver->Create(outputFilePath.toStdString().c_str(), width, height, numBands, GDT_Byte, nullptr);
    if (outputDataset == nullptr) {
        qDebug() << "Failed to create output dataset.";
        return;
    }

    GDALRasterBand* redBand = inputDataset->GetRasterBand(bandRed);
    GDALRasterBand* greenBand = inputDataset->GetRasterBand(bandGreen);
    GDALRasterBand* blueBand = inputDataset->GetRasterBand(bandBlue);

    GDALRasterBand* outBand1 = outputDataset->GetRasterBand(1);
    GDALRasterBand* outBand2 = numBands > 1 ? outputDataset->GetRasterBand(2) : nullptr;
    GDALRasterBand* outBand3 = numBands > 2 ? outputDataset->GetRasterBand(3) : nullptr;

    // 根据需要处理数据
    if (isGray || isEqualized) {
        processSingleBand(redBand, outBand1, width, height, isEqualized); // 将红色波段处理为灰度图
        if (outBand2) outBand2->RasterIO(GF_Write, 0, 0, width, height, nullptr, width, height, GDT_Byte, 0, 0);
        if (outBand3) outBand3->RasterIO(GF_Write, 0, 0, width, height, nullptr, width, height, GDT_Byte, 0, 0);
    }
    else if (isFalseColor) {
        // 假彩色处理
        processFalseColorBands(redBand, greenBand, blueBand, outBand1, outBand2, outBand3, width, height);
    }
    else {
        // 否则处理RGB三个波段
        processRGBBands(redBand, greenBand, blueBand, outBand1, outBand2, outBand3, width, height);
    }

    // 设置地理信息
    double geoTransform[6];
    inputDataset->GetGeoTransform(geoTransform);
    outputDataset->SetGeoTransform(geoTransform);
    outputDataset->SetProjection(inputDataset->GetProjectionRef());

    GDALClose(outputDataset);
}

//处理RGB波段
void RasterBandAnalysis::processRGBBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height) {
    unsigned char* redBuffer = new unsigned char[width * height];
    unsigned char* greenBuffer = new unsigned char[width * height];
    unsigned char* blueBuffer = new unsigned char[width * height];

    // 读取波段数据
    redBand->RasterIO(GF_Read, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    greenBand->RasterIO(GF_Read, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    blueBand->RasterIO(GF_Read, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    // 写入输出波段数据
    outRedBand->RasterIO(GF_Write, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    outGreenBand->RasterIO(GF_Write, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    outBlueBand->RasterIO(GF_Write, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] redBuffer;
    delete[] greenBuffer;
    delete[] blueBuffer;
}

//处理假彩色波段
void RasterBandAnalysis::processFalseColorBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height) {
    unsigned char* redBuffer = new unsigned char[width * height];
    unsigned char* greenBuffer = new unsigned char[width * height];
    unsigned char* blueBuffer = new unsigned char[width * height];

    // 读取波段数据
    redBand->RasterIO(GF_Read, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    greenBand->RasterIO(GF_Read, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    blueBand->RasterIO(GF_Read, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    // 假彩色处理
    // 例如，将红色波段作为绿色，将绿色波段作为红色
    outRedBand->RasterIO(GF_Write, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    outGreenBand->RasterIO(GF_Write, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    outBlueBand->RasterIO(GF_Write, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] redBuffer;
    delete[] greenBuffer;
    delete[] blueBuffer;
}

//处理单个波段
void RasterBandAnalysis::processSingleBand(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height, bool isEqualized) {
    if (isEqualized) {
        // 先进行直方图均衡化处理
        performHistogramEqualization(inBand, outBand, width, height);
    }
    else {
        // 否则只进行灰度处理
        performGrayscaleProcessing(inBand, outBand, width, height);
    }
}

//灰度直方图
void RasterBandAnalysis::performGrayscaleProcessing(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height) {
    unsigned char* buffer = new unsigned char[width * height];

    inBand->RasterIO(GF_Read, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    // 将处理后的数据写入输出波段
    outBand->RasterIO(GF_Write, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] buffer;
}

//直方图均衡化
void RasterBandAnalysis::performHistogramEqualization(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height) {
    unsigned char* buffer = new unsigned char[width * height];

    inBand->RasterIO(GF_Read, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0); // 读取完毕，更新进度

    // 直方图均衡化处理
    int histogram[256] = { 0 };

    // 计算直方图
    for (int i = 0; i < width * height; ++i) {
        int value = buffer[i];
        histogram[value]++;
    }

    // 计算累计直方图
    int cumulativeHistogram[256] = { 0 };
    cumulativeHistogram[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cumulativeHistogram[i] = cumulativeHistogram[i - 1] + histogram[i];
    }

    // 应用直方图均衡化
    for (int i = 0; i < width * height; ++i) {
        int value = buffer[i];
        buffer[i] = (cumulativeHistogram[value] * 255) / (width * height);
    }

    // 将处理后的数据写入输出波段
    outBand->RasterIO(GF_Write, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    delete[] buffer;
}

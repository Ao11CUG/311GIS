#include "RasterNeighborhoodStatistics.h"
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <fstream>
#include <QFileDialog>
#include <iostream>
#include <vector>
#include <limits>
#include <QApplication>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QFuture>
#include <QVector>
#include <chrono>
#include <QMessageBox>

RasterNeighborhoodStatistics::RasterNeighborhoodStatistics(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("Raster Neighborhood Statistics Analysis");
    setWindowIcon(QIcon(":/login/res/domainAnalysis.png"));

    connect(mUi.openFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setInputRasterPath);

    connect(mUi.saveCSVFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setOutputCSVPath);
    connect(mUi.saveMaxFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setOutputMaxRasterPath);
    connect(mUi.saveMinFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setOutputMinRasterPath);
    connect(mUi.saveMeanFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setOutputMeanRasterPath);
    connect(mUi.saveWholeCSVFile, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::setOutputWholeCSVPath);
    connect(mUi.begin, &QPushButton::clicked, this, &RasterNeighborhoodStatistics::beginClicked);
}

RasterNeighborhoodStatistics::~RasterNeighborhoodStatistics() {

}

//打开输入文件
void RasterNeighborhoodStatistics::setInputRasterPath()
{
    mstrInputRasterPath = QFileDialog::getOpenFileName(nullptr, "Open File", "", "TIFF (*.tif)");

    qDebug() << "Input Raster Path: " << mstrInputRasterPath;

    if (mstrInputRasterPath.isEmpty()) return;

    else {
        QFileInfo fileInfo(mstrInputRasterPath);
        QString baseName = fileInfo.fileName();
        mUi.lineEditGrid->setText(baseName);
    }

}

//保存输出单个栅格统计CSV
void RasterNeighborhoodStatistics::setOutputCSVPath()
{
    mstrOutputCSVPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "CSV (*.csv)");

    if (mstrOutputCSVPath.isEmpty()) return;
    else {
        mUi.lineEditCSVResult->setText(mstrOutputCSVPath);
    }
}

//保存根据波段统计CSV
void RasterNeighborhoodStatistics::setOutputWholeCSVPath()
{
    mstrOutputWholeCSVPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "CSV (*.csv)");

    if (mstrOutputWholeCSVPath.isEmpty()) return;
    else {
        mUi.lineEditWholeCSVResult->setText(mstrOutputWholeCSVPath);
    }
}

//保存最大值栅格
void RasterNeighborhoodStatistics::setOutputMaxRasterPath()
{
    mstrOutputMaxRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMaxRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMaxResult->setText(mstrOutputMaxRasterPath);
    }
}

//保存最小值栅格
void RasterNeighborhoodStatistics::setOutputMinRasterPath()
{
    mstrOutputMinRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMinRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMinResult->setText(mstrOutputMinRasterPath);
    }
}

//保存均值栅格
void RasterNeighborhoodStatistics::setOutputMeanRasterPath()
{
    mstrOutputMeanRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMeanRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMeanResult->setText(mstrOutputMeanRasterPath);
    }
}

// 计算小方块的统计值（最大值、最小值、平均值）
void RasterNeighborhoodStatistics::calculateBlockStatistics(int i, int j, int blockSizeX, int blockSizeY, uint8_t* buffer, int nXSize, int nYSize, double& minValue, double& maxValue, double& meanValue)
{
    double sum = 0.0;
    minValue = std::numeric_limits<double>::max();
    maxValue = std::numeric_limits<double>::min();
    int count = 0;

    for (int y = i; y < std::min(i + blockSizeY, nYSize); y++)
    {
        for (int x = j; x < std::min(j + blockSizeX, nXSize); x++)
        {
            int index = y * nXSize + x;
            // 正确地将 8 位无符号整型转换为 double 类型进行计算
            uint8_t value = buffer[index];
            double convertedValue = static_cast<double>(value);
            if (!std::isnan(convertedValue))
            {
                sum += convertedValue;
                minValue = std::min(minValue, convertedValue);
                maxValue = std::max(maxValue, convertedValue);
                count++;
            }
        }
    }

    if (count > 0)
    {
        meanValue = sum / count;
    }
    else
    {
        meanValue = NAN;
    }
}

// 生成统计值栅格（最大值、最小值、平均值）
void RasterNeighborhoodStatistics::generateStatisticsRaster(int blockSizeX, int blockSizeY, uint8_t* buffer, int nXSize, int nYSize, std::vector<double>& maxRaster, std::vector<double>& minRaster, std::vector<double>& meanRaster)
{
    for (int i = 0; i < nYSize; i += blockSizeY)
    {
        for (int j = 0; j < nXSize; j += blockSizeX)
        {
            double minValue, maxValue, meanValue;
            calculateBlockStatistics(i, j, blockSizeX, blockSizeY, buffer, nXSize, nYSize, minValue, maxValue, meanValue);
            for (int y = i; y < std::min(i + blockSizeY, nYSize); y++)
            {
                for (int x = j; x < std::min(j + blockSizeX, nXSize); x++)
                {
                    int index = y * nXSize + x;
                    maxRaster[index] = maxValue;
                    minRaster[index] = minValue;
                    meanRaster[index] = meanValue;
                }
            }
        }
    }
}

// 将统计值输出到 CSV 文件（原始的单个像素统计值 CSV）
void RasterNeighborhoodStatistics::outputStatisticsToCSV(const std::vector<double>& maxRaster, const std::vector<double>& minRaster, const std::vector<double>& meanRaster, int nXSize, int nYSize)
{
    if (!mstrOutputCSVPath.isEmpty())
    {
        std::ofstream outputFile(mstrOutputCSVPath.toStdString());
        outputFile << "X,Y,Max,Min,Mean" << std::endl;
        for (int i = 0; i < nYSize; i++)
        {
            for (int j = 0; j < nXSize; j++)
            {
                int index = i * nXSize + j;
                outputFile << j << "," << i << "," << maxRaster[index] << "," << minRaster[index] << "," << meanRaster[index] << std::endl;
            }
        }
        outputFile.close();
    }
}

// 提取创建并设置栅格文件的函数
GDALDataset* RasterNeighborhoodStatistics::createAndConfigureRaster(const std::string& outputPath, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY)
{
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (poDriver == nullptr)
    {
        qDebug() << "Could not get GDAL driver for GTiff.";
        throw std::runtime_error("Could not get GDAL driver for GTiff.");
    }

    // 将数据类型改为与原栅格一致
    GDALDataset* poOutputDS = poDriver->Create(outputPath.c_str(), nXSize, nYSize, bandCount, GDT_Byte, nullptr);
    if (poOutputDS == nullptr)
    {
        qDebug() << "Could not create output raster dataset.";
        throw std::runtime_error("Could not create output raster dataset.");
    }

    // 设置像元大小
    double adfGeoTransform[6] = { 0, pixelSizeX, 0, 0, 0, pixelSizeY };
    poOutputDS->SetGeoTransform(adfGeoTransform);

    return poOutputDS;
}

// 生成最大值栅格文件
void RasterNeighborhoodStatistics::generateMaxRasterFile(const std::vector<double>& maxRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY)
{
    if (!mstrOutputMaxRasterPath.isEmpty())
    {
        GDALDataset* poOutputDS = createAndConfigureRaster(mstrOutputMaxRasterPath.toStdString(), nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);

        for (int band = 1; band <= bandCount; band++)
        {
            GDALRasterBand* poOutputBand = poOutputDS->GetRasterBand(band);
            for (int i = 0; i < nYSize; i++)
            {
                for (int j = 0; j < nXSize; j++)
                {
                    int index = i * nXSize + j;
                    // 将 double 类型的最大值转换为 8 位无符号整型
                    double maxValue = maxRaster[index];
                    uint8_t valueToSet = static_cast<uint8_t>(maxValue);
                    int result = poOutputBand->RasterIO(GF_Write, j, i, 1, 1, &valueToSet, 1, 1, GDT_Byte, 0, 0);
                    if (result != CE_None)
                    {
                        qDebug() << "Error writing data to maximum raster file for band " << band << ", pixel (" << j << ", " << i << ")";
                    }
                }
            }
        }

        GDALClose(poOutputDS);
    }
}

// 生成最小值栅格文件
void RasterNeighborhoodStatistics::generateMinRasterFile(const std::vector<double>& minRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY)
{
    if (!mstrOutputMinRasterPath.isEmpty())
    {
        GDALDataset* poOutputDS = createAndConfigureRaster(mstrOutputMinRasterPath.toStdString(), nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);

        for (int band = 1; band <= bandCount; band++)
        {
            GDALRasterBand* poOutputBand = poOutputDS->GetRasterBand(band);
            for (int i = 0; i < nYSize; i++)
            {
                for (int j = 0; j < nXSize; j++)
                {
                    int index = i * nXSize + j;
                    // 将 double 类型的最小值转换为 8 位无符号整型
                    double minValue = minRaster[index];
                    uint8_t valueToSet = static_cast<uint8_t>(minValue);
                    int result = poOutputBand->RasterIO(GF_Write, j, i, 1, 1, &valueToSet, 1, 1, GDT_Byte, 0, 0);
                    if (result != CE_None)
                    {
                        qDebug() << "Error writing data to minimum raster file for band " << band << ", pixel (" << j << ", " << i << ")";
                    }
                }
            }
        }

        GDALClose(poOutputDS);
    }
}

// 生成平均值栅格文件（修改后的版本，具有正确的波段数和像元大小）
void RasterNeighborhoodStatistics::generateMeanRasterFile(const std::vector<double>& meanRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY)
{
    if (!mstrOutputMeanRasterPath.isEmpty())
    {
        GDALDataset* poOutputDS = createAndConfigureRaster(mstrOutputMeanRasterPath.toStdString(), nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);

        for (int band = 1; band <= bandCount; band++)
        {
            GDALRasterBand* poOutputBand = poOutputDS->GetRasterBand(band);
            for (int i = 0; i < nYSize; i++)
            {
                for (int j = 0; j < nXSize; j++)
                {
                    int index = i * nXSize + j;
                    // 将 double 类型的平均值转换为 8 位无符号整型
                    double meanValue = meanRaster[index];
                    uint8_t valueToSet = static_cast<uint8_t>(meanValue);
                    int result = poOutputBand->RasterIO(GF_Write, j, i, 1, 1, &valueToSet, 1, 1, GDT_Byte, 0, 0);
                    if (result != CE_None)
                    {
                        qDebug() << "Error writing data to mean raster file for band " << band << ", pixel (" << j << ", " << i << ")";
                    }
                }
            }
        }

        GDALClose(poOutputDS);
    }
}

//用于将每个波段的统计值输出到 CSV 文件
void RasterNeighborhoodStatistics::outputBandStatisticsToCSV()
{
    if (!mstrOutputWholeCSVPath.isEmpty())
    {
        std::ofstream outputFile(mstrOutputWholeCSVPath.toStdString());
        outputFile << "Band,Max,Min,Mean" << std::endl;

        GDALAllRegister();
        // 打开输入栅格数据集
        GDALDataset* poInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
        if (poInputDS == 0)
        {
            qDebug() << "Failed to open input raster dataset.";
            return;
        }

        // 获取原栅格的波段数
        int bandCount = poInputDS->GetRasterCount();

        for (int band = 1; band <= bandCount; band++)
        {
            GDALRasterBand* poInputBand = poInputDS->GetRasterBand(band);
            double minValue = std::numeric_limits<double>::max();
            double maxValue = std::numeric_limits<double>::min();
            double sum = 0.0;
            int count = 0;

            // 获取栅格的尺寸
            int nXSize = poInputBand->GetXSize();
            int nYSize = poInputBand->GetYSize();
            for (int i = 0; i < nYSize; i++)
            {
                for (int j = 0; j < nXSize; j++)
                {
                    // 读取像素值
                    double value;
                    int result = poInputBand->RasterIO(GF_Read, j, i, 1, 1, &value, 1, 1, GDT_Float64, 0, 0);
                    if (result == CE_None)
                    {
                        if (!std::isnan(value))
                        {
                            sum += value;
                            minValue = std::min(minValue, value);
                            maxValue = std::max(maxValue, value);
                            count++;
                        }
                    }
                }
            }

            double meanValue;
            if (count > 0)
            {
                meanValue = sum / count;
            }
            else
            {
                meanValue = NAN;
            }

            outputFile << band << "," << maxValue << "," << minValue << "," << meanValue << std::endl;
        }

        outputFile.close();
        GDALClose(poInputDS);
    }
    else
    {
        std::cerr << "Output CSV path for band statistics is empty. Skipping writing to CSV." << std::endl;
    }
}

//多线程处理分析
void RasterNeighborhoodStatistics::neighborhoodStatistics()
{
    if (mstrInputRasterPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrOutputCSVPath.isEmpty() && mstrOutputMaxRasterPath.isEmpty() && mstrOutputMeanRasterPath.isEmpty() && mstrOutputMinRasterPath.isEmpty() && mstrOutputWholeCSVPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No any output file selected."));
        return;
    }

    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    emit progressUpdated(10);

    // 打开输入栅格数据集
    GDALDataset* poInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (poInputDS == nullptr)
    {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    emit progressUpdated(20);

    // 获取输入栅格波段
    GDALRasterBand* poInputBand = poInputDS->GetRasterBand(1);

    emit progressUpdated(30);

    // 获取原栅格的波段数
    int bandCount = poInputBand->GetDataset()->GetRasterCount();

    // 获取栅格的尺寸
    int nXSize = poInputBand->GetXSize();
    int nYSize = poInputBand->GetYSize();

    // 获取地理变换参数
    double adfGeoTransform[6];
    poInputDS->GetGeoTransform(adfGeoTransform);

    // 获取像元大小
    double pixelSizeX = std::abs(adfGeoTransform[1]);
    double pixelSizeY = std::abs(adfGeoTransform[5]);

    emit progressUpdated(40);

    // 计算小方块在 X 和 Y 方向的大小
    int blockSizeX = nXSize / 1000;
    int blockSizeY = nYSize / 1000;

    // 分配缓冲区用于存储栅格数据
    uint8_t* buffer = new uint8_t[nXSize * nYSize];

    // 读取栅格数据到缓冲区
    poInputBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, buffer, nXSize, nYSize, GDT_Byte, 0, 0);

    std::vector<double> maxRaster(nXSize * nYSize);
    std::vector<double> minRaster(nXSize * nYSize);
    std::vector<double> meanRaster(nXSize * nYSize);

    emit progressUpdated(50);

    // 创建线程来生成统计值栅格
    std::thread statThread([this, blockSizeX, blockSizeY, buffer, nXSize, nYSize, &maxRaster, &minRaster, &meanRaster]() {
        generateStatisticsRaster(blockSizeX, blockSizeY, buffer, nXSize, nYSize, maxRaster, minRaster, meanRaster);
        });

    // 等待统计线程完成
    if (statThread.joinable()) {
        statThread.join();
    }

    // 立即释放 buffer 内存
    delete[] buffer;

    // 创建线程处理生成和保存栅格文件
    std::vector<std::thread> fileThreads;

    // 使用互斥锁保护输出到 CSV 的过程
    {
        std::lock_guard<std::mutex> lock(mMtx);
        outputBandStatisticsToCSV();
    }

    // 生成最大值栅格文件
    fileThreads.emplace_back([this, maxRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMaxRasterFile(maxRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    // 生成最小值栅格文件
    fileThreads.emplace_back([this, minRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMinRasterFile(minRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    // 生成均值栅格文件
    fileThreads.emplace_back([this, meanRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMeanRasterFile(meanRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    emit progressUpdated(60);

    // 等待所有文件生成线程完成
    for (auto& thread : fileThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    emit progressUpdated(80);

    // 输出统计信息到 CSV
    {
        std::lock_guard<std::mutex> lock(mMtx);
        outputStatisticsToCSV(maxRaster, minRaster, meanRaster, nXSize, nYSize);
    }

    GDALClose(poInputDS);

    emit progressUpdated(100);

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Raster Neighborhood Statistics Analysis runs successfully."));
}


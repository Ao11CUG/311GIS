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

//�������ļ�
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

//�����������դ��ͳ��CSV
void RasterNeighborhoodStatistics::setOutputCSVPath()
{
    mstrOutputCSVPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "CSV (*.csv)");

    if (mstrOutputCSVPath.isEmpty()) return;
    else {
        mUi.lineEditCSVResult->setText(mstrOutputCSVPath);
    }
}

//������ݲ���ͳ��CSV
void RasterNeighborhoodStatistics::setOutputWholeCSVPath()
{
    mstrOutputWholeCSVPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "CSV (*.csv)");

    if (mstrOutputWholeCSVPath.isEmpty()) return;
    else {
        mUi.lineEditWholeCSVResult->setText(mstrOutputWholeCSVPath);
    }
}

//�������ֵդ��
void RasterNeighborhoodStatistics::setOutputMaxRasterPath()
{
    mstrOutputMaxRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMaxRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMaxResult->setText(mstrOutputMaxRasterPath);
    }
}

//������Сֵդ��
void RasterNeighborhoodStatistics::setOutputMinRasterPath()
{
    mstrOutputMinRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMinRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMinResult->setText(mstrOutputMinRasterPath);
    }
}

//�����ֵդ��
void RasterNeighborhoodStatistics::setOutputMeanRasterPath()
{
    mstrOutputMeanRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputMeanRasterPath.isEmpty()) return;
    else {
        mUi.lineEditMeanResult->setText(mstrOutputMeanRasterPath);
    }
}

// ����С�����ͳ��ֵ�����ֵ����Сֵ��ƽ��ֵ��
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
            // ��ȷ�ؽ� 8 λ�޷�������ת��Ϊ double ���ͽ��м���
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

// ����ͳ��ֵդ�����ֵ����Сֵ��ƽ��ֵ��
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

// ��ͳ��ֵ����� CSV �ļ���ԭʼ�ĵ�������ͳ��ֵ CSV��
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

// ��ȡ����������դ���ļ��ĺ���
GDALDataset* RasterNeighborhoodStatistics::createAndConfigureRaster(const std::string& outputPath, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY)
{
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (poDriver == nullptr)
    {
        qDebug() << "Could not get GDAL driver for GTiff.";
        throw std::runtime_error("Could not get GDAL driver for GTiff.");
    }

    // ���������͸�Ϊ��ԭդ��һ��
    GDALDataset* poOutputDS = poDriver->Create(outputPath.c_str(), nXSize, nYSize, bandCount, GDT_Byte, nullptr);
    if (poOutputDS == nullptr)
    {
        qDebug() << "Could not create output raster dataset.";
        throw std::runtime_error("Could not create output raster dataset.");
    }

    // ������Ԫ��С
    double adfGeoTransform[6] = { 0, pixelSizeX, 0, 0, 0, pixelSizeY };
    poOutputDS->SetGeoTransform(adfGeoTransform);

    return poOutputDS;
}

// �������ֵդ���ļ�
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
                    // �� double ���͵����ֵת��Ϊ 8 λ�޷�������
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

// ������Сֵդ���ļ�
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
                    // �� double ���͵���Сֵת��Ϊ 8 λ�޷�������
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

// ����ƽ��ֵդ���ļ����޸ĺ�İ汾��������ȷ�Ĳ���������Ԫ��С��
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
                    // �� double ���͵�ƽ��ֵת��Ϊ 8 λ�޷�������
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

//���ڽ�ÿ�����ε�ͳ��ֵ����� CSV �ļ�
void RasterNeighborhoodStatistics::outputBandStatisticsToCSV()
{
    if (!mstrOutputWholeCSVPath.isEmpty())
    {
        std::ofstream outputFile(mstrOutputWholeCSVPath.toStdString());
        outputFile << "Band,Max,Min,Mean" << std::endl;

        GDALAllRegister();
        // ������դ�����ݼ�
        GDALDataset* poInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
        if (poInputDS == 0)
        {
            qDebug() << "Failed to open input raster dataset.";
            return;
        }

        // ��ȡԭդ��Ĳ�����
        int bandCount = poInputDS->GetRasterCount();

        for (int band = 1; band <= bandCount; band++)
        {
            GDALRasterBand* poInputBand = poInputDS->GetRasterBand(band);
            double minValue = std::numeric_limits<double>::max();
            double maxValue = std::numeric_limits<double>::min();
            double sum = 0.0;
            int count = 0;

            // ��ȡդ��ĳߴ�
            int nXSize = poInputBand->GetXSize();
            int nYSize = poInputBand->GetYSize();
            for (int i = 0; i < nYSize; i++)
            {
                for (int j = 0; j < nXSize; j++)
                {
                    // ��ȡ����ֵ
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

//���̴߳������
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

    // ��¼��ʼʱ��
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    emit progressUpdated(10);

    // ������դ�����ݼ�
    GDALDataset* poInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (poInputDS == nullptr)
    {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    emit progressUpdated(20);

    // ��ȡ����դ�񲨶�
    GDALRasterBand* poInputBand = poInputDS->GetRasterBand(1);

    emit progressUpdated(30);

    // ��ȡԭդ��Ĳ�����
    int bandCount = poInputBand->GetDataset()->GetRasterCount();

    // ��ȡդ��ĳߴ�
    int nXSize = poInputBand->GetXSize();
    int nYSize = poInputBand->GetYSize();

    // ��ȡ����任����
    double adfGeoTransform[6];
    poInputDS->GetGeoTransform(adfGeoTransform);

    // ��ȡ��Ԫ��С
    double pixelSizeX = std::abs(adfGeoTransform[1]);
    double pixelSizeY = std::abs(adfGeoTransform[5]);

    emit progressUpdated(40);

    // ����С������ X �� Y ����Ĵ�С
    int blockSizeX = nXSize / 1000;
    int blockSizeY = nYSize / 1000;

    // ���仺�������ڴ洢դ������
    uint8_t* buffer = new uint8_t[nXSize * nYSize];

    // ��ȡդ�����ݵ�������
    poInputBand->RasterIO(GF_Read, 0, 0, nXSize, nYSize, buffer, nXSize, nYSize, GDT_Byte, 0, 0);

    std::vector<double> maxRaster(nXSize * nYSize);
    std::vector<double> minRaster(nXSize * nYSize);
    std::vector<double> meanRaster(nXSize * nYSize);

    emit progressUpdated(50);

    // �����߳�������ͳ��ֵդ��
    std::thread statThread([this, blockSizeX, blockSizeY, buffer, nXSize, nYSize, &maxRaster, &minRaster, &meanRaster]() {
        generateStatisticsRaster(blockSizeX, blockSizeY, buffer, nXSize, nYSize, maxRaster, minRaster, meanRaster);
        });

    // �ȴ�ͳ���߳����
    if (statThread.joinable()) {
        statThread.join();
    }

    // �����ͷ� buffer �ڴ�
    delete[] buffer;

    // �����̴߳������ɺͱ���դ���ļ�
    std::vector<std::thread> fileThreads;

    // ʹ�û�������������� CSV �Ĺ���
    {
        std::lock_guard<std::mutex> lock(mMtx);
        outputBandStatisticsToCSV();
    }

    // �������ֵդ���ļ�
    fileThreads.emplace_back([this, maxRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMaxRasterFile(maxRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    // ������Сֵդ���ļ�
    fileThreads.emplace_back([this, minRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMinRasterFile(minRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    // ���ɾ�ֵդ���ļ�
    fileThreads.emplace_back([this, meanRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY]() {
        generateMeanRasterFile(meanRaster, nXSize, nYSize, bandCount, pixelSizeX, pixelSizeY);
        });

    emit progressUpdated(60);

    // �ȴ������ļ������߳����
    for (auto& thread : fileThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    emit progressUpdated(80);

    // ���ͳ����Ϣ�� CSV
    {
        std::lock_guard<std::mutex> lock(mMtx);
        outputStatisticsToCSV(maxRaster, minRaster, meanRaster, nXSize, nYSize);
    }

    GDALClose(poInputDS);

    emit progressUpdated(100);

    // ��¼����ʱ��
    auto endTime = std::chrono::high_resolution_clock::now();
    // ���㾭����ʱ�䣨��λ���룩
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // ���ƾ��ȵ� 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // ���������ڵĲۺ������½���
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Raster Neighborhood Statistics Analysis runs successfully."));
}


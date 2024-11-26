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

//���ļ�
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

//�������ɫ���
void RasterBandAnalysis::saveTrueColor() {
    mstrTrueColorPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrTrueColorPath.isEmpty()) return;

    mUi.lineEditOutputRaster1->setText(mstrTrueColorPath);
}

//����ٲ�ɫ���
void RasterBandAnalysis::saveFalseColor() {
    mstrFalseColorPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrFalseColorPath.isEmpty()) return;

    mUi.lineEditOutputRaster2->setText(mstrFalseColorPath);
}

//����Ҷ�ֱ��ͼ���
void RasterBandAnalysis::saveGrayHistogram() {
    mstrGrayHistogramPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrGrayHistogramPath.isEmpty()) return;

    mUi.lineEditOutputRaster3->setText(mstrGrayHistogramPath);
}

//����ֱ��ͼ���⻯���
void RasterBandAnalysis::saveEnhancedHistogram() {
    mstrEnhancedHistogramPath = QFileDialog::getSaveFileName(nullptr, tr("Save File"), "", tr("TIFF (*.tif)"));

    if (mstrEnhancedHistogramPath.isEmpty()) return;

    mUi.lineEditOutputRaster4->setText(mstrEnhancedHistogramPath);
}

//���߳�
void RasterBandAnalysis::processRasterData() {
    if (mstrRasterFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrTrueColorPath.isEmpty() && mstrFalseColorPath.isEmpty() && mstrGrayHistogramPath.isEmpty() && mstrEnhancedHistogramPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No any output file selected."));
        return;
    }

    // ��¼��ʼʱ��
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    emit progressUpdated(10);

    std::vector<std::thread> threads;

    GDALDataset* dataset = (GDALDataset*)GDALOpen(mstrRasterFilePath.toStdString().c_str(), GA_ReadOnly);
    if (dataset == nullptr) {
        return; // ��ʧ�ܣ�ֱ�ӷ���
    }

    emit progressUpdated(20);

    // �������ɫ��ʾ
    if (!mstrTrueColorPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrTrueColorPath, 1, 2, 3);
            });
    }

    emit progressUpdated(30);

    // ����ٲ�ɫ��ʾ
    if (!mstrFalseColorPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrFalseColorPath, 1, 3, 2, false, false, true);
            });
    }

    emit progressUpdated(40);

    // ����Ҷ�ֱ��ͼ
    if (!mstrGrayHistogramPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrGrayHistogramPath, 1, 1, 1, true);
            });
    }

    emit progressUpdated(50);

    // ����ֱ��ͼ���⻯���ͼ��
    if (!mstrEnhancedHistogramPath.isEmpty()) {
        threads.emplace_back([&]() {
            std::lock_guard<std::mutex> lock(mMex);
            saveRasterAsTiff(dataset, mstrEnhancedHistogramPath, 1, 1, 1, false, true);
            });
    }

    // �ȴ������߳����
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    emit progressUpdated(80);

    GDALClose(dataset);

    emit progressUpdated(100);

    // ��¼����ʱ��
    auto endTime = std::chrono::high_resolution_clock::now();
    // ���㾭����ʱ�䣨��λ���룩
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // ���ƾ��ȵ� 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // ���������ڵĲۺ������½���
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Raster Band Analysis runs successfully."));
}

//ת��ΪTiff����
void RasterBandAnalysis::saveRasterAsTiff(GDALDataset* inputDataset, const QString& outputFilePath, int bandRed, int bandGreen, int bandBlue, bool isGray, bool isEqualized, bool isFalseColor) {
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (driver == nullptr) {
        qDebug() << "GTiff driver not available.";
        return;
    }

    int width = inputDataset->GetRasterXSize();
    int height = inputDataset->GetRasterYSize();

    // �ж����������
    int numBands = (isGray || isEqualized) ? 1 : (isFalseColor ? 3 : 3);

    // ����������ݼ�
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

    // ������Ҫ��������
    if (isGray || isEqualized) {
        processSingleBand(redBand, outBand1, width, height, isEqualized); // ����ɫ���δ���Ϊ�Ҷ�ͼ
        if (outBand2) outBand2->RasterIO(GF_Write, 0, 0, width, height, nullptr, width, height, GDT_Byte, 0, 0);
        if (outBand3) outBand3->RasterIO(GF_Write, 0, 0, width, height, nullptr, width, height, GDT_Byte, 0, 0);
    }
    else if (isFalseColor) {
        // �ٲ�ɫ����
        processFalseColorBands(redBand, greenBand, blueBand, outBand1, outBand2, outBand3, width, height);
    }
    else {
        // ������RGB��������
        processRGBBands(redBand, greenBand, blueBand, outBand1, outBand2, outBand3, width, height);
    }

    // ���õ�����Ϣ
    double geoTransform[6];
    inputDataset->GetGeoTransform(geoTransform);
    outputDataset->SetGeoTransform(geoTransform);
    outputDataset->SetProjection(inputDataset->GetProjectionRef());

    GDALClose(outputDataset);
}

//����RGB����
void RasterBandAnalysis::processRGBBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height) {
    unsigned char* redBuffer = new unsigned char[width * height];
    unsigned char* greenBuffer = new unsigned char[width * height];
    unsigned char* blueBuffer = new unsigned char[width * height];

    // ��ȡ��������
    redBand->RasterIO(GF_Read, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    greenBand->RasterIO(GF_Read, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    blueBand->RasterIO(GF_Read, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    // д�������������
    outRedBand->RasterIO(GF_Write, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    outGreenBand->RasterIO(GF_Write, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    outBlueBand->RasterIO(GF_Write, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] redBuffer;
    delete[] greenBuffer;
    delete[] blueBuffer;
}

//����ٲ�ɫ����
void RasterBandAnalysis::processFalseColorBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height) {
    unsigned char* redBuffer = new unsigned char[width * height];
    unsigned char* greenBuffer = new unsigned char[width * height];
    unsigned char* blueBuffer = new unsigned char[width * height];

    // ��ȡ��������
    redBand->RasterIO(GF_Read, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    greenBand->RasterIO(GF_Read, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    blueBand->RasterIO(GF_Read, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    // �ٲ�ɫ����
    // ���磬����ɫ������Ϊ��ɫ������ɫ������Ϊ��ɫ
    outRedBand->RasterIO(GF_Write, 0, 0, width, height, greenBuffer, width, height, GDT_Byte, 0, 0);
    outGreenBand->RasterIO(GF_Write, 0, 0, width, height, redBuffer, width, height, GDT_Byte, 0, 0);
    outBlueBand->RasterIO(GF_Write, 0, 0, width, height, blueBuffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] redBuffer;
    delete[] greenBuffer;
    delete[] blueBuffer;
}

//����������
void RasterBandAnalysis::processSingleBand(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height, bool isEqualized) {
    if (isEqualized) {
        // �Ƚ���ֱ��ͼ���⻯����
        performHistogramEqualization(inBand, outBand, width, height);
    }
    else {
        // ����ֻ���лҶȴ���
        performGrayscaleProcessing(inBand, outBand, width, height);
    }
}

//�Ҷ�ֱ��ͼ
void RasterBandAnalysis::performGrayscaleProcessing(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height) {
    unsigned char* buffer = new unsigned char[width * height];

    inBand->RasterIO(GF_Read, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    // ������������д���������
    outBand->RasterIO(GF_Write, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    if (mstrEnhancedHistogramPath.isEmpty()) emit progressUpdated(100);

    delete[] buffer;
}

//ֱ��ͼ���⻯
void RasterBandAnalysis::performHistogramEqualization(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height) {
    unsigned char* buffer = new unsigned char[width * height];

    inBand->RasterIO(GF_Read, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0); // ��ȡ��ϣ����½���

    // ֱ��ͼ���⻯����
    int histogram[256] = { 0 };

    // ����ֱ��ͼ
    for (int i = 0; i < width * height; ++i) {
        int value = buffer[i];
        histogram[value]++;
    }

    // �����ۼ�ֱ��ͼ
    int cumulativeHistogram[256] = { 0 };
    cumulativeHistogram[0] = histogram[0];
    for (int i = 1; i < 256; ++i) {
        cumulativeHistogram[i] = cumulativeHistogram[i - 1] + histogram[i];
    }

    // Ӧ��ֱ��ͼ���⻯
    for (int i = 0; i < width * height; ++i) {
        int value = buffer[i];
        buffer[i] = (cumulativeHistogram[value] * 255) / (width * height);
    }

    // ������������д���������
    outBand->RasterIO(GF_Write, 0, 0, width, height, buffer, width, height, GDT_Byte, 0, 0);

    delete[] buffer;
}

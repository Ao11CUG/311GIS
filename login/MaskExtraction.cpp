#include "MaskExtraction.h"
#include <QDebug>
#include <QFileDialog>
#include <cpl_conv.h>
#include <gdal_alg.h>
#include <ogrsf_frmts.h>
#include <QApplication>
#include <gdalwarper.h>
#include <cpl_error.h>
#include <gdal_priv.h>
#include <chrono>
#include <QMessageBox>

MaskExtraction::MaskExtraction(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("Mask Extraction");
    setWindowIcon(QIcon(":/login/res/maskExtraction.png"));

    connect(mUi.inputRasterButton, &QPushButton::clicked, this, &MaskExtraction::selectInputRaster);
    connect(mUi.inputMaskButton, &QPushButton::clicked, this, &MaskExtraction::selectMask);
    connect(mUi.outputRasterButton, &QPushButton::clicked, this, &MaskExtraction::selectOutputRaster);
    connect(mUi.confirmButton, &QPushButton::clicked, this, &MaskExtraction::beginClicked);
}

MaskExtraction::~MaskExtraction()
{}

// 计算矢量数据与栅格数据的有效交集范围，并添加投影信息
void MaskExtraction::calculateIntersectionAndSetProjection(GDALDataset* mpPoInputDS, OGRLayer* poLayer, std::vector<OGREnvelope>& polygonEnvelopes) {
    if (mpPoInputDS->GetGeoTransform(mdAdfGeoTransform) == CE_None) {
        // 遍历矢量图层中的每个特征
        for (OGRFeature* poFeature = poLayer->GetNextFeature(); poFeature != nullptr; poFeature = poLayer->GetNextFeature()) {
            OGRGeometry* poGeometry = poFeature->GetGeometryRef();
            if (poGeometry != nullptr && poGeometry->IsValid()) {
                if (poGeometry->getGeometryType() == wkbPolygon) {
                    OGRPolygon* poPolygon = (OGRPolygon*)poGeometry;
                    OGREnvelope envelope;
                    poPolygon->getExteriorRing()->getEnvelope(&envelope);
                    polygonEnvelopes.push_back(envelope);
                }
            }
            OGRFeature::DestroyFeature(poFeature);
        }
        emit progressUpdated(40);

        // 设置栅格的投影（现在使用输入栅格自身的投影）
        const char* inputProjection = mpPoInputDS->GetProjectionRef();
        mpPoInputDS->SetProjection(inputProjection);
        emit progressUpdated(45);
    }
}

//矢量作为掩膜
void MaskExtraction::vectorMask(const QString& mstrInputRasterPath, const QString& vectorPath, const QString& mstrOutputRasterPath) {
    // 打开输入栅格数据集
    GDALDataset* mpPoInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (mpPoInputDS == nullptr) {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    // 获取原栅格的波段数量
    int bandCount = mpPoInputDS->GetRasterCount();
    emit progressUpdated(10);

    // 打开矢量数据集
    GDALDataset* poVectorDS = (GDALDataset*)GDALOpenEx(vectorPath.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poVectorDS == nullptr) {
        qDebug() << "Failed to open vector dataset.";
        GDALClose(mpPoInputDS);
        return;
    }

    // 获取矢量数据集的第一个图层
    OGRLayer* poLayer = poVectorDS->GetLayer(0);
    if (poLayer == nullptr) {
        qDebug() << "Failed to get the first layer from vector dataset.";
        GDALClose(mpPoInputDS);
        GDALClose(poVectorDS);
        return;
    }

    // 存储每个多边形的范围
    std::vector<OGREnvelope> polygonEnvelopes;
    emit progressUpdated(20);
    calculateIntersectionAndSetProjection(mpPoInputDS, poLayer, polygonEnvelopes);

    // 处理每个多边形
    std::vector<TileInfo> tileInfos;
    int polygonIndex = 0;
    for (const auto& envelope : polygonEnvelopes) {
        // 计算每个多边形对应的栅格范围
        int tempStartColFeature = static_cast<int>((envelope.MinX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempStartRowFeature = static_cast<int>((envelope.MaxY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);
        int tempEndColFeature = static_cast<int>((envelope.MaxX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempEndRowFeature = static_cast<int>((envelope.MinY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);

        // 记录小块栅格信息
        TileInfo tileInfo;
        tileInfo.x_offset = tempStartColFeature;
        tileInfo.y_offset = tempStartRowFeature;
        tileInfo.width = tempEndColFeature - tempStartColFeature + 1;
        tileInfo.height = tempEndRowFeature - tempStartRowFeature + 1;
        // 这里可以添加存储 tempEndColFeature 和 tempEndRowFeature 的成员变量
        tileInfo.endColFeature = tempEndColFeature;
        tileInfo.endRowFeature = tempEndRowFeature;
        tileInfos.push_back(tileInfo);

        polygonIndex++;
    }

    emit progressUpdated(65);

    // 关闭矢量数据集
    GDALClose(poVectorDS);

    // 拼接小块栅格数据到一个完整的栅格
    stitchTilesFromMemory(tileInfos, polygonEnvelopes, mstrOutputRasterPath, bandCount, mpPoInputDS);

    // 关闭输入栅格数据集
    GDALClose(mpPoInputDS);
}

//栅格作为掩膜
void MaskExtraction::rasterMask(const QString& mstrInputRasterPath, const QString& mstrMaskPath, const QString& mstrOutputRasterPath) {
    // 打开输入栅格数据集
    GDALDataset* mpPoInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (mpPoInputDS == nullptr) {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    // 打开掩膜栅格数据集
    GDALDataset* poMaskDS = (GDALDataset*)GDALOpen(mstrMaskPath.toStdString().c_str(), GA_ReadOnly);
    if (poMaskDS == nullptr) {
        qDebug() << "Failed to open mask raster dataset.";
        GDALClose(mpPoInputDS);
        return;
    }

    // 获取原栅格的波段数量
    int bandCount = mpPoInputDS->GetRasterCount();
    int maskBandCount = poMaskDS->GetRasterCount();

    if (bandCount != maskBandCount) {
        qDebug() << "The number of bands in the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 获取输入栅格和掩膜栅格的地理变换信息
    double adfInputGeoTransform[6];
    if (mpPoInputDS->GetGeoTransform(adfInputGeoTransform) != CE_None) {
        qDebug() << "Failed to get the geotransform for the input raster.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }
    double adfMaskGeoTransform[6];
    if (poMaskDS->GetGeoTransform(adfMaskGeoTransform) != CE_None) {
        qDebug() << "Failed to get the geotransform for the mask raster.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 检查输入栅格和掩膜栅格的地理变换是否相同
    bool isGeoTransformEqual = true;
    for (int i = 0; i < 6; i++) {
        if (adfInputGeoTransform[i] != adfMaskGeoTransform[i]) {
            isGeoTransformEqual = false;
            break;
        }
    }
    if (!isGeoTransformEqual) {
        qDebug() << "The geotransforms of the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 获取输入栅格和掩膜栅格的投影信息
    const char* inputProjection = mpPoInputDS->GetProjectionRef();
    const char* maskProjection = poMaskDS->GetProjectionRef();
    if (strcmp(inputProjection, maskProjection) != 0) {
        qDebug() << "The projections of the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 获取输入栅格和掩膜栅格的宽度和高度
    int inputWidth = mpPoInputDS->GetRasterXSize();
    int inputHeight = mpPoInputDS->GetRasterYSize();
    int maskWidth = poMaskDS->GetRasterXSize();
    int maskHeight = poMaskDS->GetRasterYSize();

    if (inputWidth != maskWidth || inputHeight != maskHeight) {
        qDebug() << "The dimensions of the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 创建目标栅格数据集
    GDALDriver* targetDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (targetDriver == nullptr) {
        qDebug() << "Failed to get GTiff driver for target raster.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }
    GDALDataset* poTargetOutputDS = targetDriver->Create((mstrOutputRasterPath + "_masked.tif").toStdString().c_str(), inputWidth, inputHeight, bandCount, GDT_Byte, nullptr);
    if (poTargetOutputDS == nullptr) {
        qDebug() << "Failed to create target output raster dataset.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // 设置目标栅格的地理变换和投影等信息
    if (poTargetOutputDS->GetGeoTransform(mdAdfGeoTransform) == CE_None) {
        poTargetOutputDS->SetGeoTransform(adfInputGeoTransform);
    }
    poTargetOutputDS->SetProjection(inputProjection);

    // 对每个波段进行掩膜操作
    for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
        GDALRasterBand* inputBand = mpPoInputDS->GetRasterBand(bandIndex + 1);
        GDALRasterBand* maskBand = poMaskDS->GetRasterBand(bandIndex + 1);
        GDALRasterBand* targetBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);

        // 读取输入栅格和掩膜栅格的数据到缓冲区
        GByte* inputData = (GByte*)CPLMalloc(inputWidth * inputHeight * sizeof(GByte));
        GByte* maskData = (GByte*)CPLMalloc(maskWidth * maskHeight * sizeof(GByte));
        int resultReadInput = inputBand->RasterIO(GF_Read, 0, 0, inputWidth, inputHeight, inputData, inputWidth, inputHeight, GDT_Byte, 0, 0);
        int resultReadMask = maskBand->RasterIO(GF_Read, 0, 0, maskWidth, maskHeight, maskData, maskWidth, maskHeight, GDT_Byte, 0, 0);
        if (resultReadInput != CE_None || resultReadMask != CE_None) {
            qDebug() << "Failed to read data from source for band " << bandIndex;
            CPLFree(inputData);
            CPLFree(maskData);
            GDALClose(mpPoInputDS);
            GDALClose(poMaskDS);
            GDALClose(poTargetOutputDS);
            return;
        }

        // 进行掩膜操作
        for (int i = 0; i < inputWidth * inputHeight; i++) {
            if (maskData[i] == 0) {
                inputData[i] = 255; // 设置为白色（假设 255 表示白色）
            }
        }

        // 将处理后的数据写入目标栅格
        int resultWrite = targetBand->RasterIO(GF_Write, 0, 0, inputWidth, inputHeight, inputData, inputWidth, inputHeight, GDT_Byte, 0, 0);
        if (resultWrite != CE_None) {
            qDebug() << "Failed to write data to target for band " << bandIndex;
            CPLFree(inputData);
            CPLFree(maskData);
            GDALClose(mpPoInputDS);
            GDALClose(poMaskDS);
            GDALClose(poTargetOutputDS);
            return;
        }

        // 释放缓冲区内存
        CPLFree(inputData);
        CPLFree(maskData);
    }

    GDALClose(mpPoInputDS);
    GDALClose(poMaskDS);
    GDALClose(poTargetOutputDS);
}

// 计算每个小块在最终拼接图像中的准确偏移量
void MaskExtraction::stitchTilesFromMemory(const std::vector<TileInfo>& tileInfos, const std::vector<OGREnvelope>& polygonEnvelopes, const QString& mstrOutputRasterPath, int bandCount, GDALDataset* mpPoInputDS) {
    std::vector<std::pair<int, int>> tileOffsets;
    for (const auto& envelope : polygonEnvelopes) {
        int tempStartColFeature = static_cast<int>((envelope.MinX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempStartRowFeature = static_cast<int>((envelope.MaxY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);
        tileOffsets.push_back(std::make_pair(tempStartColFeature, tempStartRowFeature));
    }
    emit progressUpdated(70);

    // 确定最终拼接图像的大小
    int maxX = 0;
    int maxY = 0;
    for (const auto& tileInfo : tileInfos) {
        int endX = tileInfo.endColFeature;
        int endY = tileInfo.endRowFeature;
        if (endX > maxX) maxX = endX;
        if (endY > maxY) maxY = endY;
    }
    emit progressUpdated(75);

    GDALDriver* targetDriver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (targetDriver == nullptr) {
        qDebug() << "Failed to get GTiff driver for target raster.";
        return;
    }
    GDALDataset* poTargetOutputDS = targetDriver->Create((mstrOutputRasterPath).toStdString().c_str(), maxX + 1, maxY + 1, bandCount, GDT_Byte, nullptr);
    if (poTargetOutputDS == nullptr) {
        qDebug() << "Failed to create target output raster dataset.";
        return;
    }
    emit progressUpdated(80);

    // 将整个目标栅格初始化为白色背景
    for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
        GDALRasterBand* dstBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);
        dstBand->Fill(255);
    }

    // 设置目标栅格的地理变换和投影等信息
    if (poTargetOutputDS->GetGeoTransform(mdAdfGeoTransform) == CE_None) {
        poTargetOutputDS->SetGeoTransform(mdAdfGeoTransform);
    }
    poTargetOutputDS->SetProjection("EPSG:4326");

    int tileIndex = 0;
    for (const auto& tileInfo : tileInfos) {
        const std::pair<int, int>& offset = tileOffsets[tileIndex];
        int tempStartColFeature = tileInfo.x_offset;
        int tempStartRowFeature = tileInfo.y_offset;
        int tileWidth = tileInfo.width;
        int tileHeight = tileInfo.height;

        // 这里需要根据实际情况修改，假设每个波段的数据缓冲区大小与 tileWidth 和 tileHeight 相关
        std::vector<GByte*> dataBuffers(bandCount, nullptr);
        for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
            // 分配缓冲区内存
            dataBuffers[bandIndex] = (GByte*)CPLMalloc(tileWidth * tileHeight * sizeof(GByte));

            // 从原栅格对应的小块中读取数据到缓冲区
            GDALRasterBand* srcBand = mpPoInputDS->GetRasterBand(bandIndex + 1);
            int resultRead = srcBand->RasterIO(GF_Read, tempStartColFeature, tempStartRowFeature, tileWidth, tileHeight, dataBuffers[bandIndex], tileWidth, tileHeight, GDT_Byte, 0, 0);
            if (resultRead != CE_None) {
                qDebug() << "Failed to read data from source for band " << bandIndex << " of tile " << tileIndex;
            }

            GDALRasterBand* dstBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);
            // 修改这里，只写入应该有数据的部分，而不覆盖白色背景的其他部分
            int resultWrite = dstBand->RasterIO(GF_Write, offset.first, offset.second, tileWidth, tileHeight, dataBuffers[bandIndex], tileWidth, tileHeight, GDT_Byte, 0, 0);
            if (resultWrite != CE_None) {
                qDebug() << "Failed to write data from tile to target for band " << bandIndex << " of tile " << tileIndex;
            }
        }

        // 释放内存缓冲区
        for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
            CPLFree(dataBuffers[bandIndex]);
        }

        tileIndex++;
    }

    emit progressUpdated(90);

    GDALClose(poTargetOutputDS);
}

//开始掩膜
void MaskExtraction::performMasking(const QString& mstrInputRasterPath, const QString& mstrMaskPath, const QString& mstrOutputRasterPath) {
    if (mstrMaskPath.endsWith(".tif")) {
        // rasterMask(mstrInputRasterPath, mstrMaskPath, mstrOutputRasterPath);
    }
    else if (mstrMaskPath.endsWith(".shp")) {
        vectorMask(mstrInputRasterPath, mstrMaskPath, mstrOutputRasterPath);
    }
    else {
        qDebug() << "Unsupported mask format. Please select a valid mask file (either raster in.tif or vector in.shp).";
    }
}

//打开文件
void MaskExtraction::selectInputRaster()
{
    mstrInputRasterPath = QFileDialog::getOpenFileName(nullptr, "Open File", "", "TIFF (*.tif)");

    if (mstrInputRasterPath.isEmpty()) {
        return;
    }
    QFileInfo fileInfo(mstrInputRasterPath);
    QString basePath = fileInfo.fileName();
    mUi.lineEditInputRaster->setText(basePath);
}

//打开用于掩膜的数据
void MaskExtraction::selectMask()
{
    mstrMaskPath = QFileDialog::getOpenFileName(nullptr, "Open File", "", "TIFF (*.tif) | Shapefile (*.shp)");

    if (mstrMaskPath.isEmpty())
        return;

    QFileInfo fileInfo(mstrMaskPath);
    QString basePath = fileInfo.fileName();
    mUi.lineEditInputMask->setText(basePath);
}

//保存文件
void MaskExtraction::selectOutputRaster()
{
    mstrOutputRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputRasterPath.isEmpty())
        return;

    mUi.lineEditOutputRaster->setText(mstrOutputRasterPath);
}

//开始分析
void MaskExtraction::process() {
    if (mstrInputRasterPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrMaskPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No mask input file selected."));
        return;
    }

    if (mstrOutputRasterPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No output file selected."));
        return;
    }

    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    if (mstrInputRasterPath.isEmpty()) {
        qDebug() << "No input raster file is selected. The program exits.";
        return;
    }
    if (mstrMaskPath.isEmpty()) {
        qDebug() << "No mask file is selected. The program exits.";
        return;
    }
    performMasking(mstrInputRasterPath, mstrMaskPath, mstrOutputRasterPath);

    emit progressUpdated(100);

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Mask Extraction runs successfully."));
}

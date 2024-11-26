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

// ����ʸ��������դ�����ݵ���Ч������Χ�������ͶӰ��Ϣ
void MaskExtraction::calculateIntersectionAndSetProjection(GDALDataset* mpPoInputDS, OGRLayer* poLayer, std::vector<OGREnvelope>& polygonEnvelopes) {
    if (mpPoInputDS->GetGeoTransform(mdAdfGeoTransform) == CE_None) {
        // ����ʸ��ͼ���е�ÿ������
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

        // ����դ���ͶӰ������ʹ������դ�������ͶӰ��
        const char* inputProjection = mpPoInputDS->GetProjectionRef();
        mpPoInputDS->SetProjection(inputProjection);
        emit progressUpdated(45);
    }
}

//ʸ����Ϊ��Ĥ
void MaskExtraction::vectorMask(const QString& mstrInputRasterPath, const QString& vectorPath, const QString& mstrOutputRasterPath) {
    // ������դ�����ݼ�
    GDALDataset* mpPoInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (mpPoInputDS == nullptr) {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    // ��ȡԭդ��Ĳ�������
    int bandCount = mpPoInputDS->GetRasterCount();
    emit progressUpdated(10);

    // ��ʸ�����ݼ�
    GDALDataset* poVectorDS = (GDALDataset*)GDALOpenEx(vectorPath.toStdString().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (poVectorDS == nullptr) {
        qDebug() << "Failed to open vector dataset.";
        GDALClose(mpPoInputDS);
        return;
    }

    // ��ȡʸ�����ݼ��ĵ�һ��ͼ��
    OGRLayer* poLayer = poVectorDS->GetLayer(0);
    if (poLayer == nullptr) {
        qDebug() << "Failed to get the first layer from vector dataset.";
        GDALClose(mpPoInputDS);
        GDALClose(poVectorDS);
        return;
    }

    // �洢ÿ������εķ�Χ
    std::vector<OGREnvelope> polygonEnvelopes;
    emit progressUpdated(20);
    calculateIntersectionAndSetProjection(mpPoInputDS, poLayer, polygonEnvelopes);

    // ����ÿ�������
    std::vector<TileInfo> tileInfos;
    int polygonIndex = 0;
    for (const auto& envelope : polygonEnvelopes) {
        // ����ÿ������ζ�Ӧ��դ��Χ
        int tempStartColFeature = static_cast<int>((envelope.MinX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempStartRowFeature = static_cast<int>((envelope.MaxY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);
        int tempEndColFeature = static_cast<int>((envelope.MaxX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempEndRowFeature = static_cast<int>((envelope.MinY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);

        // ��¼С��դ����Ϣ
        TileInfo tileInfo;
        tileInfo.x_offset = tempStartColFeature;
        tileInfo.y_offset = tempStartRowFeature;
        tileInfo.width = tempEndColFeature - tempStartColFeature + 1;
        tileInfo.height = tempEndRowFeature - tempStartRowFeature + 1;
        // ���������Ӵ洢 tempEndColFeature �� tempEndRowFeature �ĳ�Ա����
        tileInfo.endColFeature = tempEndColFeature;
        tileInfo.endRowFeature = tempEndRowFeature;
        tileInfos.push_back(tileInfo);

        polygonIndex++;
    }

    emit progressUpdated(65);

    // �ر�ʸ�����ݼ�
    GDALClose(poVectorDS);

    // ƴ��С��դ�����ݵ�һ��������դ��
    stitchTilesFromMemory(tileInfos, polygonEnvelopes, mstrOutputRasterPath, bandCount, mpPoInputDS);

    // �ر�����դ�����ݼ�
    GDALClose(mpPoInputDS);
}

//դ����Ϊ��Ĥ
void MaskExtraction::rasterMask(const QString& mstrInputRasterPath, const QString& mstrMaskPath, const QString& mstrOutputRasterPath) {
    // ������դ�����ݼ�
    GDALDataset* mpPoInputDS = (GDALDataset*)GDALOpen(mstrInputRasterPath.toStdString().c_str(), GA_ReadOnly);
    if (mpPoInputDS == nullptr) {
        qDebug() << "Failed to open input raster dataset.";
        return;
    }

    // ����Ĥդ�����ݼ�
    GDALDataset* poMaskDS = (GDALDataset*)GDALOpen(mstrMaskPath.toStdString().c_str(), GA_ReadOnly);
    if (poMaskDS == nullptr) {
        qDebug() << "Failed to open mask raster dataset.";
        GDALClose(mpPoInputDS);
        return;
    }

    // ��ȡԭդ��Ĳ�������
    int bandCount = mpPoInputDS->GetRasterCount();
    int maskBandCount = poMaskDS->GetRasterCount();

    if (bandCount != maskBandCount) {
        qDebug() << "The number of bands in the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // ��ȡ����դ�����Ĥդ��ĵ���任��Ϣ
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

    // �������դ�����Ĥդ��ĵ���任�Ƿ���ͬ
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

    // ��ȡ����դ�����Ĥդ���ͶӰ��Ϣ
    const char* inputProjection = mpPoInputDS->GetProjectionRef();
    const char* maskProjection = poMaskDS->GetProjectionRef();
    if (strcmp(inputProjection, maskProjection) != 0) {
        qDebug() << "The projections of the input raster and mask raster do not match.";
        GDALClose(mpPoInputDS);
        GDALClose(poMaskDS);
        return;
    }

    // ��ȡ����դ�����Ĥդ��Ŀ�Ⱥ͸߶�
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

    // ����Ŀ��դ�����ݼ�
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

    // ����Ŀ��դ��ĵ���任��ͶӰ����Ϣ
    if (poTargetOutputDS->GetGeoTransform(mdAdfGeoTransform) == CE_None) {
        poTargetOutputDS->SetGeoTransform(adfInputGeoTransform);
    }
    poTargetOutputDS->SetProjection(inputProjection);

    // ��ÿ�����ν�����Ĥ����
    for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
        GDALRasterBand* inputBand = mpPoInputDS->GetRasterBand(bandIndex + 1);
        GDALRasterBand* maskBand = poMaskDS->GetRasterBand(bandIndex + 1);
        GDALRasterBand* targetBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);

        // ��ȡ����դ�����Ĥդ������ݵ�������
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

        // ������Ĥ����
        for (int i = 0; i < inputWidth * inputHeight; i++) {
            if (maskData[i] == 0) {
                inputData[i] = 255; // ����Ϊ��ɫ������ 255 ��ʾ��ɫ��
            }
        }

        // ������������д��Ŀ��դ��
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

        // �ͷŻ������ڴ�
        CPLFree(inputData);
        CPLFree(maskData);
    }

    GDALClose(mpPoInputDS);
    GDALClose(poMaskDS);
    GDALClose(poTargetOutputDS);
}

// ����ÿ��С��������ƴ��ͼ���е�׼ȷƫ����
void MaskExtraction::stitchTilesFromMemory(const std::vector<TileInfo>& tileInfos, const std::vector<OGREnvelope>& polygonEnvelopes, const QString& mstrOutputRasterPath, int bandCount, GDALDataset* mpPoInputDS) {
    std::vector<std::pair<int, int>> tileOffsets;
    for (const auto& envelope : polygonEnvelopes) {
        int tempStartColFeature = static_cast<int>((envelope.MinX - mdAdfGeoTransform[0]) / mdAdfGeoTransform[1]);
        int tempStartRowFeature = static_cast<int>((envelope.MaxY - mdAdfGeoTransform[3]) / mdAdfGeoTransform[5]);
        tileOffsets.push_back(std::make_pair(tempStartColFeature, tempStartRowFeature));
    }
    emit progressUpdated(70);

    // ȷ������ƴ��ͼ��Ĵ�С
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

    // ������Ŀ��դ���ʼ��Ϊ��ɫ����
    for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
        GDALRasterBand* dstBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);
        dstBand->Fill(255);
    }

    // ����Ŀ��դ��ĵ���任��ͶӰ����Ϣ
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

        // ������Ҫ����ʵ������޸ģ�����ÿ�����ε����ݻ�������С�� tileWidth �� tileHeight ���
        std::vector<GByte*> dataBuffers(bandCount, nullptr);
        for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
            // ���仺�����ڴ�
            dataBuffers[bandIndex] = (GByte*)CPLMalloc(tileWidth * tileHeight * sizeof(GByte));

            // ��ԭդ���Ӧ��С���ж�ȡ���ݵ�������
            GDALRasterBand* srcBand = mpPoInputDS->GetRasterBand(bandIndex + 1);
            int resultRead = srcBand->RasterIO(GF_Read, tempStartColFeature, tempStartRowFeature, tileWidth, tileHeight, dataBuffers[bandIndex], tileWidth, tileHeight, GDT_Byte, 0, 0);
            if (resultRead != CE_None) {
                qDebug() << "Failed to read data from source for band " << bandIndex << " of tile " << tileIndex;
            }

            GDALRasterBand* dstBand = poTargetOutputDS->GetRasterBand(bandIndex + 1);
            // �޸����ֻд��Ӧ�������ݵĲ��֣��������ǰ�ɫ��������������
            int resultWrite = dstBand->RasterIO(GF_Write, offset.first, offset.second, tileWidth, tileHeight, dataBuffers[bandIndex], tileWidth, tileHeight, GDT_Byte, 0, 0);
            if (resultWrite != CE_None) {
                qDebug() << "Failed to write data from tile to target for band " << bandIndex << " of tile " << tileIndex;
            }
        }

        // �ͷ��ڴ滺����
        for (int bandIndex = 0; bandIndex < bandCount; bandIndex++) {
            CPLFree(dataBuffers[bandIndex]);
        }

        tileIndex++;
    }

    emit progressUpdated(90);

    GDALClose(poTargetOutputDS);
}

//��ʼ��Ĥ
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

//���ļ�
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

//��������Ĥ������
void MaskExtraction::selectMask()
{
    mstrMaskPath = QFileDialog::getOpenFileName(nullptr, "Open File", "", "TIFF (*.tif) | Shapefile (*.shp)");

    if (mstrMaskPath.isEmpty())
        return;

    QFileInfo fileInfo(mstrMaskPath);
    QString basePath = fileInfo.fileName();
    mUi.lineEditInputMask->setText(basePath);
}

//�����ļ�
void MaskExtraction::selectOutputRaster()
{
    mstrOutputRasterPath = QFileDialog::getSaveFileName(nullptr, "Save File", "", "TIFF (*.tif)");

    if (mstrOutputRasterPath.isEmpty())
        return;

    mUi.lineEditOutputRaster->setText(mstrOutputRasterPath);
}

//��ʼ����
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

    // ��¼��ʼʱ��
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

    // ��¼����ʱ��
    auto endTime = std::chrono::high_resolution_clock::now();
    // ���㾭����ʱ�䣨��λ���룩
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // ���ƾ��ȵ� 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // ���������ڵĲۺ������½���
    emit analysisProgressGoing(elapsedTime);

    QMessageBox::information(this, tr("Success"), tr("Mask Extraction runs successfully."));
}

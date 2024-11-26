#include "VectorLayerBufferAnalysis.h"
#include <QMessageBox>
#include <QDebug>
#include <chrono>

VectorLayerBufferAnalysis::VectorLayerBufferAnalysis(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("Vector Layer Buffer Analysis");
    setWindowIcon(QIcon(":/login/res/bufferAnalysis.png"));

    // Initialize GDAL and set the logging level to debug
    GDALAllRegister();
    CPLSetConfigOption("GDAL_LOG_LEVEL", "DEBUG");

    connect(mUi.openFile, &QPushButton::clicked, this, &VectorLayerBufferAnalysis::onBrowseButtonClicked);
    connect(mUi.begin, &QPushButton::clicked, this, &VectorLayerBufferAnalysis::beginClicked);
    connect(mUi.outputVectorButton, &QPushButton::clicked, this, &VectorLayerBufferAnalysis::saveOutputVectorFile);

    // ��ʼ��ͼ��Ϊ�հ�
    mOriginalImage = QImage(780, 540, QImage::Format_ARGB32);
    mOriginalImage.fill(Qt::transparent);

    // ��ʼ�����ͼ��Ϊ�հ�
    mResultImage = QImage(780, 540, QImage::Format_ARGB32);
    mResultImage.fill(Qt::transparent);
}

VectorLayerBufferAnalysis::~VectorLayerBufferAnalysis()
{
}

//���ļ�
void VectorLayerBufferAnalysis::onBrowseButtonClicked()
{
    mstrFilePath = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("Shapefile (*.shp)"));

    if (mstrFilePath.isEmpty()) return;

    QFileInfo fileInfo(mstrFilePath);
    QString baseName = fileInfo.fileName();
    mUi.lineEditVector->setText(baseName);
}

//�����ļ�
void VectorLayerBufferAnalysis::saveOutputVectorFile() {
    mstrResultPath = QFileDialog::getSaveFileName(this,
        tr("Save File"),
        "/home",
        tr("Shapefile (*.shp)"));

    if (mstrFilePath.isEmpty()) return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//��ʼ����
void VectorLayerBufferAnalysis::onAnalyzeButtonClicked()
{
    bool ok;
    double bufferDistance = mUi.bufferParamLineEdit->text().toDouble(&ok);

    if (ok)
    {
        mOriginalImage.fill(Qt::transparent);  // ÿ�η���ǰ���ͼ��
        performBufferAnalysis(bufferDistance);  // ���ٴ����ļ�·��

        QMessageBox::information(this, tr("Success"), tr("Vector Layer Buffer Analysis runs successfully."));
    }
    else
    {
        QMessageBox::warning(this, "Error", "Invalid buffer distance.");
    }
}

//�������������
void VectorLayerBufferAnalysis::loadAndProcessData(const QString& fileName)
{
    GDALAllRegister();

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

    if (poDS == NULL)
    {
        QMessageBox::warning(this, "Error", "Failed to open the file: " + fileName);
        return;
    }

    OGRLayer* poLayer = poDS->GetLayer(0);

    // ��ȡͼ��Ŀռ�ο�
    OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
    if (poSRS == NULL)
    {
        QMessageBox::warning(this, "Error", "Fail to get information.");
        GDALClose(poDS);
        return;
    }

    // ��ȡ��ķ�Χ��������
    OGREnvelope env;
    poLayer->GetExtent(&env);

    // �����ʼ��������
    double mdScaleX = width() / (env.MaxX - env.MinX);
    double mdScaleY = height() / (env.MaxY - env.MinY);
    double mdMinX = env.MinX;
    double mdMinY = env.MinY;

    // ѭ��������������Ⱦ��ԭʼͼ����ɫ����Ϊ����ɫ
    QPainter painter(&mOriginalImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(173, 216, 230));  // ����ɫ
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL)
        {
            renderGeometry(&painter, poGeometry, mdScaleX, mdScaleY, mdMinX, mdMinY, true);  // ����һ��������ʾ��ԭʼ����
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
}

//��������������
void VectorLayerBufferAnalysis::performBufferAnalysis(double bufferDistance)
{
    if (mstrFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No output file selected."));
        return;
    }

    // ��¼��ʼʱ��
    auto startTime = std::chrono::high_resolution_clock::now();

    GDALAllRegister();

    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

    if (poDS == NULL)
    {
        QMessageBox::warning(this, "Error", "Failed to open the file for buffer analysis: " + mstrFilePath);
        return;
    }
    emit progressUpdated(10);

    OGRLayer* poLayer = poDS->GetLayer(0);

    // ��ȡͼ��Ŀռ�ο�
    OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
    if (poSRS == NULL)
    {
        QMessageBox::warning(this, "Error", "No poSRS get.");
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(25);

    
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    GDALDataset* poOutDS = poDriver->Create(mstrResultPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poOutDS == NULL)
    {
        // ��ȡ�������ϸ�Ĵ�����Ϣ
        QMessageBox::warning(this, "Error", QString("Failed to create the output shapefile: %1. Error %2: %3")
            .arg(mstrResultPath.toStdString().c_str()).arg(CPLGetLastErrorNo()).arg(CPLGetLastErrorMsg()));
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(40);

    // �����µ�ͼ�㣬ʹ��ԭͼ��Ŀռ�ο�
    OGRLayer* poOutLayer = poOutDS->CreateLayer("buffer", poSRS, wkbPolygon, NULL);
    if (poOutLayer == NULL)
    {
        // ��ȡ�������ϸ�Ĵ�����Ϣ
        QMessageBox::warning(this, "Error", QString("Failed to create output layer. Error %1: %2")
            .arg(CPLGetLastErrorNo()).arg(CPLGetLastErrorMsg()));
        GDALClose(poOutDS);
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(55);

    // ѭ���������������л�����������ֱ����ԭʼͼ���ϵ��ӻ��ƣ���ɫ����Ϊ��ɫ
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL)
        {
            OGRwkbGeometryType geomType = wkbFlatten(poGeometry->getGeometryType());
            OGRGeometry* poBuffer = NULL;

            // �жϼ������Ͳ����ɶ�Ӧ�Ļ�����
            switch (geomType)
            {
            case wkbPoint:
            case wkbMultiPoint:
            case wkbLineString:
            case wkbMultiLineString:
            case wkbPolygon:
            case wkbMultiPolygon:
                poBuffer = poGeometry->Buffer(bufferDistance, 30);
                break;
            default:
                continue;  // ������֧�ֵ����ͣ�ֱ�ӽ�����һ��ѭ��
            }

            if (poBuffer != NULL)
            {
                QPainter painter(&mOriginalImage);
                painter.setRenderHint(QPainter::Antialiasing);
                OGREnvelope extent;
                poLayer->GetExtent(&extent);
                renderGeometry(&painter, poBuffer, width() / (extent.MaxX - extent.MinX),
                    height() / (extent.MaxY - extent.MinY),
                    extent.MinX, extent.MinY, false);  // ����һ��������ʾ����ԭʼ����

                // ��������������д����� Shapefile
                OGRFeature* poOutFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
                poOutFeature->SetGeometry(poBuffer);
                poOutLayer->CreateFeature(poOutFeature);
                OGRFeature::DestroyFeature(poOutFeature);
            }
            OGRGeometryFactory::destroyGeometry(poBuffer); // �ͷ��ڴ�
        }
        OGRFeature::DestroyFeature(poFeature);
    }
    emit progressUpdated(80);

    GDALClose(poOutDS);
    GDALClose(poDS);

    emit progressUpdated(100);

    // ��¼����ʱ��
    auto endTime = std::chrono::high_resolution_clock::now();
    // ���㾭����ʱ�䣨��λ���룩
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // ���ƾ��ȵ� 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // ���������ڵĲۺ������½���
    emit analysisProgressGoing(elapsedTime);

    // ������������Ƶ�ȫ�ֱ��� mResultImage
    mResultImage = mOriginalImage.copy();
}

//����
void VectorLayerBufferAnalysis::renderGeometry(QPainter* painter, OGRGeometry* geometry, double mdScaleX, double mdScaleY, double mdMinX, double mdMinY, bool isOriginal)
{
    if (wkbFlatten(geometry->getGeometryType()) == wkbPoint)
    {
        OGRPoint* point = (OGRPoint*)geometry;
        if (isOriginal)  // ���ݴ���Ĳ����ж���ɫ
        {
            painter->setPen(QColor(173, 216, 230));  // ԭʼ��Ϊ����ɫ
        }
        else
        {
            painter->setPen(Qt::red);  // ���������Ϊ��ɫ
        }
        int x = static_cast<int>((point->getX() - mdMinX) * mdScaleX);
        int y = static_cast<int>(height() - (point->getY() - mdMinY) * mdScaleY);
        painter->drawPoint(x, y);
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbLineString)
    {
        OGRLineString* line = (OGRLineString*)geometry;
        if (isOriginal)  // ���ݴ���Ĳ����ж���ɫ
        {
            painter->setPen(QColor(173, 216, 230));  // ԭʼ��Ϊ����ɫ
        }
        else
        {
            painter->setPen(Qt::red);  // ���������Ϊ��ɫ
        }
        QPolygonF polyline;
        for (int i = 0; i < line->getNumPoints(); ++i)
        {
            int x = static_cast<int>((line->getX(i) - mdMinX) * mdScaleX);
            int y = static_cast<int>(height() - (line->getY(i) - mdMinY) * mdScaleY);
            QPointF point(x, y);
            polyline << point;
        }
        painter->drawPolyline(polyline);
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon)
    {
        OGRPolygon* polygon = (OGRPolygon*)geometry;
        if (isOriginal)  // ���ݴ���Ĳ����ж���ɫ
        {
            painter->setPen(QColor(173, 216, 230));  // ԭʼ�����Ϊ����ɫ
            painter->setBrush(QColor(173, 216, 230));  // ���ɫҲΪ����ɫ
        }
        else
        {
            painter->setPen(Qt::red);  // ���������Ϊ��ɫ
            painter->setBrush(Qt::red);  // ���ɫΪ��ɫ
        }
        QPolygonF polygonF;
        for (int i = 0; i < polygon->getExteriorRing()->getNumPoints(); ++i)
        {
            int x = static_cast<int>((polygon->getExteriorRing()->getX(i) - mdMinX) * mdScaleX);
            int y = static_cast<int>(height() - (polygon->getExteriorRing()->getY(i) - mdMinY) * mdScaleY);
            QPointF point(x, y);
            polygonF << point;
        }
        painter->drawPolygon(polygonF);
    }
}
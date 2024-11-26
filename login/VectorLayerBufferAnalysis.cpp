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

    // 初始化图像为空白
    mOriginalImage = QImage(780, 540, QImage::Format_ARGB32);
    mOriginalImage.fill(Qt::transparent);

    // 初始化结果图像为空白
    mResultImage = QImage(780, 540, QImage::Format_ARGB32);
    mResultImage.fill(Qt::transparent);
}

VectorLayerBufferAnalysis::~VectorLayerBufferAnalysis()
{
}

//打开文件
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

//保存文件
void VectorLayerBufferAnalysis::saveOutputVectorFile() {
    mstrResultPath = QFileDialog::getSaveFileName(this,
        tr("Save File"),
        "/home",
        tr("Shapefile (*.shp)"));

    if (mstrFilePath.isEmpty()) return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//开始分析
void VectorLayerBufferAnalysis::onAnalyzeButtonClicked()
{
    bool ok;
    double bufferDistance = mUi.bufferParamLineEdit->text().toDouble(&ok);

    if (ok)
    {
        mOriginalImage.fill(Qt::transparent);  // 每次分析前清空图像
        performBufferAnalysis(bufferDistance);  // 不再传递文件路径

        QMessageBox::information(this, tr("Success"), tr("Vector Layer Buffer Analysis runs successfully."));
    }
    else
    {
        QMessageBox::warning(this, "Error", "Invalid buffer distance.");
    }
}

//处理输入的数据
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

    // 获取图层的空间参考
    OGRSpatialReference* poSRS = poLayer->GetSpatialRef();
    if (poSRS == NULL)
    {
        QMessageBox::warning(this, "Error", "Fail to get information.");
        GDALClose(poDS);
        return;
    }

    // 获取层的范围用于缩放
    OGREnvelope env;
    poLayer->GetExtent(&env);

    // 计算初始缩放因子
    double mdScaleX = width() / (env.MaxX - env.MinX);
    double mdScaleY = height() / (env.MaxY - env.MinY);
    double mdMinX = env.MinX;
    double mdMinY = env.MinY;

    // 循环处理特征并渲染到原始图像，颜色设置为淡蓝色
    QPainter painter(&mOriginalImage);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(173, 216, 230));  // 淡蓝色
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL)
        {
            renderGeometry(&painter, poGeometry, mdScaleX, mdScaleY, mdMinX, mdMinY, true);  // 新增一个参数表示是原始数据
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    GDALClose(poDS);
}

//缓冲区分析计算
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

    // 记录开始时间
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

    // 获取图层的空间参考
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
        // 获取并输出详细的错误信息
        QMessageBox::warning(this, "Error", QString("Failed to create the output shapefile: %1. Error %2: %3")
            .arg(mstrResultPath.toStdString().c_str()).arg(CPLGetLastErrorNo()).arg(CPLGetLastErrorMsg()));
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(40);

    // 创建新的图层，使用原图层的空间参考
    OGRLayer* poOutLayer = poOutDS->CreateLayer("buffer", poSRS, wkbPolygon, NULL);
    if (poOutLayer == NULL)
    {
        // 获取并输出详细的错误信息
        QMessageBox::warning(this, "Error", QString("Failed to create output layer. Error %1: %2")
            .arg(CPLGetLastErrorNo()).arg(CPLGetLastErrorMsg()));
        GDALClose(poOutDS);
        GDALClose(poDS);
        return;
    }
    emit progressUpdated(55);

    // 循环处理特征并进行缓冲区分析，直接在原始图像上叠加绘制，颜色设置为红色
    poLayer->ResetReading();
    OGRFeature* poFeature;
    while ((poFeature = poLayer->GetNextFeature()) != NULL)
    {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != NULL)
        {
            OGRwkbGeometryType geomType = wkbFlatten(poGeometry->getGeometryType());
            OGRGeometry* poBuffer = NULL;

            // 判断几何类型并生成对应的缓冲区
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
                continue;  // 遇到不支持的类型，直接进入下一次循环
            }

            if (poBuffer != NULL)
            {
                QPainter painter(&mOriginalImage);
                painter.setRenderHint(QPainter::Antialiasing);
                OGREnvelope extent;
                poLayer->GetExtent(&extent);
                renderGeometry(&painter, poBuffer, width() / (extent.MaxX - extent.MinX),
                    height() / (extent.MaxY - extent.MinY),
                    extent.MinX, extent.MinY, false);  // 新增一个参数表示不是原始数据

                // 将缓冲区几何体写入输出 Shapefile
                OGRFeature* poOutFeature = OGRFeature::CreateFeature(poOutLayer->GetLayerDefn());
                poOutFeature->SetGeometry(poBuffer);
                poOutLayer->CreateFeature(poOutFeature);
                OGRFeature::DestroyFeature(poOutFeature);
            }
            OGRGeometryFactory::destroyGeometry(poBuffer); // 释放内存
        }
        OGRFeature::DestroyFeature(poFeature);
    }
    emit progressUpdated(80);

    GDALClose(poOutDS);
    GDALClose(poDS);

    emit progressUpdated(100);

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);

    // 将分析结果复制到全局变量 mResultImage
    mResultImage = mOriginalImage.copy();
}

//绘制
void VectorLayerBufferAnalysis::renderGeometry(QPainter* painter, OGRGeometry* geometry, double mdScaleX, double mdScaleY, double mdMinX, double mdMinY, bool isOriginal)
{
    if (wkbFlatten(geometry->getGeometryType()) == wkbPoint)
    {
        OGRPoint* point = (OGRPoint*)geometry;
        if (isOriginal)  // 根据传入的参数判断颜色
        {
            painter->setPen(QColor(173, 216, 230));  // 原始点为淡蓝色
        }
        else
        {
            painter->setPen(Qt::red);  // 缓冲区结果为红色
        }
        int x = static_cast<int>((point->getX() - mdMinX) * mdScaleX);
        int y = static_cast<int>(height() - (point->getY() - mdMinY) * mdScaleY);
        painter->drawPoint(x, y);
    }
    else if (wkbFlatten(geometry->getGeometryType()) == wkbLineString)
    {
        OGRLineString* line = (OGRLineString*)geometry;
        if (isOriginal)  // 根据传入的参数判断颜色
        {
            painter->setPen(QColor(173, 216, 230));  // 原始线为淡蓝色
        }
        else
        {
            painter->setPen(Qt::red);  // 缓冲区结果为红色
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
        if (isOriginal)  // 根据传入的参数判断颜色
        {
            painter->setPen(QColor(173, 216, 230));  // 原始多边形为淡蓝色
            painter->setBrush(QColor(173, 216, 230));  // 填充色也为淡蓝色
        }
        else
        {
            painter->setPen(Qt::red);  // 缓冲区结果为红色
            painter->setBrush(Qt::red);  // 填充色为红色
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
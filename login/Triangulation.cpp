#include "Triangulation.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <QPushButton>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <ogrsf_frmts.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;
typedef K::Point_2 Point;

Triangulation::Triangulation(QWidget* parent)
    : QWidget(parent)
{
    mUi.setupUi(this);

    setWindowTitle("Triangulation Analysis");
    setWindowIcon(QIcon(":/login/res/Triangulation.png"));

    // 手动连接信号与槽
    connect(mUi.openFile, &QPushButton::clicked, this, &Triangulation::selectInputFile);
    connect(mUi.outputVectorButton, &QPushButton::clicked, this, &Triangulation::selectOutputFile);
    connect(mUi.begin, &QPushButton::clicked, this, &Triangulation::beginClicked);
}

Triangulation::~Triangulation()
{}

//打开文件
void Triangulation::selectInputFile()
{
    // 选择输入矢量文件
    mstrInputFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Shapefile (*.shp)"));
    if (mstrInputFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(mstrInputFilePath);
    QString baseName = fileInfo.fileName();
    mUi.lineEditVector->setText(baseName);
}

//保存文件
void Triangulation::selectOutputFile()
{
    // 选择输出路径，并确保用户不会覆盖已有文件
    mstrResultPath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Shapefile (*.shp)"));
    if (mstrResultPath.isEmpty()) return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//三角网分析
void Triangulation::runTriangulation()
{
    if (mstrInputFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
        return;
    }

    if (mstrResultPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No output file selected."));
        return;
    }

    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();

    // 检查是否选择了输入文件和保存路径
    if (mstrInputFilePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No input file selected. Please select an input file first."));
        return;
    }
    if (mstrResultPath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No output file path selected. Please select a save path first."));
        return;
    }

    emit progressUpdated(10);

    try {
        // 执行 Delaunay 三角剖分，并获取三角形列表
        std::vector<Triangle> triangles = computeDelaunayTriangulation();

        if (triangles.empty()) {
            std::cerr << "No triangles were generated!" << std::endl;
            return;
        }
        else {
            std::cout << "Generated " << triangles.size() << " triangles." << std::endl;
        }

        emit progressUpdated(70);

        // 将三角形列表保存到 Shapefile 中
        saveShapefile(triangles);

        emit progressUpdated(100);

        // 记录结束时间
        auto endTime = std::chrono::high_resolution_clock::now();
        // 计算经过的时间（单位：秒）
        double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

        // 限制精度到 0.001
        elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

        // 调用主窗口的槽函数更新界面
        emit analysisProgressGoing(elapsedTime);

        QMessageBox::information(this, tr("Success"), tr("Triangulation Analysis runs successfully."));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
    }
}

//计算三角网
std::vector<Triangle> Triangulation::computeDelaunayTriangulation() {
    // 初始化 GDAL 库
    GDALAllRegister();

    // 打开输入的 Shapefile
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrInputFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to open input shapefile.");
    }

    emit progressUpdated(20);

    // 获取矢量图层
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to get layer from input shapefile.");
    }

    emit progressUpdated(25);

    // 从 Shapefile 中读取点数据
    std::vector<Point> points;
    OGRFeature* poFeature;
    poLayer->ResetReading();
    while ((poFeature = poLayer->GetNextFeature()) != nullptr) {
        OGRGeometry* poGeometry = poFeature->GetGeometryRef();
        if (poGeometry != nullptr && wkbFlatten(poGeometry->getGeometryType()) == wkbPoint) {
            OGRPoint* poPoint = (OGRPoint*)poGeometry;
            points.push_back(Point(poPoint->getX(), poPoint->getY()));
        }
        OGRFeature::DestroyFeature(poFeature);
    }

    emit progressUpdated(45);

    // 关闭 Shapefile
    GDALClose(poDS);

    // 执行 Delaunay 三角剖分
    Delaunay dt;
    dt.insert(points.begin(), points.end());

    // 将结果存储为 Triangle 结构的向量
    std::vector<Triangle> triangles;
    for (auto it = dt.finite_faces_begin(); it != dt.finite_faces_end(); ++it) {
        Triangle t;
        t.x1 = it->vertex(0)->point().x();
        t.y1 = it->vertex(0)->point().y();
        t.x2 = it->vertex(1)->point().x();
        t.y2 = it->vertex(1)->point().y();
        t.x3 = it->vertex(2)->point().x();
        t.y3 = it->vertex(2)->point().y();
        triangles.push_back(t);
    }

    emit progressUpdated(65);

    return triangles;
}

//保存成shp
void Triangulation::saveShapefile(const std::vector<Triangle>& triangles) {
    // 创建 GDAL 驱动
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (poDriver == nullptr) {
        throw std::runtime_error("ESRI Shapefile driver not available.");
    }

    emit progressUpdated(75);

    // 创建输出 Shapefile
    GDALDataset* poDS = poDriver->Create(mstrResultPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to create output shapefile.");
    }

    // 创建图层
    OGRLayer* poLayer = poDS->CreateLayer("triangles", NULL, wkbPolygon, NULL);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to create layer in output shapefile.");
    }

    // 定义字段
    OGRFieldDefn oFieldName("ID", OFTInteger);
    poLayer->CreateField(&oFieldName);

    emit progressUpdated(80);

    // 为每个三角形创建要素
    int nID = 0;
    for (const auto& triangle : triangles) {
        OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        // 设置 ID 字段
        poFeature->SetField("ID", nID++);

        // 创建三角形的几何体
        OGRPolygon oPolygon;
        OGRLinearRing oRing;
        oRing.addPoint(triangle.x1, triangle.y1);
        oRing.addPoint(triangle.x2, triangle.y2);
        oRing.addPoint(triangle.x3, triangle.y3);
        oRing.addPoint(triangle.x1, triangle.y1); // 闭合三角形
        oPolygon.addRing(&oRing);

        poFeature->SetGeometry(&oPolygon);

        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            OGRFeature::DestroyFeature(poFeature);
            GDALClose(poDS);
            throw std::runtime_error("Failed to create feature in shapefile.");
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    // 关闭 Shapefile
    GDALClose(poDS);

    emit progressUpdated(90);
}
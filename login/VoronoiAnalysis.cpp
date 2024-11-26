#include "VoronoiAnalysis.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <ogrsf_frmts.h>
#include <chrono>

// 定义类型
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;
typedef K::Point_2 Point;
typedef K::Segment_2 Segment;
typedef Delaunay::Face_circulator Face_circulator;

VoronoiAnalysis::VoronoiAnalysis(QWidget* parent)
    : QWidget(parent)
{

    mUi.setupUi(this);

    setWindowTitle("Voronoi Analysis");
    setWindowIcon(QIcon(":/login/res/Voronoi.png"));

    connect(mUi.openFile, &QPushButton::clicked, this, &VoronoiAnalysis::selectInputFile);
    connect(mUi.outputVectorButton, &QPushButton::clicked, this, &VoronoiAnalysis::selectSavePath);
    connect(mUi.begin, &QPushButton::clicked, this, &VoronoiAnalysis::beginClicked);

}

VoronoiAnalysis::~VoronoiAnalysis()
{}

//打开文件
void VoronoiAnalysis::selectInputFile()
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
void VoronoiAnalysis::selectSavePath()
{
    mstrResultPath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Shapefile (*.shp)"));
    if (mstrResultPath.isEmpty())
        return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//执行计算
void VoronoiAnalysis::performVoronoi()
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
        // 计算并导出 Voronoi 图
        computeAndExport();
        QMessageBox::information(this, tr("Success"), tr("Voronoi Analysis runs successfully."));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
    }

    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    // 计算经过的时间（单位：秒）
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // 限制精度到 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // 调用主窗口的槽函数更新界面
    emit analysisProgressGoing(elapsedTime);
}

//计算并导出Voronoi图
void VoronoiAnalysis::computeAndExport()
{
    // 执行 Voronoi 图的计算，并获取多边形列表
    std::vector<Polygon> polygons = computeVoronoiDiagram();

    if (polygons.empty()) {
        std::cerr << "No polygons were generated!" << std::endl;
        return;
    }
    else {
        std::cout << "Generated " << polygons.size() << " polygons." << std::endl;
    }

    emit progressUpdated(60);

    // 将多边形列表保存到 Shapefile 中
    saveShapefile(polygons);
}

//计算Voronoi图
std::vector<Polygon> VoronoiAnalysis::computeVoronoiDiagram()
{
    // 初始化 GDAL 库
    GDALAllRegister();

    // 打开输入的 Shapefile
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrInputFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to open input shapefile.");
    }

    emit progressUpdated(15);

    // 获取矢量图层
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to get layer from input shapefile.");
    }

    emit progressUpdated(20);

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

    emit progressUpdated(30);

    // 关闭 Shapefile
    GDALClose(poDS);

    // 执行 Delaunay 三角剖分
    Delaunay dt;
    dt.insert(points.begin(), points.end());

    // 手动提取 Voronoi 图
    std::vector<Polygon> polygons;
    for (auto vit = dt.finite_vertices_begin(); vit != dt.finite_vertices_end(); ++vit) {
        Polygon poly;
        Face_circulator fc_start = dt.incident_faces(vit), fc = fc_start;
        do {
            if (!dt.is_infinite(fc)) {
                auto circumcenter = dt.circumcenter(fc);
                poly.vertices.push_back({ circumcenter.x(), circumcenter.y() });
            }
        } while (++fc != fc_start);

        if (!poly.vertices.empty()) {
            polygons.push_back(poly);
        }
    }

    return polygons;

    emit progressUpdated(50);
}

//保存成shp
void VoronoiAnalysis::saveShapefile(const std::vector<Polygon>& polygons)
{
    // 创建 GDAL 驱动
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (poDriver == nullptr) {
        throw std::runtime_error("ESRI Shapefile driver not available.");
    }

    emit progressUpdated(65);

    // 创建输出 Shapefile
    GDALDataset* poDS = poDriver->Create(mstrResultPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to create output shapefile.");
    }

    emit progressUpdated(70);

    // 创建图层
    OGRLayer* poLayer = poDS->CreateLayer("voronoi", NULL, wkbPolygon, NULL);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to create layer in output shapefile.");
    }

    emit progressUpdated(75);

    // 定义字段
    OGRFieldDefn oFieldName("ID", OFTInteger);
    poLayer->CreateField(&oFieldName);

    // 为每个多边形创建要素
    int nID = 0;
    for (const auto& polygon : polygons) {
        OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        // 设置 ID 字段
        poFeature->SetField("ID", nID++);

        // 创建多边形的几何体
        OGRPolygon oPolygon;
        OGRLinearRing oRing;
        for (const auto& vertex : polygon.vertices) {
            oRing.addPoint(vertex.first, vertex.second);
        }
        oRing.closeRings(); // 闭合多边形
        oPolygon.addRing(&oRing);

        poFeature->SetGeometry(&oPolygon);

        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            OGRFeature::DestroyFeature(poFeature);
            GDALClose(poDS);
            throw std::runtime_error("Failed to create feature in shapefile.");
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    emit progressUpdated(90);

    // 关闭 Shapefile
    GDALClose(poDS);

    emit progressUpdated(100);
}

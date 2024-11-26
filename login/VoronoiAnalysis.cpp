#include "VoronoiAnalysis.h"
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <fstream>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <ogrsf_frmts.h>
#include <chrono>

// ��������
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

//���ļ�
void VoronoiAnalysis::selectInputFile()
{
    // ѡ������ʸ���ļ�
    mstrInputFilePath = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Shapefile (*.shp)"));
    if (mstrInputFilePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(mstrInputFilePath);
    QString baseName = fileInfo.fileName();
    mUi.lineEditVector->setText(baseName);
}

//�����ļ�
void VoronoiAnalysis::selectSavePath()
{
    mstrResultPath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Shapefile (*.shp)"));
    if (mstrResultPath.isEmpty())
        return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//ִ�м���
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

    // ��¼��ʼʱ��
    auto startTime = std::chrono::high_resolution_clock::now();

    // ����Ƿ�ѡ���������ļ��ͱ���·��
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
        // ���㲢���� Voronoi ͼ
        computeAndExport();
        QMessageBox::information(this, tr("Success"), tr("Voronoi Analysis runs successfully."));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
    }

    // ��¼����ʱ��
    auto endTime = std::chrono::high_resolution_clock::now();
    // ���㾭����ʱ�䣨��λ���룩
    double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

    // ���ƾ��ȵ� 0.001
    elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

    // ���������ڵĲۺ������½���
    emit analysisProgressGoing(elapsedTime);
}

//���㲢����Voronoiͼ
void VoronoiAnalysis::computeAndExport()
{
    // ִ�� Voronoi ͼ�ļ��㣬����ȡ������б�
    std::vector<Polygon> polygons = computeVoronoiDiagram();

    if (polygons.empty()) {
        std::cerr << "No polygons were generated!" << std::endl;
        return;
    }
    else {
        std::cout << "Generated " << polygons.size() << " polygons." << std::endl;
    }

    emit progressUpdated(60);

    // ��������б��浽 Shapefile ��
    saveShapefile(polygons);
}

//����Voronoiͼ
std::vector<Polygon> VoronoiAnalysis::computeVoronoiDiagram()
{
    // ��ʼ�� GDAL ��
    GDALAllRegister();

    // ������� Shapefile
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrInputFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to open input shapefile.");
    }

    emit progressUpdated(15);

    // ��ȡʸ��ͼ��
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to get layer from input shapefile.");
    }

    emit progressUpdated(20);

    // �� Shapefile �ж�ȡ������
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

    // �ر� Shapefile
    GDALClose(poDS);

    // ִ�� Delaunay �����ʷ�
    Delaunay dt;
    dt.insert(points.begin(), points.end());

    // �ֶ���ȡ Voronoi ͼ
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

//�����shp
void VoronoiAnalysis::saveShapefile(const std::vector<Polygon>& polygons)
{
    // ���� GDAL ����
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (poDriver == nullptr) {
        throw std::runtime_error("ESRI Shapefile driver not available.");
    }

    emit progressUpdated(65);

    // ������� Shapefile
    GDALDataset* poDS = poDriver->Create(mstrResultPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to create output shapefile.");
    }

    emit progressUpdated(70);

    // ����ͼ��
    OGRLayer* poLayer = poDS->CreateLayer("voronoi", NULL, wkbPolygon, NULL);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to create layer in output shapefile.");
    }

    emit progressUpdated(75);

    // �����ֶ�
    OGRFieldDefn oFieldName("ID", OFTInteger);
    poLayer->CreateField(&oFieldName);

    // Ϊÿ������δ���Ҫ��
    int nID = 0;
    for (const auto& polygon : polygons) {
        OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        // ���� ID �ֶ�
        poFeature->SetField("ID", nID++);

        // ��������εļ�����
        OGRPolygon oPolygon;
        OGRLinearRing oRing;
        for (const auto& vertex : polygon.vertices) {
            oRing.addPoint(vertex.first, vertex.second);
        }
        oRing.closeRings(); // �պ϶����
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

    // �ر� Shapefile
    GDALClose(poDS);

    emit progressUpdated(100);
}

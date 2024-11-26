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

    // �ֶ������ź����
    connect(mUi.openFile, &QPushButton::clicked, this, &Triangulation::selectInputFile);
    connect(mUi.outputVectorButton, &QPushButton::clicked, this, &Triangulation::selectOutputFile);
    connect(mUi.begin, &QPushButton::clicked, this, &Triangulation::beginClicked);
}

Triangulation::~Triangulation()
{}

//���ļ�
void Triangulation::selectInputFile()
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
void Triangulation::selectOutputFile()
{
    // ѡ�����·������ȷ���û����Ḳ�������ļ�
    mstrResultPath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Shapefile (*.shp)"));
    if (mstrResultPath.isEmpty()) return;

    mUi.lineEditOutputVector->setText(mstrResultPath);
}

//����������
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
        // ִ�� Delaunay �����ʷ֣�����ȡ�������б�
        std::vector<Triangle> triangles = computeDelaunayTriangulation();

        if (triangles.empty()) {
            std::cerr << "No triangles were generated!" << std::endl;
            return;
        }
        else {
            std::cout << "Generated " << triangles.size() << " triangles." << std::endl;
        }

        emit progressUpdated(70);

        // ���������б��浽 Shapefile ��
        saveShapefile(triangles);

        emit progressUpdated(100);

        // ��¼����ʱ��
        auto endTime = std::chrono::high_resolution_clock::now();
        // ���㾭����ʱ�䣨��λ���룩
        double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

        // ���ƾ��ȵ� 0.001
        elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

        // ���������ڵĲۺ������½���
        emit analysisProgressGoing(elapsedTime);

        QMessageBox::information(this, tr("Success"), tr("Triangulation Analysis runs successfully."));
    }
    catch (const std::exception& e) {
        QMessageBox::critical(this, tr("Error"), tr(e.what()));
    }
}

//����������
std::vector<Triangle> Triangulation::computeDelaunayTriangulation() {
    // ��ʼ�� GDAL ��
    GDALAllRegister();

    // ������� Shapefile
    GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrInputFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to open input shapefile.");
    }

    emit progressUpdated(20);

    // ��ȡʸ��ͼ��
    OGRLayer* poLayer = poDS->GetLayer(0);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to get layer from input shapefile.");
    }

    emit progressUpdated(25);

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

    emit progressUpdated(45);

    // �ر� Shapefile
    GDALClose(poDS);

    // ִ�� Delaunay �����ʷ�
    Delaunay dt;
    dt.insert(points.begin(), points.end());

    // ������洢Ϊ Triangle �ṹ������
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

//�����shp
void Triangulation::saveShapefile(const std::vector<Triangle>& triangles) {
    // ���� GDAL ����
    GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (poDriver == nullptr) {
        throw std::runtime_error("ESRI Shapefile driver not available.");
    }

    emit progressUpdated(75);

    // ������� Shapefile
    GDALDataset* poDS = poDriver->Create(mstrResultPath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
    if (poDS == nullptr) {
        throw std::runtime_error("Failed to create output shapefile.");
    }

    // ����ͼ��
    OGRLayer* poLayer = poDS->CreateLayer("triangles", NULL, wkbPolygon, NULL);
    if (poLayer == nullptr) {
        GDALClose(poDS);
        throw std::runtime_error("Failed to create layer in output shapefile.");
    }

    // �����ֶ�
    OGRFieldDefn oFieldName("ID", OFTInteger);
    poLayer->CreateField(&oFieldName);

    emit progressUpdated(80);

    // Ϊÿ�������δ���Ҫ��
    int nID = 0;
    for (const auto& triangle : triangles) {
        OGRFeature* poFeature = OGRFeature::CreateFeature(poLayer->GetLayerDefn());

        // ���� ID �ֶ�
        poFeature->SetField("ID", nID++);

        // ���������εļ�����
        OGRPolygon oPolygon;
        OGRLinearRing oRing;
        oRing.addPoint(triangle.x1, triangle.y1);
        oRing.addPoint(triangle.x2, triangle.y2);
        oRing.addPoint(triangle.x3, triangle.y3);
        oRing.addPoint(triangle.x1, triangle.y1); // �պ�������
        oPolygon.addRing(&oRing);

        poFeature->SetGeometry(&oPolygon);

        if (poLayer->CreateFeature(poFeature) != OGRERR_NONE) {
            OGRFeature::DestroyFeature(poFeature);
            GDALClose(poDS);
            throw std::runtime_error("Failed to create feature in shapefile.");
        }

        OGRFeature::DestroyFeature(poFeature);
    }

    // �ر� Shapefile
    GDALClose(poDS);

    emit progressUpdated(90);
}
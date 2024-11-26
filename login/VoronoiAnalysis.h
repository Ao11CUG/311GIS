#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VoronoiAnalysis.h"
#include <vector>
#include <string>

// ����һ���ṹ����ʾ�����
struct Polygon {
    std::vector<std::pair<double, double>> vertices;
};

class VoronoiAnalysis : public QWidget
{
    Q_OBJECT

public:
    VoronoiAnalysis(QWidget* parent = nullptr);
    ~VoronoiAnalysis();

    QString mstrInputFilePath;   // ���ڴ洢�����ļ�·����ȫ�ֱ���
    QString mstrResultPath;      // ���ڴ洢����·����ȫ�ֱ���

signals:
    void beginClicked();       // ��ʼ������ź�
    void progressUpdated(int progress); // ������ȸ����ź�
    void analysisProgressGoing(double elapsedTime);

public slots:
    void selectInputFile(); // ѡ�������ļ�·��
    void selectSavePath();   // ѡ�񱣴��ļ�·��
    void performVoronoi();   // ִ��������������������

private:
    void computeAndExport();   // ���㲢���� Voronoi ͼ
    std::vector<Polygon> computeVoronoiDiagram();  // ���� Voronoi ͼ
    void saveShapefile(const std::vector<Polygon>& polygons);  // ���� Shapefile

    Ui::VoronoiAnalysisClass mUi;
};

#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Triangulation.h"
#include <string>
#include <vector>

// ����һ���ṹ����ʾ������
struct Triangle {
    double x1, y1;
    double x2, y2;
    double x3, y3;
};

class Triangulation : public QWidget
{
    Q_OBJECT

public:
    Triangulation(QWidget* parent = nullptr);
    ~Triangulation();

    QString mstrInputFilePath;   // ���ڴ洢�����ļ�·����ȫ�ֱ���
    QString mstrResultPath;      // ���ڴ洢����·����ȫ�ֱ���

public slots:
    void selectInputFile(); // ѡ�������ļ�·��
    void selectOutputFile();   // ѡ�񱣴��ļ�·��
    void runTriangulation();   // ִ��������������������

signals:
    void beginClicked();
    void progressUpdated(int progress); // ������ȸ����ź�
    void analysisProgressGoing(double elapsedTime);

private:
    Ui::TriangulationClass mUi;

    // ���� Delaunay �����ʷֲ����� Shapefile
    std::vector<Triangle> computeDelaunayTriangulation();
    void saveShapefile(const std::vector<Triangle>& triangles);
};



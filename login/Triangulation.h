#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_Triangulation.h"
#include <string>
#include <vector>

// 定义一个结构来表示三角形
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

    QString mstrInputFilePath;   // 用于存储输入文件路径的全局变量
    QString mstrResultPath;      // 用于存储保存路径的全局变量

public slots:
    void selectInputFile(); // 选择输入文件路径
    void selectOutputFile();   // 选择保存文件路径
    void runTriangulation();   // 执行三角网分析并保存结果

signals:
    void beginClicked();
    void progressUpdated(int progress); // 定义进度更新信号
    void analysisProgressGoing(double elapsedTime);

private:
    Ui::TriangulationClass mUi;

    // 计算 Delaunay 三角剖分并保存 Shapefile
    std::vector<Triangle> computeDelaunayTriangulation();
    void saveShapefile(const std::vector<Triangle>& triangles);
};



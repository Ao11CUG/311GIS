#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_VoronoiAnalysis.h"
#include <vector>
#include <string>

// 定义一个结构来表示多边形
struct Polygon {
    std::vector<std::pair<double, double>> vertices;
};

class VoronoiAnalysis : public QWidget
{
    Q_OBJECT

public:
    VoronoiAnalysis(QWidget* parent = nullptr);
    ~VoronoiAnalysis();

    QString mstrInputFilePath;   // 用于存储输入文件路径的全局变量
    QString mstrResultPath;      // 用于存储保存路径的全局变量

signals:
    void beginClicked();       // 开始处理的信号
    void progressUpdated(int progress); // 定义进度更新信号
    void analysisProgressGoing(double elapsedTime);

public slots:
    void selectInputFile(); // 选择输入文件路径
    void selectSavePath();   // 选择保存文件路径
    void performVoronoi();   // 执行三角网分析并保存结果

private:
    void computeAndExport();   // 计算并导出 Voronoi 图
    std::vector<Polygon> computeVoronoiDiagram();  // 计算 Voronoi 图
    void saveShapefile(const std::vector<Polygon>& polygons);  // 保存 Shapefile

    Ui::VoronoiAnalysisClass mUi;
};

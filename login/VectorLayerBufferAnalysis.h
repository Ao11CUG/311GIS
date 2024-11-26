#pragma once

#include <QtWidgets>
#include "ui_VectorLayerBufferAnalysis.h"
#include <QObject>
#include <QImage>
#include <gdal.h>
#include <ogr_api.h>
#include <QFileDialog>
#include <QMessageBox>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <ogr_core.h>
#include <ogrsf_frmts.h>
#include <ogr_feature.h>
#include <ogr_geometry.h>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <cpl_error.h>
#include <mutex>

class VectorLayerBufferAnalysis : public QWidget
{
    Q_OBJECT

public:
    VectorLayerBufferAnalysis(QWidget* parent = nullptr);
    ~VectorLayerBufferAnalysis();

    QString mstrFilePath; // 存储用户输入的文件路径
	QString mstrResultPath;//存储结果文件
    QImage mResultImage; // 存储分析结果的图像

	// 加载和处理矢量数据
	void loadAndProcessData(const QString& fileName);
	// 执行缓冲区分析
	void performBufferAnalysis(double bufferDistance);

	// 原始数据图像
	QImage mOriginalImage;

	// 渲染几何图形的函数
	void renderGeometry(QPainter* painter, OGRGeometry* geometry, double mdScaleX, double mdScaleY, double mdMinX, double mdMinY, bool isOriginal);  // 新增参数 isOriginal

	double mdScaleX = 0;
	double mdScaleY = 0;
	double mdMinX = 0;
	double mdMinY = 0;

public slots:
	void onBrowseButtonClicked();
	void onAnalyzeButtonClicked();
	void saveOutputVectorFile();

signals:
    void beginClicked();
	void progressUpdated(int progress); // 定义进度更新信号
	void analysisProgressGoing(double elapsedTime);

private:
    Ui::VectorLayerBufferAnalysisClass mUi;
};

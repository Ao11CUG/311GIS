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

    QString mstrFilePath; // �洢�û�������ļ�·��
	QString mstrResultPath;//�洢����ļ�
    QImage mResultImage; // �洢���������ͼ��

	// ���غʹ���ʸ������
	void loadAndProcessData(const QString& fileName);
	// ִ�л���������
	void performBufferAnalysis(double bufferDistance);

	// ԭʼ����ͼ��
	QImage mOriginalImage;

	// ��Ⱦ����ͼ�εĺ���
	void renderGeometry(QPainter* painter, OGRGeometry* geometry, double mdScaleX, double mdScaleY, double mdMinX, double mdMinY, bool isOriginal);  // �������� isOriginal

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
	void progressUpdated(int progress); // ������ȸ����ź�
	void analysisProgressGoing(double elapsedTime);

private:
    Ui::VectorLayerBufferAnalysisClass mUi;
};

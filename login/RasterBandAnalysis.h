#pragma once

#include <QWidget>
#include "ui_ProcessRaster.h"
#include <QObject>
#include <QString>
#include <gdal_priv.h>
#include <mutex>

class RasterBandAnalysis : public QWidget
{
    Q_OBJECT

public:
    RasterBandAnalysis(QWidget* parent = nullptr);
    ~RasterBandAnalysis();

    // դ���ļ�·��
    QString mstrRasterFilePath;
    //���ɫ��ʾ 
    QString mstrTrueColorPath;
    //�ٲ�ɫ��ʾ
    QString mstrFalseColorPath;
    //�Ҷ�ֱ��ͼ
    QString mstrGrayHistogramPath;
    //ֱ��ͼ���⻯
    QString mstrEnhancedHistogramPath;

    int mnTotalSteps; // ��¼�ܲ���

public slots:
    // դ������
    void processRasterData();  // ����դ��ͼ����������
    void openFile();           // ���ļ��Ĳۺ���
    void saveTrueColor();
    void saveGrayHistogram();
    void saveEnhancedHistogram();
    void saveFalseColor();

signals:
    void beginClicked();       // ��ʼ������ź�
    void progressUpdated(int progress); // ������ȸ����ź�
    void analysisProgressGoing(double elapsedTime);

public:
    // ����դ��ͼ��ΪTIFF�ļ��ĺ���
    void saveRasterAsTiff(GDALDataset* inputDataset, const QString& outputFilePath, int bandRed, int bandGreen, int bandBlue, bool isGray = false, bool isEqualized = false, bool isFalseColor = false);

    // �����������ݣ����ڻҶ�ͼ��ֱ��ͼ���⻯���ĺ���
    void processSingleBand(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height, bool isEqualized);

    // ����RGB�������ݵĺ���
    void processRGBBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height);

    //�ٲ�ɫ��ʾ
    void processFalseColorBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height);
    
    void performGrayscaleProcessing(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height);

    void performHistogramEqualization(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height);

private:
    // UI����
    Ui::rasterBandAnalysisClass mUi;
    std::mutex mMex; // ������
};

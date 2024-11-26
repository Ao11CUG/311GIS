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

    // 栅格文件路径
    QString mstrRasterFilePath;
    //真彩色显示 
    QString mstrTrueColorPath;
    //假彩色显示
    QString mstrFalseColorPath;
    //灰度直方图
    QString mstrGrayHistogramPath;
    //直方图均衡化
    QString mstrEnhancedHistogramPath;

    int mnTotalSteps; // 记录总步骤

public slots:
    // 栅格处理函数
    void processRasterData();  // 进行栅格图像处理并保存结果
    void openFile();           // 打开文件的槽函数
    void saveTrueColor();
    void saveGrayHistogram();
    void saveEnhancedHistogram();
    void saveFalseColor();

signals:
    void beginClicked();       // 开始处理的信号
    void progressUpdated(int progress); // 定义进度更新信号
    void analysisProgressGoing(double elapsedTime);

public:
    // 保存栅格图像为TIFF文件的函数
    void saveRasterAsTiff(GDALDataset* inputDataset, const QString& outputFilePath, int bandRed, int bandGreen, int bandBlue, bool isGray = false, bool isEqualized = false, bool isFalseColor = false);

    // 处理单波段数据（用于灰度图和直方图均衡化）的函数
    void processSingleBand(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height, bool isEqualized);

    // 处理RGB波段数据的函数
    void processRGBBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height);

    //假彩色显示
    void processFalseColorBands(GDALRasterBand* redBand, GDALRasterBand* greenBand, GDALRasterBand* blueBand, GDALRasterBand* outRedBand, GDALRasterBand* outGreenBand, GDALRasterBand* outBlueBand, int width, int height);
    
    void performGrayscaleProcessing(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height);

    void performHistogramEqualization(GDALRasterBand* inBand, GDALRasterBand* outBand, int width, int height);

private:
    // UI对象
    Ui::rasterBandAnalysisClass mUi;
    std::mutex mMex; // 互斥锁
};

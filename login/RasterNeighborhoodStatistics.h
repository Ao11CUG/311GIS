#include <QWidget>
#include <gdal_priv.h>
#include <thread>
#include <vector>
#include <mutex>
#include "ui_RasterNeighborhoodStatistics.h"

class RasterNeighborhoodStatistics : public QWidget
{
    Q_OBJECT

public:
    RasterNeighborhoodStatistics(QWidget* parent = nullptr);
    ~RasterNeighborhoodStatistics();

    void calculateBlockStatistics(int i, int j, int blockSizeX, int blockSizeY, uint8_t* buffer, int nXSize, int nYSize, double& minValue, double& maxValue, double& meanValue);
    void generateStatisticsRaster(int blockSizeX, int blockSizeY, uint8_t* buffer, int nXSize, int nYSize, std::vector<double>& maxRaster, std::vector<double>& minRaster, std::vector<double>& meanRaster);
    void outputStatisticsToCSV(const std::vector<double>& maxRaster, const std::vector<double>& minRaster, const std::vector<double>& meanRaster, int nXSize, int nYSize);
    void generateMaxRasterFile(const std::vector<double>& maxRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY);
    void generateMinRasterFile(const std::vector<double>& minRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY);
    void generateMeanRasterFile(const std::vector<double>& meanRaster, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY);
    GDALDataset* createAndConfigureRaster(const std::string& outputPath, int nXSize, int nYSize, int bandCount, double pixelSizeX, double pixelSizeY);
    void outputBandStatisticsToCSV();

    QString mstrInputRasterPath;//输入栅格路径
    QString mstrOutputCSVPath;//输入CSV文件（像素级）路径
    QString mstrOutputMaxRasterPath;//输出最大值统计栅格路径
    QString mstrOutputMinRasterPath;//输出最小值统计栅格路径
    QString mstrOutputMeanRasterPath;//输出平均值统计栅格路径
    QString mstrOutputWholeCSVPath;//输出CSV文件（整体 分波段）路径

public slots:
    void setInputRasterPath();//选择输入栅格文件
    void setOutputCSVPath();//选择输出CSV文件（输出像素级别 速度慢）
    void setOutputWholeCSVPath();//选择输出CSV文件（整体 分波段）
    void setOutputMaxRasterPath();//选择输出最大值统计栅格文件
    void setOutputMinRasterPath();//选择输出最小值统计栅格文件
    void setOutputMeanRasterPath();//选择输出平均值统计栅格文件
    void neighborhoodStatistics();//执行操作

signals:
    void beginClicked();
    void progressUpdated(int progress);
    void analysisProgressGoing(double elapsedTime);

private:
    Ui::RasterNeighborhoodStatisticsClass mUi;
    std::mutex mMtx; // 互斥锁
};

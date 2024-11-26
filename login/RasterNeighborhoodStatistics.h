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

    QString mstrInputRasterPath;//����դ��·��
    QString mstrOutputCSVPath;//����CSV�ļ������ؼ���·��
    QString mstrOutputMaxRasterPath;//������ֵͳ��դ��·��
    QString mstrOutputMinRasterPath;//�����Сֵͳ��դ��·��
    QString mstrOutputMeanRasterPath;//���ƽ��ֵͳ��դ��·��
    QString mstrOutputWholeCSVPath;//���CSV�ļ������� �ֲ��Σ�·��

public slots:
    void setInputRasterPath();//ѡ������դ���ļ�
    void setOutputCSVPath();//ѡ�����CSV�ļ���������ؼ��� �ٶ�����
    void setOutputWholeCSVPath();//ѡ�����CSV�ļ������� �ֲ��Σ�
    void setOutputMaxRasterPath();//ѡ��������ֵͳ��դ���ļ�
    void setOutputMinRasterPath();//ѡ�������Сֵͳ��դ���ļ�
    void setOutputMeanRasterPath();//ѡ�����ƽ��ֵͳ��դ���ļ�
    void neighborhoodStatistics();//ִ�в���

signals:
    void beginClicked();
    void progressUpdated(int progress);
    void analysisProgressGoing(double elapsedTime);

private:
    Ui::RasterNeighborhoodStatisticsClass mUi;
    std::mutex mMtx; // ������
};

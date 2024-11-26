#pragma once

#include <QWidget>
#include <QString>
#include <gdal_priv.h>
#include <vector>
#include <ogrsf_frmts.h>
#include "ui_MaskExtraction.h"

class MaskExtraction : public QWidget
{
    Q_OBJECT

public:
    MaskExtraction(QWidget *parent = nullptr);
    ~MaskExtraction();

    // 计算矢量数据与栅格数据的有效交集范围，并添加投影信息
    void calculateIntersectionAndSetProjection(GDALDataset* mpPoInputDS, OGRLayer* poLayer, std::vector<OGREnvelope>& polygonEnvelopes);

    void vectorMask(const QString& mstrInputRasterPath, const QString& vectorPath, const QString& outputRasterPathPrefix);

    void rasterMask(const QString& mstrInputRasterPath, const QString& mstrMaskPath, const QString& mstrOutputRasterPath);

    void performMasking(const QString& mstrInputRasterPath, const QString& mstrMaskPath, const QString& outputRasterPathPrefix);

    double mdAdfGeoTransform[6];

    struct TileInfo {
        int x_offset;
        int y_offset;
        int width;
        int height;
        int endColFeature;
        int endRowFeature;
        std::vector<std::vector<GByte*>> dataBuffer;
    };

    std::vector<TileInfo> tileInfos;

    void stitchTilesFromMemory(const std::vector<TileInfo>& tileInfos, const std::vector<OGREnvelope>& polygonEnvelopes, const QString& outputRasterPathPrefix, int bandCount, GDALDataset* mpPoInputDS);

    QString mstrInputRasterPath;//输入栅格文件路径
    QString mstrMaskPath;//掩膜文件路径
    QString mstrOutputRasterPath;//选择输出栅格文件路径

signals:
    void beginClicked();
    void progressUpdated(int progress);
    void analysisProgressGoing(double elapsedTime);

public slots:
    // 槽函数用于选择输入栅格文件路径
    void selectInputRaster();
    // 槽函数用于选择掩膜文件路径
    void selectMask();
    // 槽函数用于选择输出栅格文件路径
    void selectOutputRaster();
    //槽函数用于进行开始运行的函数
    void process();

private:
    Ui::MaskExtractionClass mUi;
    GDALDataset* mpPoInputDS;
};

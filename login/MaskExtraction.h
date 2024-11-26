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

    // ����ʸ��������դ�����ݵ���Ч������Χ�������ͶӰ��Ϣ
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

    QString mstrInputRasterPath;//����դ���ļ�·��
    QString mstrMaskPath;//��Ĥ�ļ�·��
    QString mstrOutputRasterPath;//ѡ�����դ���ļ�·��

signals:
    void beginClicked();
    void progressUpdated(int progress);
    void analysisProgressGoing(double elapsedTime);

public slots:
    // �ۺ�������ѡ������դ���ļ�·��
    void selectInputRaster();
    // �ۺ�������ѡ����Ĥ�ļ�·��
    void selectMask();
    // �ۺ�������ѡ�����դ���ļ�·��
    void selectOutputRaster();
    //�ۺ������ڽ��п�ʼ���еĺ���
    void process();

private:
    Ui::MaskExtractionClass mUi;
    GDALDataset* mpPoInputDS;
};

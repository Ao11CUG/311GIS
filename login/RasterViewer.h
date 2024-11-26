#include <QApplication>
#include <QLabel>
#include <QFileDialog>
#include <gdal_priv.h>

class RasterViewer {
public:
    RasterViewer() {}

    QImage showRaster(const QString& selectedPath) {
        // 初始化 GDAL
        GDALAllRegister();

        // 打开栅格数据
        GDALDataset* poDataset;
        poDataset = (GDALDataset*)GDALOpen(selectedPath.toUtf8().constData(), GA_ReadOnly);
        handleGdalError(CPLGetLastErrorType(), CPLGetLastErrorNo(), CPLGetLastErrorMsg());

        // 获取栅格数据的宽度和高度
        int nXSize = poDataset->GetRasterXSize();
        int nYSize = poDataset->GetRasterYSize();

        // 创建 QImage 对象
        QImage image(nXSize, nYSize, QImage::Format_RGB888);

        // 检查波段数量
        int bandCount = poDataset->GetRasterCount();
        if (bandCount >= 3) {
            // 如果有红、绿、蓝波段
            GByte* redBand = new GByte[nXSize * nYSize];
            GByte* greenBand = new GByte[nXSize * nYSize];
            GByte* blueBand = new GByte[nXSize * nYSize];

            poDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, redBand, nXSize, nYSize, GDT_Byte, 0, 0);
            poDataset->GetRasterBand(2)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, greenBand, nXSize, nYSize, GDT_Byte, 0, 0);
            poDataset->GetRasterBand(3)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, blueBand, nXSize, nYSize, GDT_Byte, 0, 0);

            // 将波段数据写入 QImage
            for (int y = 0; y < nYSize; y++) {
                for (int x = 0; x < nXSize; x++) {
                    int redValue = redBand[y * nXSize + x];
                    int greenValue = greenBand[y * nXSize + x];
                    int blueValue = blueBand[y * nXSize + x];
                    image.setPixel(x, y, qRgb(redValue, greenValue, blueValue));
                }
            }

            // 清理内存
            delete[] redBand;
            delete[] greenBand;
            delete[] blueBand;
        }
        else if (bandCount == 1) {
            // 如果只有一个波段（灰度图）
            GByte* grayBand = new GByte[nXSize * nYSize];
            poDataset->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, nXSize, nYSize, grayBand, nXSize, nYSize, GDT_Byte, 0, 0);

            // 将单一波段数据写入 QImage
            for (int y = 0; y < nYSize; y++) {
                for (int x = 0; x < nXSize; x++) {
                    int grayValue = grayBand[y * nXSize + x];
                    image.setPixel(x, y, qRgb(grayValue, grayValue, grayValue)); // 将灰度值设置为 RGB
                }
            }

            // 清理内存
            delete[] grayBand;
        }
        else {
            qWarning() << "No valid bands found in the raster dataset.";
        }

        GDALClose(poDataset);
        return image;
    }

private:
    // 处理 GDAL 错误的函数
    void handleGdalError(CPLErr errClass, int errNo, const char* msg) {
        if (errClass != CE_None) {
            QString errorMessage = QString("GDAL Error: %1 (%2): %3").arg(errClass).arg(errNo).arg(QString::fromUtf8(msg));
            qCritical() << errorMessage;
            exit(1);
        }
    }
};

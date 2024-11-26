#include <QMainWindow>
#include <QAction>
#include "ui_mainwindow.h"
#include "VectorLayerRenderer.h" 
#include "RasterViewer.h"
#include "MaskExtraction.h"
#include "LayeredCanvas.h"
#include "GeoJsonInput.h"
#include "VectorLayerBufferAnalysis.h"
#include "WKTInput.h"
#include "StatisticAnalysis.h"
#include "RasterBandAnalysis.h"
#include "Triangulation.h"
#include "RasterNeighborhoodStatistics.h"
#include "VoronoiAnalysis.h"
#include "ConvexAnalysis.h"
#include "UserTips.h"
#include "TravelQuYuan.h"
#include <QMap>
#include <QListView>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include <QList>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <Command.h>

//记录操作
struct Action {
    enum Type { Add, Remove };
    Type type;
    QStringList fileNames;  // 使用 QStringList 存储多个文件名
};

// 定义图层信息结构
struct LayerInfo {
    QString filePath;
    QString type; // "shp", "tif", "geojson", "csv", etc.
    bool visibility;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void logMessage(const QString& message);

    int getIndexByFileName(const QString& fileName) const;

public slots:
    void removeSelectedItem();
    void openFile();
    void saveFile();
    void saveProject();
    void loadProject();
    void restoreSelectedLayerPosition();
    void openLayersManage();
    void openToolBox();
    void showMaskExtraction();
    void toggleLayerVisibility(QStandardItem* item);
    void showVectorLayerBufferAnalysis();
    void showStatisticAnalysis();
    void showRasterBandAnalysis();
    void showTriangulation();
    void showRasterNeighborhoodStatistics();
    void showVoronoiAnalysis();
    void showUserTips();
    void showTravelQuYuan();
    void searchButtons();
    void handleSearchResultClick(const QModelIndex& index);
    void switchToLightMode();
    void switchToDarkMode();
    void switchToBlueMode();
    void startEditMode();
    void endEditMode();
    void changeColor();
    void paintMode();
    void changePenColor();
    void changePenWidth();
    void on_drawPolylineAct_triggered();
    void on_drawEllipseAct_triggered();
    void on_drawRectangleAct_triggered();
    void on_moveAct_triggered();
    void clearLayer();
    void undo();
    void redo();
    void clearLastGraphic();
    void onAnalysisProgressUpdated(int progress);
    void onAnalysisProgressGoing(double elapsedTime);
    void showConvexHullAnalysis();

private:
    LayeredCanvas* mpCanvas;
    QList<LayerInfo> mvLayers; // 存储图层信息
    Ui::mainwindowClass mUi;

    QStack<Action> mvActionHistory; // 操作历史记录
    QStack<Action> mvRedoHistory;

    QMap<int, QImage> mMapImageIndex; // 存储索引与 QImage 的映射
    QMap<int, QString> mMapFileIndex; // 存储文件名和索引的映射

    QStandardItemModel* mpSearchedToolModel;
    bool mbIsPaintModeActive = false;// 定义一个成员变量来记录绘画模式是否开启
};



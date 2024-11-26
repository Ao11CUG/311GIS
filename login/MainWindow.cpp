#include "MainWindow.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QColorDialog>
#include <qstack.h>
#include <QDateTime>
#include <QObject>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    mpCanvas(nullptr) // 初始化 canvas 为 nullptr
{
    mUi.setupUi(this);

    logMessage("Welcome to use 311GIS system!");

    setWindowTitle("311GIS v1.0");
    setWindowIcon(QIcon(":/login/res/311GIS.png"));

    mUi.leftDock1->setWindowTitle("Layer manager");
    mUi.leftDock2->setWindowTitle("Toolbox");

    switchToBlueMode();

    connect(mUi.openFile, &QAction::triggered, this, &MainWindow::openFile);
    connect(mUi.saveFile, &QAction::triggered, this, &MainWindow::saveFile);
    connect(mUi.openProjectAct, &QAction::triggered, this, &MainWindow::loadProject);
    connect(mUi.saveProjectAct, &QAction::triggered, this, &MainWindow::saveProject);
    connect(mUi.deleteFile, &QPushButton::clicked, this, &MainWindow::removeSelectedItem);
    connect(mUi.clearAct, &QAction::triggered, this, &MainWindow::clearLayer);
    connect(mUi.replaceButton, &QPushButton::clicked, this, &MainWindow::restoreSelectedLayerPosition);
    connect(mUi.openLayersManageAct, &QAction::triggered, this, &MainWindow::openLayersManage);
    connect(mUi.openToolBox, &QAction::triggered, this, &MainWindow::openToolBox);
    connect(mUi.maskExtrationAct, &QAction::triggered, this, &MainWindow::showMaskExtraction);
    connect(mUi.maskExtrationButton, &QPushButton::clicked, this, &MainWindow::showMaskExtraction);
    connect(mUi.bufferAnalysisAct, &QAction::triggered, this, &MainWindow::showVectorLayerBufferAnalysis);
    connect(mUi.bufferAnalysisButton, &QPushButton::clicked, this, &MainWindow::showVectorLayerBufferAnalysis);
    connect(mUi.statisticAnalysisAct, &QAction::triggered, this, &MainWindow::showStatisticAnalysis);
    connect(mUi.statisticAnalysisButton, &QPushButton::clicked, this, &MainWindow::showStatisticAnalysis);
    connect(mUi.rasterBandAnalysisAct, &QAction::triggered, this, &MainWindow::showRasterBandAnalysis);
    connect(mUi.rasterBandAnalysisButton, &QPushButton::clicked, this, &MainWindow::showRasterBandAnalysis);
    connect(mUi.delaunayAct, &QAction::triggered, this, &MainWindow::showTriangulation);
    connect(mUi.delaunayButton, &QPushButton::clicked, this, &MainWindow::showTriangulation);
    connect(mUi.domainAnalysisAct, &QAction::triggered, this, &MainWindow::showRasterNeighborhoodStatistics);
    connect(mUi.domainAnalysisButton, &QPushButton::clicked, this, &MainWindow::showRasterNeighborhoodStatistics);
    connect(mUi.voronoiAnalysisAct, &QAction::triggered, this, &MainWindow::showVoronoiAnalysis);
    connect(mUi.voronoiAnalysisButton, &QPushButton::clicked, this, &MainWindow::showVoronoiAnalysis);
    connect(mUi.convexHullAnalysisAct, &QAction::triggered, this, &MainWindow::showConvexHullAnalysis);
    connect(mUi.convexHullAnalysisButton, &QPushButton::clicked, this, &MainWindow::showConvexHullAnalysis);
    connect(mUi.actionUserTips, &QAction::triggered, this, &MainWindow::showUserTips);
    connect(mUi.actionQuYuan, &QAction::triggered, this, &MainWindow::showTravelQuYuan);

    // 创建一个 QStandardItemModel 并设置给 listView
    QStandardItemModel* model = new QStandardItemModel(this);
    mUi.listView->setModel(model);
    connect(model, &QStandardItemModel::itemChanged, this, &MainWindow::toggleLayerVisibility);

    // 搜索功能
    mpSearchedToolModel = new QStandardItemModel(this);
    mUi.searchedTool->setModel(mpSearchedToolModel);

    connect(mUi.searchButton, &QPushButton::clicked, this, &MainWindow::searchButtons);
    connect(mUi.searchedTool, &QListView::clicked, this, &MainWindow::handleSearchResultClick);

    // 主题切换动作
    connect(mUi.lightModeAct, &QAction::triggered, this, &MainWindow::switchToLightMode);
    connect(mUi.darkModeAct, &QAction::triggered, this, &MainWindow::switchToDarkMode);
    connect(mUi.blueModeAct, &QAction::triggered, this, &MainWindow::switchToBlueMode);

    //编辑模式更改
    mUi.changeColorAct->setVisible(false);
    mUi.paintModeAct->setVisible(false);
    mUi.drawPolylineAct->setVisible(false);
    mUi.drawEllipseAct->setVisible(false);
    mUi.drawRectangleAct->setVisible(false);
    mUi.moveAct->setVisible(false);
    mUi.clearAct->setVisible(false);
    mUi.changePenColorAct->setVisible(false);
    mUi.changeThicknessAct->setVisible(false);
    mUi.clearLastGraphicAct->setVisible(false);

    connect(mUi.beginEdit, &QAction::triggered, this, &MainWindow::startEditMode);
    connect(mUi.endEdit, &QAction::triggered, this, &MainWindow::endEditMode);
    connect(mUi.changeColorAct, &QAction::triggered, this, &MainWindow::changeColor);
    connect(mUi.paintModeAct, &QAction::triggered, this, &MainWindow::paintMode);
    connect(mUi.changePenColorAct, &QAction::triggered, this, &MainWindow::changePenColor);
    connect(mUi.changeThicknessAct, &QAction::triggered, this, &MainWindow::changePenWidth);
    connect(mUi.clearLastGraphicAct, &QAction::triggered, this, &MainWindow::clearLastGraphic);

    // 撤销和重做的 QAction
    mUi.undoAct->setVisible(true);
    mUi.redoAct->setVisible(true);
    connect(mUi.undoAct, &QAction::triggered, this, &MainWindow::undo);
    connect(mUi.redoAct, &QAction::triggered, this, &MainWindow::redo);

    // 显示进度条
    mUi.progressBar->setVisible(true);
    // 设置进度条范围
    mUi.progressBar->setRange(0, 100);
    mUi.progressBar->setValue(0); // 初始化进度
}

MainWindow::~MainWindow()
{
}

//----------基础功能----------

//打开文件
void MainWindow::openFile() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open File"),
        "/home",
        tr("File (*.shp *.tif *.geojson *.json *.csv)"));;

    if (fileName.isEmpty())
        return;

    logMessage("Opening file: " + fileName);

    QFileInfo fileInfo(fileName);
    QString suffix = fileInfo.suffix().toLower();

    // 确保 scrollArea 内部是 LayeredCanvas
    if (!mpCanvas) {
        mpCanvas = new LayeredCanvas(this);
        mUi.scrollArea->setWidget(mpCanvas);
        mUi.scrollArea->setWidgetResizable(true);
    }

    QImage loadedImage; // 新增变量用于存储加载的图像

    if (suffix == "shp") {
        VectorLayerRenderer renderer(fileName);
        loadedImage = renderer.renderToImage(); // 渲染 SHP 文件为 QImage
        mpCanvas->addLayer(loadedImage); // 添加图层，并传入文件名以便获取坐标范围
    }
    else if (suffix == "tif") {
        RasterViewer rasterV;
        loadedImage = rasterV.showRaster(fileName);
        mpCanvas->addLayer(loadedImage);
    }
    else if (suffix == "geojson" || suffix == "json") {
        GeoJsonInput renderer(fileName);
        loadedImage = renderer.renderToImage();
        mpCanvas->addLayer(loadedImage);
    }
    else if (suffix == "csv") {
        WKTInput WKT;
        loadedImage = WKT.visualizeWkt(fileName);
        mpCanvas->addLayer(loadedImage);
    }

    // 根据后缀名选择图标
    QIcon fileIcon;
    if (suffix == "shp") {
        fileIcon = QIcon(":/login/res/vector.png");
    }
    else if (suffix == "tif") {
        fileIcon = QIcon(":/login/res/grid.png");
    }
    else if (suffix == "geojson" || suffix == "json") {
        fileIcon = QIcon(":/login/res/GeoJson.png");
    }
    else if (suffix == "csv") {
        fileIcon = QIcon(":/login/res/WKT.png");
    }

    // 将文件名添加到 listView 的 model 中
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());

    QString baseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(fileIcon, baseName);
    item->setCheckable(true); // 设置复选框
    item->setCheckState(Qt::Checked); // 默认选中
    model->appendRow(item);

    // 获取新添加的项的索引
    int index = model->rowCount() - 1;
    mMapFileIndex[index] = fileName; // 存储文件名和索引
    mMapImageIndex[index] = loadedImage; // 存储 QImage 和索引的对应关系

    // 记录操作
    Action action;
    action.type = Action::Add;
    action.fileNames << fileName;
    mvActionHistory.push(action);

    logMessage("File added to list: " + baseName);
    logMessage("File opened successfully: " + fileName);
}

//保存文件
void MainWindow::saveFile() {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (!model)
        return;

    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    QString newFileName = QFileDialog::getSaveFileName(
        this,
        tr("Save File"),
        QDir::homePath(),
        tr("Shapefiles (*.shp);;TIFF (*.tif);;GeoJSON (*.geojson);;JSON (*.json);;CSV (*.csv);;PNG (*.png);;JPG (*.jpg);;All (*)")
    );

    if (newFileName.isEmpty())
        return;

    logMessage("Saving file as: " + newFileName);

    // 处理图像保存逻辑
    QString imageSuffix = QFileInfo(newFileName).suffix().toLower();

    if (imageSuffix == "png" || imageSuffix == "jpg" || imageSuffix == "jpeg") {
        // 处理图像保存逻辑
        foreach(const QModelIndex & index, selectedIndexes) {
            QString originalFileName = mMapFileIndex[index.row()];
            QString imageSuffix = QFileInfo(newFileName).suffix().toLower();

            // 保存 QImage 为 PNG 或 JPG
            if (imageSuffix == "png" || imageSuffix == "jpg" || imageSuffix == "jpeg") {
                QImage imageToSave = mMapImageIndex[index.row()]; // 获取对应的 QImage
                if (imageToSave.isNull()) {
                    logMessage("Image at index " + QString::number(index.row()) + " is null.");
                }
                else {
                    logMessage("Image at index " + QString::number(index.row()) + " retrieved successfully.");
                }
                if (!imageToSave.isNull()) {
                    bool saveSuccess = false;
                    if (imageSuffix == "png") {
                        saveSuccess = imageToSave.save(newFileName, "PNG");
                    }
                    else if (imageSuffix == "jpg" || imageSuffix == "jpeg") {
                        saveSuccess = imageToSave.save(newFileName, "JPG");
                    }
                    if (saveSuccess) {
                        logMessage("Image saved successfully: " + newFileName);
                    }
                    else {
                        logMessage("Failed to save image: " + newFileName + " - Error saving image.");
                    }
                }
                else {
                    logMessage("Failed to retrieve image for saving: " + newFileName);
                }
            }
        }
    }
    else {
        foreach(const QModelIndex & index, selectedIndexes) {
            QStandardItem* item = model->itemFromIndex(index);
            if (item) {
                QString originalFileName = mMapFileIndex[index.row()];
                QString suffix = QFileInfo(originalFileName).suffix().toLower();

                // 保存 SHP 文件及其相关文件
                if (suffix == "shp") {
                    QString baseName = QFileInfo(newFileName).baseName();

                    // 复制主文件
                    QString newShpName = newFileName;
                    if (!newShpName.endsWith(".shp", Qt::CaseInsensitive)) {
                        newShpName += ".shp";
                    }
                    QFile::copy(originalFileName, newShpName);

                    // 复制相关文件
                    QStringList extensions = { ".cpg", ".dbf", ".prj", ".sbn", ".sbx", ".shx", ".xml" };
                    foreach(const QString & ext, extensions) {
                        QString originalRelatedFileName = QFileInfo(originalFileName).absolutePath() + "/" + QFileInfo(originalFileName).baseName() + ext;
                        QString newRelatedFileName = QFileInfo(newFileName).absolutePath() + "/" + baseName + ext;
                        QFile::copy(originalRelatedFileName, newRelatedFileName);
                    }

                    logMessage("SHP file and its related files saved successfully: " + newFileName);
                }
                // 处理其他文件类型
                else if (suffix == "tif") {
                    if (!newFileName.endsWith(".tif", Qt::CaseInsensitive)) {
                        newFileName += ".tif";
                    }
                    QFile::copy(originalFileName, newFileName);
                    logMessage("TIFF file saved successfully: " + newFileName);
                }
                else if (suffix == "geojson" || suffix == "json") {
                    if (!newFileName.endsWith(".geojson", Qt::CaseInsensitive) && !newFileName.endsWith(".json", Qt::CaseInsensitive)) {
                        newFileName += ".geojson"; // 默认添加 .geojson 后缀
                    }
                    QFile::copy(originalFileName, newFileName);
                    logMessage("GeoJSON/JSON file saved successfully: " + newFileName);
                }
                else if (suffix == "csv") {
                    if (!newFileName.endsWith(".csv", Qt::CaseInsensitive)) {
                        newFileName += ".csv";
                    }
                    QFile::copy(originalFileName, newFileName);
                    logMessage("CSV file saved successfully: " + newFileName);
                }
            }
        }
    }

}

//保存工程文件
void MainWindow::saveProject() {
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Project"),
        QDir::homePath(),
        tr("XML (*.xml);;All Files (*)")
    );

    if (filePath.isEmpty()) {
        logMessage("Project save cancelled.");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        logMessage("Failed to save project: " + file.errorString());
        return;
    }

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("Project");

    // 保存所有现存的图层
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (model) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QStandardItem* item = model->item(row);
            if (item) {
                LayerInfo layer;
                layer.filePath = mMapFileIndex[row]; // 根据索引获取文件路径
                layer.type = QFileInfo(layer.filePath).suffix(); // 使用后缀作为类型
                xmlWriter.writeStartElement("Layer");
                xmlWriter.writeTextElement("FilePath", layer.filePath);
                xmlWriter.writeTextElement("Type", layer.type);
                xmlWriter.writeEndElement(); // 结束 Layer
            }
        }
    }

    xmlWriter.writeEndElement(); // 结束 Project
    xmlWriter.writeEndDocument();

    file.close();
    logMessage("Project saved successfully: " + filePath);
}

//打开工程文件
void MainWindow::loadProject() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Project"),
        QDir::homePath(),
        tr("XML (*.xml);;All Files (*)")
    );

    if (filePath.isEmpty()) {
        logMessage("Project load cancelled.");
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        logMessage("Failed to load project: " + file.errorString());
        return;
    }

    QXmlStreamReader xmlReader(&file);
    mvLayers.clear(); // 清空之前的图层信息

    //记录操作
    Action action;
    action.type = Action::Add;

    while (!xmlReader.atEnd() && !xmlReader.hasError()) {
        xmlReader.readNext();

        if (xmlReader.isStartElement()) {
            if (xmlReader.name() == QString("Layer")) {
                LayerInfo layer;
                while (!(xmlReader.isEndElement() && xmlReader.name() == QString("Layer"))) {
                    xmlReader.readNext();
                    if (xmlReader.isStartElement()) {
                        if (xmlReader.name() == QString("FilePath")) {
                            layer.filePath = xmlReader.readElementText();
                            action.fileNames << layer.filePath;
                        }
                        else if (xmlReader.name() == QString("Type")) {
                            layer.type = xmlReader.readElementText();
                        }
                    }
                }
                mvLayers.append(layer); // 将图层信息添加到列表中
            }
        }
    }

    if (xmlReader.hasError()) {
        logMessage("XML error: " + xmlReader.errorString());
    }
    else {
        logMessage("Project loaded successfully: " + filePath);
    }

    mvActionHistory.push(action);

    file.close();

    // 根据读取的图层信息更新 UI 和图层
    for (const LayerInfo& layer : mvLayers) {
        // 根据文件类型调用不同的加载逻辑
        QString suffix = QFileInfo(layer.filePath).suffix().toLower();

        // 确保 scrollArea 内部是 LayeredCanvas
        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(true);
        }

        QImage loadedImage; // 新增变量用于存储加载的图像

        if (suffix == "shp") {
            VectorLayerRenderer renderer(layer.filePath);
            loadedImage = renderer.renderToImage(); // 渲染 SHP 文件为 QImage
            mpCanvas->addLayer(loadedImage); // 添加图层
        }
        else if (suffix == "tif") {
            RasterViewer rasterV;
            loadedImage = rasterV.showRaster(layer.filePath);
            mpCanvas->addLayer(loadedImage);
        }
        else if (suffix == "geojson" || suffix == "json") {
            GeoJsonInput renderer(layer.filePath);
            loadedImage = renderer.renderToImage();
            mpCanvas->addLayer(loadedImage);
        }
        else if (suffix == "csv") {
            WKTInput WKT;
            loadedImage = WKT.visualizeWkt(layer.filePath);
            mpCanvas->addLayer(loadedImage);
        }

        // 根据后缀名选择图标
        QIcon fileIcon;
        if (suffix == "shp") {
            fileIcon = QIcon(":/login/res/vector.png");
        }
        else if (suffix == "tif") {
            fileIcon = QIcon(":/login/res/grid.png");
        }
        else if (suffix == "geojson" || suffix == "json") {
            fileIcon = QIcon(":/login/res/GeoJson.png");
        }
        else if (suffix == "csv") {
            fileIcon = QIcon(":/login/res/WKT.png");
        }

        // 将文件名添加到 listView 的 model 中
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        QString baseName = QFileInfo(layer.filePath).fileName();
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // 设置复选框
        item->setCheckState(Qt::Checked); // 默认选中
        model->appendRow(item);

        // 获取新添加的项的索引
        int index = model->rowCount() - 1;
        mMapFileIndex[index] = layer.filePath; // 存储文件名和索引
        mMapImageIndex[index] = loadedImage; // 存储 QImage 和索引的对应关系

        logMessage("File added to list: " + baseName);
        logMessage("File opened successfully: " + layer.filePath);
    }
}

//打开图层管理器
void MainWindow::openLayersManage() {
    logMessage("Opening layer management dialog.");

    if (mUi.leftDock1 && !mUi.leftDock1->isVisible()) {
        mUi.leftDock1->setVisible(true);
    }

    logMessage("Layer management dialog opened successfully.");
}

//打开工具箱
void MainWindow::openToolBox() {
    logMessage("Opening ToolBox.");

    if (mUi.leftDock2 && !mUi.leftDock2->isVisible()) {
        mUi.leftDock2->setVisible(true);
    }

    logMessage("ToolBox opened successfully");
}

//删除功能
void MainWindow::removeSelectedItem() {
    // 获取当前 ListView 的 model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (!model)
        return;

    // 获取选中的项的索引
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    // 获取选中项的行号
    int row = selectedIndexes.first().row();

    QString removedFileName = mMapFileIndex.value(row);

    // 从 listView 的 model 中移除选中的项
    model->removeRow(row);

    // 检查并从 QMap 中移除相应的条目
    if (mMapFileIndex.contains(row)) {
        mMapFileIndex.remove(row);
        logMessage("Removed file index mapping for row: " + QString::number(row));
    }
    if (mMapImageIndex.contains(row)) {
        mMapImageIndex.remove(row);
        logMessage("Removed image index mapping for row: " + QString::number(row));
    }

    // 从 LayeredCanvas 中移除对应的图层
    if (mpCanvas) {
        mpCanvas->removeLayer(row);
    }

    logMessage("Selected item " + QString::number(row + 1) + " removed.");

    // 更新 mMapFileIndex 和 mMapImageIndex 中的索引
    auto updateIndexMap = [](QMap<int, QString>& indexMap, int removedRow) {
        QMap<int, QString> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QString value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    auto updateImageIndexMap = [](QMap<int, QImage>& indexMap, int removedRow) {
        QMap<int, QImage> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QImage value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    updateIndexMap(mMapFileIndex, row);
    updateImageIndexMap(mMapImageIndex, row);

    // 记录操作
    Action action;
    action.type = Action::Remove;
    action.fileNames << removedFileName;
    mvActionHistory.push(action);

    logMessage("Index maps updated after removal.");
}

// 实现获取索引的函数
int MainWindow::getIndexByFileName(const QString& fileName) const
{
    // 查找 mMapFileIndex 中是否存在对应的文件名
    if (mMapFileIndex.values().contains(fileName)) {
        // 获取第一个匹配的文件名的键值（即索引）
        return mMapFileIndex.key(fileName);
    }
    else {
        // 如果没有找到，返回 -1 表示文件名不存在
        return -1;
    }
}

//撤销功能
void MainWindow::undo() {
    if (mvActionHistory.isEmpty()) {
        logMessage("No actions to undo.");
        return;
    }

    Action lastAction = mvActionHistory.pop(); // 获取最后一项操作
    mvRedoHistory.push(lastAction); // 将最后一项操作压入重做历史栈

    // 更新 mMapFileIndex 和 mMapImageIndex 中的索引
    auto updateIndexMap = [](QMap<int, QString>& indexMap, int removedRow) {
        QMap<int, QString> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QString value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    auto updateImageIndexMap = [](QMap<int, QImage>& indexMap, int removedRow) {
        QMap<int, QImage> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QImage value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    switch (lastAction.type) {
    case Action::Add: {
        // 如果是添加操作，移除最后添加的图层
        for (const QString& fileName : lastAction.fileNames) {
            int row = getIndexByFileName(fileName);

            // 获取当前 ListView 的 model
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (!model)
                return;

            // 从 listView 的 model 中移除选中的项
            model->removeRow(row);

            // 检查并从 QMap 中移除相应的条目
            if (mMapFileIndex.contains(row)) {
                mMapFileIndex.remove(row);
            }
            if (mMapImageIndex.contains(row)) {
                mMapImageIndex.remove(row);
            }

            // 从 LayeredCanvas 中移除对应的图层
            if (mpCanvas) {
                mpCanvas->removeLayer(row);
            }


            updateIndexMap(mMapFileIndex, row);
            updateImageIndexMap(mMapImageIndex, row);
        }
        break;
    }
    case Action::Remove: {
        // 如果是移除操作，重新添加这些图层
        for (const QString& fileName : lastAction.fileNames) {
            QFileInfo fileInfo(fileName);
            QString suffix = fileInfo.suffix().toLower();

            // 确保 scrollArea 内部是 LayeredCanvas
            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(true);
            }

            QImage loadedImage; // 新增变量用于存储加载的图像

            if (suffix == "shp") {
                VectorLayerRenderer renderer(fileName);
                loadedImage = renderer.renderToImage(); // 渲染 SHP 文件为 QImage
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "tif") {
                RasterViewer rasterV;
                loadedImage = rasterV.showRaster(fileName);
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "geojson" || suffix == "json") {
                GeoJsonInput renderer(fileName);
                loadedImage = renderer.renderToImage();
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "csv") {
                WKTInput WKT;
                loadedImage = WKT.visualizeWkt(fileName);
                mpCanvas->addLayer(loadedImage);
            }

            // 根据后缀名选择图标
            QIcon fileIcon;
            if (suffix == "shp") {
                fileIcon = QIcon(":/login/res/vector.png");
            }
            else if (suffix == "tif") {
                fileIcon = QIcon(":/login/res/grid.png");
            }
            else if (suffix == "geojson" || suffix == "json") {
                fileIcon = QIcon(":/login/res/GeoJson.png");
            }
            else if (suffix == "csv") {
                fileIcon = QIcon(":/login/res/WKT.png");
            }

            // 将文件名添加到 listView 的 model 中
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            QString baseName = fileInfo.fileName();
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            model->appendRow(item);

            // 获取新添加的项的索引
            int index = model->rowCount() - 1;
            mMapFileIndex[index] = fileName;
            mMapImageIndex[index] = loadedImage;
        }
        break;
    }
    }
    logMessage("Undo successfully.");
}

//重做功能
void MainWindow::redo() {
    if (mvRedoHistory.isEmpty()) {
        logMessage("No actions to redo.");
        return;
    }

    Action lastAction = mvRedoHistory.pop(); // 获取最后一项重做操作

    // 更新 mMapFileIndex 和 mMapImageIndex 中的索引
    auto updateIndexMap = [](QMap<int, QString>& indexMap, int removedRow) {
        QMap<int, QString> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QString value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    auto updateImageIndexMap = [](QMap<int, QImage>& indexMap, int removedRow) {
        QMap<int, QImage> updatedMap;
        for (auto it = indexMap.begin(); it != indexMap.end(); ++it) {
            int currentIndex = it.key();
            QImage value = it.value();
            if (currentIndex > removedRow) {
                updatedMap[currentIndex - 1] = value;
            }
            else {
                updatedMap[currentIndex] = value;
            }
        }
        indexMap = updatedMap;
        };

    switch (lastAction.type) {
    case Action::Add: {
        for (const QString& fileName : lastAction.fileNames) {
            QFileInfo fileInfo(fileName);
            QString suffix = fileInfo.suffix().toLower();

            // 确保 scrollArea 内部是 LayeredCanvas
            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(true);
            }

            QImage loadedImage; // 新增变量用于存储加载的图像

            if (suffix == "shp") {
                VectorLayerRenderer renderer(fileName);
                loadedImage = renderer.renderToImage(); // 渲染 SHP 文件为 QImage
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "tif") {
                RasterViewer rasterV;
                loadedImage = rasterV.showRaster(fileName);
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "geojson" || suffix == "json") {
                GeoJsonInput renderer(fileName);
                loadedImage = renderer.renderToImage();
                mpCanvas->addLayer(loadedImage);
            }
            else if (suffix == "csv") {
                WKTInput WKT;
                loadedImage = WKT.visualizeWkt(fileName);
                mpCanvas->addLayer(loadedImage);
            }

            // 根据后缀名选择图标
            QIcon fileIcon;
            if (suffix == "shp") {
                fileIcon = QIcon(":/login/res/vector.png");
            }
            else if (suffix == "tif") {
                fileIcon = QIcon(":/login/res/grid.png");
            }
            else if (suffix == "geojson" || suffix == "json") {
                fileIcon = QIcon(":/login/res/GeoJson.png");
            }
            else if (suffix == "csv") {
                fileIcon = QIcon(":/login/res/WKT.png");
            }

            // 将文件名添加到 listView 的 model 中
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            QString baseName = fileInfo.fileName();
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            model->appendRow(item);

            // 获取新添加的项的索引
            int index = model->rowCount() - 1;
            mMapFileIndex[index] = fileName;
            mMapImageIndex[index] = loadedImage;
        }
        break;
    }
    case Action::Remove: {
        for (const QString& fileName : lastAction.fileNames) {
            int row = getIndexByFileName(fileName);

            // 获取当前 ListView 的 model
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (!model)
                return;

            // 从 listView 的 model 中移除选中的项
            model->removeRow(row);

            // 检查并从 QMap 中移除相应的条目
            if (mMapFileIndex.contains(row)) {
                mMapFileIndex.remove(row);
            }
            if (mMapImageIndex.contains(row)) {
                mMapImageIndex.remove(row);
            }

            // 从 LayeredCanvas 中移除对应的图层
            if (mpCanvas) {
                mpCanvas->removeLayer(row);
            }


            updateIndexMap(mMapFileIndex, row);
            updateImageIndexMap(mMapImageIndex, row);
        }
        break;
    }
    }
    logMessage("Redo successfully.");
}

//复位功能
void MainWindow::restoreSelectedLayerPosition() {
    // 获取选中的项
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to restore position.");
        return; // 没有选中图层，返回
    }

    // 假设只恢复第一个选中的项
    int index = selectedIndexes.first().row();

    // 调用 LayeredCanvas 的方法恢复图层位置
    mpCanvas->restoreAllPositions(index);

    logMessage("Restored position of layer: " + QString::number(index + 1));
}

// 隐藏功能槽函数
void MainWindow::toggleLayerVisibility(QStandardItem* item) {
    if (!item)
        return;

    int row = item->row();
    if (mpCanvas) {
        bool isVisible = (item->checkState() == Qt::Checked);
        mpCanvas->setLayerVisible(row, isVisible);
    }

    if (item->checkState() == Qt::Checked) {
        logMessage("Layer " + item->text() + " is now visible.");
    }
    else {
        logMessage("Layer " + item->text() + " is now hidden.");
    }
}

//搜索功能
void MainWindow::searchButtons() {
    QString searchText = mUi.searchEdit->text().trimmed();

    // 如果搜索内容为空，不执行搜索
    if (searchText.isEmpty()) {
        logMessage("Search text is empty. Please enter a valid text.");
        // 清空之前的搜索结果
        mpSearchedToolModel->clear();
        return;
    }

    logMessage("Searching for buttons with text: " + searchText);

    // 清空之前的搜索结果
    mpSearchedToolModel->clear();

    QList<QPushButton*> allButtons = findChildren<QPushButton*>();
    bool found = false;
    for (QPushButton* button : allButtons) {
        if (button->text().contains(searchText, Qt::CaseInsensitive) && button->text() != "Delete" && button->text() != "Replace") {
            QStandardItem* item = new QStandardItem(button->text());
            mpSearchedToolModel->appendRow(item);
            found = true;
        }
    }

    if (!found) {
        logMessage("No buttons found with the text: " + searchText);
    }
}

//模拟按钮被点击
void MainWindow::handleSearchResultClick(const QModelIndex& index) {
    QString buttonText = mpSearchedToolModel->itemFromIndex(index)->text();
    QList<QPushButton*> allButtons = findChildren<QPushButton*>();
    QPushButton* button = nullptr;
    for (QPushButton* btn : allButtons) {
        if (btn->text() == buttonText) {
            button = btn;
            break;
        }
    }
    if (button) {
        button->click();
    }
    else {
        QString errorMsg = "Button not found: " + buttonText;
        logMessage(errorMsg);
        QMessageBox::warning(this, "Error", errorMsg);
    }
}

//日志输出
void MainWindow::logMessage(const QString& message) {
    if (mUi.log) {
        // 获取当前时间
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        // 将当前时间与消息连接
        QString formattedMessage = QString("[%1] %2").arg(currentTime, message);
        // 输出日志
        mUi.log->append(formattedMessage);
    }
}

//用户提示
void MainWindow::showUserTips() {
    UserTips* userTips = new UserTips();
    userTips->show();

    logMessage("Show User Tips successfully.");
}

//屈原故里旅游地图
void MainWindow::showTravelQuYuan() {
    TravelQuYuan* travelQuYuan = new TravelQuYuan();
    travelQuYuan->show();

    logMessage("Show tourist map of QuYuan's hometown successfully.");
}

//程序运行进度
void MainWindow::onAnalysisProgressUpdated(int progress) {
    // 假设您有一个进度条或其他 UI 元素来显示进度
    mUi.progressBar->setValue(progress); // 更新进度条
}

//程序运行时间
void MainWindow::onAnalysisProgressGoing(double elapsedTime) {
    mUi.time->setText(QString("%1 s").arg(elapsedTime));
}


//----------切换样式----------

//原生蓝色模式
void MainWindow::switchToBlueMode() {
    QString blueModeStylesheet = R"(
        QMainWindow {
            background-color: rgb(173, 216, 230); 
            color: #000000;
        }
        QDockWidget#leftDock1 {
            background-color: rgb(173, 216, 230); 
        }
        QDockWidget#leftDock2 {
            background-color: rgb(173, 216, 230); 
        }
        QListView#listView {
            background-color: #ffffff; 
        }
        QPushButton#deleteFile {
            background-color: #ffffff; 
        }
        QPushButton#replaceButton {
            background-color: #ffffff; 
        }
        QLineEdit#searchEdit {
            background-color: #ffffff; 
        }
        QPushButton#searchButton {
            background-color: #ffffff; 
        }
        QListView#searchedTool {
            background-color: #ffffff; 
        }
        QTabWidget#toolBox {
            background-color: #ffffff; 
        }
        QToolBar#mainToolBar {
            background-color: rgb(173, 216, 230); 
        }
        QMenuBar#menuBar {
            background-color: rgb(135, 206, 235);
        }
    )";

    qApp->setStyleSheet(""); // 清除当前窗口的样式
    qApp->setStyleSheet(blueModeStylesheet); // 设置新样式

    logMessage("Switched to original blue mode.");
}

//白天模式
void MainWindow::switchToLightMode() {
    QString lightModeStylesheet = R"(
        QMainWindow {
            background-color: #ffffff;
            color: #000000;
        }
        QDockWidget#leftDock1 {
            background-color: #ffffff;
        }
        QDockWidget#leftDock2 {
            background-color: #ffffff;
        }
        QListView#listView {
            background-color: #ffffff;
        }
        QPushButton#deleteFile {
            background-color: #ffffff;
        }
        QPushButton#replaceButton {
            background-color: #ffffff;
        }
        QLineEdit#searchEdit {
            background-color: #ffffff;
        }
        QPushButton#searchButton {
            background-color: #ffffff;
        }
        QListView#searchedTool {
            background-color: #ffffff;
        }
        QTabWidget#toolBox {
            background-color: #ffffff;
        }
        QToolBar#mainToolBar {
            background-color: #ffffff;
        }
        QMenuBar#menuBar {
            background-color: #ffffff;
        }
    )";


    // 清除之前的样式表
    qApp->setStyleSheet(""); // 清除当前窗口的样式
    qApp->setStyleSheet(lightModeStylesheet); // 设置新样式

    logMessage("Switched to daytime mode.");
}

//黑夜模式
void MainWindow::switchToDarkMode() {
    QString darkModeStylesheet = R"(
        QMainWindow { 
            background-color: #2d2d2d; color: #ffffff; 
        }
        QMenuBar#menuBar {
            color: #ffffff
        }
        QProgressBar#progressBar {
            color: #ffffff
        }
        QLabel#time {
            color: #ffffff
        }
        QToolBar#mainToolBar {
            background-color: rgb(211, 211, 211); 
        }
        QListView#listView {
            background-color: rgb(211, 211, 211);
        }
        QPushButton#deleteFile {
            background-color: rgb(211, 211, 211);
        }
        QPushButton#replaceButton {
            background-color: rgb(211, 211, 211);
        }
        QLineEdit#searchEdit {
            background-color: rgb(211, 211, 211);
        }
        QPushButton#searchButton {
            background-color: rgb(211, 211, 211);
        }
        QListView#searchedTool {
            background-color: rgb(211, 211, 211);
        }
        QTabWidget#toolBox {
            background-color: rgb(211, 211, 211);
        }
        QScrollArea#scrollArea {
            background-color: rgb(211, 211, 211);
        }
        QTextEdit#log {
            background-color: rgb(211, 211, 211);
        }
)";

    qApp->setStyleSheet(""); // 清除当前窗口的样式
    qApp->setStyleSheet(darkModeStylesheet);
    logMessage("Switched to night mode.");
}


//----------编辑功能----------

//开始编辑
void MainWindow::startEditMode() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mUi.changeColorAct->setVisible(true);
    mUi.paintModeAct->setVisible(true);
    mUi.moveAct->setVisible(true);

    logMessage("Start editing.");
}

//结束编辑
void MainWindow::endEditMode() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }

    // 如果绘画模式是开启的，则将其关闭
    if (mbIsPaintModeActive) {
        paintMode(); // 调用 paintMode() 以关闭绘画模式
    }

    // 检查当前的绘制模式是否为 Move 模式
    if (mpCanvas->getDrawMode() == mpCanvas->Move) {
        // 如果当前是 Move 模式，则切换到 None 模式（退出 Move 模式）
        mpCanvas->setDrawMode(mpCanvas->None);
        logMessage("End moving.");
    }

    // 隐藏所有编辑相关的动作
    mUi.changeColorAct->setVisible(false);
    mUi.paintModeAct->setVisible(false);
    mUi.moveAct->setVisible(false);
    mUi.drawPolylineAct->setVisible(false);
    mUi.drawEllipseAct->setVisible(false);
    mUi.drawRectangleAct->setVisible(false);
    mUi.clearAct->setVisible(false);
    mUi.changePenColorAct->setVisible(false);
    mUi.changeThicknessAct->setVisible(false);
    mUi.clearLastGraphicAct->setVisible(false);

    // 设置绘制模式为 None，以确保不在编辑模式
    mpCanvas->setDrawMode(mpCanvas->None);
    logMessage("End editing.");
}

//修改矢量图层颜色功能
void MainWindow::changeColor() {
    // 获取当前 ListView 的 model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (!model)
        return;

    // 获取选中的项的索引
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    // 获取选中项的行号
    int row = selectedIndexes.first().row();

    // 获取对应的文件名
    QString mFileName = mMapFileIndex[row];

    if (QFileInfo(mFileName).suffix() != "shp") {
        logMessage("No valid vector layer!");
        return;
    }
    else {
        // 选择颜色
        QColor newColor = QColorDialog::getColor(Qt::black, this, tr("Select Pen Color"));
        if (!newColor.isValid()) // 用户取消了颜色选择
            return;

        // 创建新的渲染器并更新画笔颜色
        VectorLayerRenderer renderer = VectorLayerRenderer(mFileName);
        renderer.setPenColor(newColor); // 更新画笔颜色

        QImage updatedImage = renderer.renderToImage();

        // 从 ListView 中移除原有项
        removeSelectedItem();

        // 将新图像添加到 LayeredCanvas
        mpCanvas->addLayer(updatedImage);

        // 更新 ListView 中的项
        QStandardItem* item = new QStandardItem(QIcon(":/login/res/vector.png"), QFileInfo(mFileName).fileName());
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        model->appendRow(item);

        // 更新文件名映射
        int index = model->rowCount() - 1;
        mMapFileIndex[index] = mFileName;
        mMapImageIndex[index] = updatedImage;

        logMessage("Color changed successfully.");
    }
}

//进入/退出绘画模式
void MainWindow::paintMode() {
    mbIsPaintModeActive = !mbIsPaintModeActive; // 切换状态

    if (mbIsPaintModeActive) {
        logMessage("Start painting.");
    }
    else {
        logMessage("End painting.");
        // 退出绘画模式时将 drawMode 设置为 None
        mpCanvas->setDrawMode(mpCanvas->None);
    }

    // 根据状态设置可见性
    mUi.drawPolylineAct->setVisible(mbIsPaintModeActive);
    mUi.drawEllipseAct->setVisible(mbIsPaintModeActive);
    mUi.drawRectangleAct->setVisible(mbIsPaintModeActive);
    mUi.clearAct->setVisible(mbIsPaintModeActive);
    mUi.changePenColorAct->setVisible(mbIsPaintModeActive);
    mUi.changeThicknessAct->setVisible(mbIsPaintModeActive);
    mUi.clearLastGraphicAct->setVisible(mbIsPaintModeActive);
}

//更改画笔颜色
void MainWindow::changePenColor() {
    QColor selectedColor = QColorDialog::getColor(Qt::black, this, tr("Select Pen Color"));
    if (selectedColor.isValid()) {
        mpCanvas->setPenColor(selectedColor); // 设置 LayeredCanvas 的画笔颜色
        logMessage("Start painting with color: " + selectedColor.name());
    }
    else {
        logMessage("Pen color change cancelled.");
    }
}

//更改画笔粗细
void MainWindow::changePenWidth() {
    bool ok;
    int penWidth = QInputDialog::getInt(this, tr("Select Pen Width"),
        tr("Pen Width:"), 1, 1, 20, 1, &ok);
    if (ok) {
        mpCanvas->setPenWidth(penWidth); // 设置 LayeredCanvas 的画笔粗细
        logMessage("Set pen width to: " + QString::number(penWidth));
    }
    else {
        logMessage("Pen width change cancelled.");
    }
}

//清除绘制的图形
void MainWindow::clearLayer() {
    // 获取选中的项
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to clear graphics.");
        return; // 没有选中图层，返回
    }

    // 假设只清空第一个选中的项
    int index = selectedIndexes.first().row();

    // 调用 LayeredCanvas 的方法清空图层图形
    mpCanvas->clearLayerGraphics(index);

    logMessage("Cleared graphics from layer: " + QString::number(index + 1));
}

//清除最后一个绘制的图形
void MainWindow::clearLastGraphic() {
    // 获取选中的项
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to clear the last graphic.");
        return; // 没有选中图层，返回
    }

    // 假设只清空第一个选中的项
    int index = selectedIndexes.first().row();

    // 调用 LayeredCanvas 的方法清空图层最后一个图形
    mpCanvas->clearLastGraphic(index);

    logMessage("Cleared the last graphic from layer: " + QString::number(index + 1));
}

//移动图层
void MainWindow::on_moveAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }

    // 检查当前的绘制模式是否为 Move 模式
    if (mpCanvas->getDrawMode() == mpCanvas->Move) {
        // 如果当前是 Move 模式，则切换到 None 模式（退出 Move 模式）
        mpCanvas->setDrawMode(mpCanvas->None);
        logMessage("Exit Move mode");
    }
    else {
        // 如果当前不是 Move 模式，则进入 Move 模式
        mpCanvas->setDrawMode(mpCanvas->Move);
        logMessage("Enter Move mode");
    }
}

//绘制多线
void MainWindow::on_drawPolylineAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawPolyline);
}

//绘制椭圆
void MainWindow::on_drawEllipseAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawEllipse);
}

//绘制矩形
void MainWindow::on_drawRectangleAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawRectangle);
}


//----------栅格分析----------

//掩膜提取（完成）
void MainWindow::showMaskExtraction() {
    logMessage("Starting MaskExtraction.");

    MaskExtraction* maskExtraction = new MaskExtraction();
    maskExtraction->show();

    maskExtractionCommand* command = new maskExtractionCommand(maskExtraction);

    mUi.progressBar->setValue(0); // 初始化进度

    connect(maskExtraction, &MaskExtraction::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(maskExtraction, &MaskExtraction::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(maskExtraction, &MaskExtraction::beginClicked, this, [this, command, maskExtraction]() {
        command->execute();
        if (maskExtraction->mstrInputRasterPath.isEmpty() && maskExtraction->mstrMaskPath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 记录操作
        Action action;
        action.type = Action::Add;

        QFileInfo fileInfo(maskExtraction->mstrInputRasterPath);
        QString baseName = fileInfo.baseName() + "(Mask Extraction)";

        QIcon fileIcon(":/login/res/maskExtraction.png");
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);

        RasterViewer rasterV;

        if (!maskExtraction->mstrOutputRasterPath.isEmpty()) { // 检查路径是否为空
            action.fileNames << maskExtraction->mstrOutputRasterPath;
            QImage maskExtractionImage = rasterV.showRaster(maskExtraction->mstrOutputRasterPath);
            QStandardItemModel* mode = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (mode) {
                int index = mode->rowCount(); // 获取当前项的索引
                mMapFileIndex.insert(index, maskExtraction->mstrOutputRasterPath); // 存储文件名和索引
                mMapImageIndex.insert(index, maskExtractionImage);
                mode->appendRow(item);
                logMessage("Result of Mask Extraction added to list: " + baseName);
            }

            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(false);
            }
            mpCanvas->addLayer(maskExtractionImage); // 传入文件路径
            logMessage("Result of Mask Extraction added to canvas: " + baseName);
        }
        else {
            return;
        }

        mvActionHistory.push(action);

        logMessage("MaskExtraction runs successfully on file: " + maskExtraction->mstrInputRasterPath + "and" + maskExtraction->mstrMaskPath);

        delete maskExtraction;
        delete command;
        });
}

//邻域分析（完成）
void MainWindow::showRasterNeighborhoodStatistics() {
    logMessage("Starting Raster Neighborhood Statistics Analysis.");

    RasterNeighborhoodStatistics* rasterNeighborhoodStatistics = new RasterNeighborhoodStatistics();
    rasterNeighborhoodStatistics->show();
    logMessage("Show Raster Neighborhood Statistics Analysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    rasterNeighborhoodStatisticsCommand* command = new rasterNeighborhoodStatisticsCommand(rasterNeighborhoodStatistics);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::beginClicked, this, [this, command, rasterNeighborhoodStatistics]() {
        command->execute();
        // 检查文件名是否为空
        if (rasterNeighborhoodStatistics->mstrInputRasterPath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 记录操作
        Action action;
        action.type = Action::Add;

        if (!rasterNeighborhoodStatistics->mstrOutputMaxRasterPath.isEmpty()) {
            // 最大值
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMaxRasterPath);
            QString baseName = fileInfo.baseName() + "(Max)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMaxRasterPath.isEmpty()) { // 检查路径是否为空
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMaxRasterPath;
                QImage maxImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMaxRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // 获取当前项的索引
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMaxRasterPath); // 存储文件名和索引
                    mMapImageIndex.insert(index, maxImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Max) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(maxImage); // 传入文件路径
                logMessage("Result of Raster Neighborhood Statistics Analysis(Max) added to canvas: " + baseName);
            }
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputMinRasterPath.isEmpty()) {
            // 最小值
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMinRasterPath);
            QString baseName = fileInfo.baseName() + "(Min)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMinRasterPath.isEmpty()) { // 检查路径是否为空
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMinRasterPath;
                QImage minImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMinRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // 获取当前项的索引
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMinRasterPath); // 存储文件名和索引
                    mMapImageIndex.insert(index, minImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Min) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(minImage); // 传入文件路径
                logMessage("Result of Raster Neighborhood Statistics Analysis(Min) added to canvas: " + baseName);
            }
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputMeanRasterPath.isEmpty()) {
            // 均值
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMeanRasterPath);
            QString baseName = fileInfo.baseName() + "(Mean)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMeanRasterPath.isEmpty()) { // 检查路径是否为空
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMeanRasterPath;
                QImage meanImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMeanRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // 获取当前项的索引
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMeanRasterPath); // 存储文件名和索引
                    mMapImageIndex.insert(index, meanImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Mean) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(meanImage); // 传入文件路径
                logMessage("Result of Raster Neighborhood Statistics Analysis(Mean) added to canvas: " + baseName);
            }
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputWholeCSVPath.isEmpty()) {
            logMessage("Result CSV of Raster Neighborhood Statistics Analysis(WholeBand) saved successfully at " + rasterNeighborhoodStatistics->mstrOutputWholeCSVPath);
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputCSVPath.isEmpty()) {
            logMessage("Result CSV of Raster Neighborhood Statistics Analysis(SingleGrid) saved successfully at " + rasterNeighborhoodStatistics->mstrOutputCSVPath);
        }
        else {
            return;
        }

        mvActionHistory.push(action);

        logMessage("Raster Neighborhood Statistics Analysis runs successfully on file: " + rasterNeighborhoodStatistics->mstrInputRasterPath);

        delete rasterNeighborhoodStatistics;
        delete command;
        });
}

//栅格波段分析（完成）
void MainWindow::showRasterBandAnalysis()
{
    logMessage("Starting Raster Band Analysis.");

    RasterBandAnalysis* processRaster = new RasterBandAnalysis();
    processRaster->show();
    logMessage("Show Raster Band Analysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    // 创建命令对象
    rasterBandAnalysisCommand* command = new rasterBandAnalysisCommand(processRaster);

    connect(processRaster, &RasterBandAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(processRaster, &RasterBandAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(processRaster, &RasterBandAnalysis::beginClicked, this, [this, command, processRaster]() {
        command->execute();
        if (processRaster->mstrRasterFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 记录操作
        Action action;
        action.type = Action::Add;

        // 真彩色显示
        QFileInfo fileInfoTrue(processRaster->mstrRasterFilePath);
        QString baseName1 = fileInfoTrue.baseName() + "(True Color)";

        QIcon fileIconTrue(":/login/res/tureColor.png");
        QStandardItem* item1 = new QStandardItem(fileIconTrue, baseName1);
        item1->setCheckable(true);
        item1->setCheckState(Qt::Checked);

        RasterViewer rasterV;

        if (!processRaster->mstrTrueColorPath.isEmpty()) { // 检查路径是否为空
            action.fileNames << processRaster->mstrTrueColorPath;
            QImage trueColorImage = rasterV.showRaster(processRaster->mstrTrueColorPath);
            QStandardItemModel* model1 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model1) {
                int index = model1->rowCount(); // 获取当前项的索引
                mMapFileIndex.insert(index, processRaster->mstrTrueColorPath); // 存储文件名和索引
                mMapImageIndex.insert(index, trueColorImage);
                model1->appendRow(item1);
                logMessage("Result of True Color Display added to list: " + baseName1);
            }

            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(false);
            }
            mpCanvas->addLayer(trueColorImage); // 传入文件路径
            logMessage("Result of True Color Display added to canvas: " + baseName1);
        }
        else {
            return;
        }

        //假彩色显示
        QFileInfo fileInfoFalse(processRaster->mstrRasterFilePath);
        QString baseName2 = fileInfoFalse.baseName() + "(False Color)";

        QIcon fileIconFalse(":/login/res/tureColor.png");
        QStandardItem* item2 = new QStandardItem(fileIconFalse, baseName2);
        item2->setCheckable(true);
        item2->setCheckState(Qt::Checked);

        if (!processRaster->mstrFalseColorPath.isEmpty()) { // 检查路径是否为空
            action.fileNames << processRaster->mstrFalseColorPath;
            QImage falseColorImage = rasterV.showRaster(processRaster->mstrFalseColorPath);

            QStandardItemModel* model2 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model2) {
                int index = model2->rowCount(); // 获取当前项的索引
                mMapFileIndex.insert(index, processRaster->mstrFalseColorPath);
                mMapImageIndex.insert(index, falseColorImage);
                model2->appendRow(item2);
                logMessage("Result of False Color Display added to list: " + baseName2);
            }

            mpCanvas->addLayer(falseColorImage); // 传入文件路径
            logMessage("Result of False Color Display added to canvas: " + baseName2);
        }
        else {
            logMessage("False Color path is empty. Skipping False Color display.");
        }

        // 灰度直方图
        QFileInfo fileInfoGray(processRaster->mstrRasterFilePath);
        QString baseName3 = fileInfoGray.baseName() + "(Gray Histogram)";

        QIcon fileIconGray(":/login/res/gray.png");
        QStandardItem* item3 = new QStandardItem(fileIconGray, baseName3);
        item3->setCheckable(true);
        item3->setCheckState(Qt::Checked);

        if (!processRaster->mstrGrayHistogramPath.isEmpty()) { // 检查路径是否为空
            action.fileNames << processRaster->mstrGrayHistogramPath;
            QImage grayImage = rasterV.showRaster(processRaster->mstrGrayHistogramPath);

            QStandardItemModel* model3 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model3) {
                int index = model3->rowCount(); // 获取当前项的索引
                mMapFileIndex.insert(index, processRaster->mstrGrayHistogramPath);
                mMapImageIndex.insert(index, grayImage);
                model3->appendRow(item3);
                logMessage("Result of Gray Histogram added to list: " + baseName3);
            }

            mpCanvas->addLayer(grayImage); // 传入文件路径
            logMessage("Result of Gray Histogram added to canvas: " + baseName3);
        }
        else {
            logMessage("Gray Histogram path is empty. Skipping Gray Histogram display.");
        }

        // 直方图均衡化
        QFileInfo fileInfoEnhanced(processRaster->mstrRasterFilePath);
        QString baseName4 = fileInfoEnhanced.baseName() + "(Histogram Equalization)";

        QIcon fileIconEnhanced(":/login/res/enhanced.png");
        QStandardItem* item4 = new QStandardItem(fileIconEnhanced, baseName4);
        item4->setCheckable(true);
        item4->setCheckState(Qt::Checked);

        if (!processRaster->mstrEnhancedHistogramPath.isEmpty()) { // 检查路径是否为空
            action.fileNames << processRaster->mstrEnhancedHistogramPath;
            QImage enhancedImage = rasterV.showRaster(processRaster->mstrEnhancedHistogramPath);

            QStandardItemModel* model4 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model4) {
                int index = model4->rowCount(); // 获取当前项的索引
                mMapFileIndex.insert(index, processRaster->mstrEnhancedHistogramPath);
                mMapImageIndex.insert(index, enhancedImage);
                model4->appendRow(item4);
                logMessage("Result of Histogram Equalization added to list: " + baseName4);
            }

            mpCanvas->addLayer(enhancedImage); // 传入文件路径
            logMessage("Result of Histogram Equalization added to canvas: " + baseName4);
        }
        else {
            logMessage("Enhanced Histogram path is empty. Skipping Histogram Equalization display.");
        }

        logMessage("RasterBandAnalysis runs successfully on file: " + processRaster->mstrRasterFilePath);

        mvActionHistory.push(action);

        // 删除对象
        processRaster->deleteLater();
        delete  command;
        });
}


//----------矢量分析----------

//缓冲区分析（完成）
void MainWindow::showVectorLayerBufferAnalysis() {
    logMessage("Starting Vector Layer Buffer Analysis.");

    VectorLayerBufferAnalysis* vectorLayerBufferAnalysis = new VectorLayerBufferAnalysis();
    vectorLayerBufferAnalysis->show();
    logMessage("Show Vector Layer Buffer Analysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    VectorLayerBufferAnalysisCommand* command = new VectorLayerBufferAnalysisCommand(vectorLayerBufferAnalysis);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::beginClicked, this, [this, command, vectorLayerBufferAnalysis]() {
        command->execute();
        if (vectorLayerBufferAnalysis->mstrFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 从文件路径中提取文件名
        QFileInfo fileInfo(vectorLayerBufferAnalysis->mstrFilePath);
        QString baseName = fileInfo.baseName() + "(Buffer)"; // 文件名（不包含扩展名）

        // 创建图标项并设置图标
        QIcon fileIcon(":/login/res/bufferAnalysis.png");
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);

        VectorLayerRenderer vectorLayerRenderer(vectorLayerBufferAnalysis->mstrResultPath);
        QImage resultImage = vectorLayerRenderer.renderToImage();

        // 将图标项添加到 listView 的 model 中
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // 获取当前项的索引
            mMapFileIndex.insert(index, vectorLayerBufferAnalysis->mstrResultPath);
            mMapImageIndex.insert(index, resultImage);
            model->appendRow(item);

            logMessage("Result of Vector Layer Buffer Analysis added to list: " + baseName);
        }

        // 将渲染的图像添加到 scrollArea 中的 LayeredCanvas
        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }
        mpCanvas->addLayer(resultImage); // 传入文件路径
        logMessage("Result of Vector Layer Buffer Analysis added to canvas:" + baseName);

        logMessage("Vector Layer Buffer Analysis runs successfully on file: " + vectorLayerBufferAnalysis->mstrFilePath);

        // 记录操作
        Action action;
        action.type = Action::Add;
        action.fileNames << vectorLayerBufferAnalysis->mstrResultPath;
        mvActionHistory.push(action);

        // 清理资源
        delete vectorLayerBufferAnalysis;
        delete command;
        });
}

//统计分析（完成）
void MainWindow::showStatisticAnalysis() {
    logMessage("Starting Vector Statistic Analysis.");

    StatisticAnalysis* statisticAnalysis = new StatisticAnalysis();
    statisticAnalysis->show();
    logMessage("Show Vector Statistic Analysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    StatisticAnalysisCommand* command = new StatisticAnalysisCommand(statisticAnalysis);

    connect(statisticAnalysis, &StatisticAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(statisticAnalysis, &StatisticAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(statisticAnalysis, &StatisticAnalysis::beginClicked, this, [this, command, statisticAnalysis]() {
        command->execute();
        if (statisticAnalysis->mstrFileName.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        logMessage("Result of Vector Statistic Analysis saved successfully: " + statisticAnalysis->mstrSaveFileName);

        logMessage("Vector Statistic Analysis runs successfully on file: " + statisticAnalysis->mstrFileName);

        delete command;
        delete statisticAnalysis;
        });
}

//三角网分析（完成）
void MainWindow::showTriangulation() {
    logMessage("Starting TriangularNetworkAnalysis.");

    Triangulation* triangulation = new Triangulation();
    triangulation->show();
    logMessage("Show TriangularNetworkAnalysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    TriangulationCommand* command = new TriangulationCommand(triangulation);

    connect(triangulation, &Triangulation::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(triangulation, &Triangulation::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(triangulation, &Triangulation::beginClicked, this, [this, command, triangulation]() {
        command->execute();
        // 检查文件名是否为空
        if (triangulation->mstrInputFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 从文件路径中提取文件名
        QFileInfo fileInfo(triangulation->mstrInputFilePath);
        QString baseName = fileInfo.baseName() + "(TriangularNetwork)"; // 文件名（不包含扩展名）

        // 创建图标项并设置图标
        QIcon fileIcon(":/login/res/Triangulation.png"); // 根据资源文件路径修改
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // 设置复选框
        item->setCheckState(Qt::Checked); // 默认选中

        VectorLayerRenderer vectorLayerRenderer(triangulation->mstrResultPath);
        QImage resultImage = vectorLayerRenderer.renderToImage();

        // 将图标项添加到 listView 的 model 中
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // 获取当前项的索引
            mMapFileIndex.insert(index, triangulation->mstrResultPath);
            mMapImageIndex.insert(index, resultImage); // 存储索引与 QImage 的映射
            model->appendRow(item);
            logMessage("Result of TriangularNetworkAnalysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // 将结果加入到视图
        mpCanvas->addLayer(resultImage); // 传入文件路径
        logMessage("Result of TriangularNetworkAnalysis added to canvas : " + baseName);

        logMessage("TriangularNetworkAnalysis runs successfully on file: " + triangulation->mstrInputFilePath);

        // 记录操作
        Action action;
        action.type = Action::Add;
        action.fileNames << triangulation->mstrResultPath;
        mvActionHistory.push(action);

        // 删除 triangulation 对象
        delete command;
        triangulation->deleteLater();
        });
}

//Voronoi图（完成）
void MainWindow::showVoronoiAnalysis() {
    logMessage("Starting VoronoiAnalysis.");

    VoronoiAnalysis* voronoiAnalysis = new VoronoiAnalysis();
    voronoiAnalysis->show();
    logMessage("Show VoronoiAnalysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    VoronoiAnalysisCommand* command = new VoronoiAnalysisCommand(voronoiAnalysis);

    connect(voronoiAnalysis, &VoronoiAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(voronoiAnalysis, &VoronoiAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(voronoiAnalysis, &VoronoiAnalysis::beginClicked, this, [this, command, voronoiAnalysis]() {
        command->execute();
        // 检查文件名是否为空
        if (voronoiAnalysis->mstrInputFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 从文件路径中提取文件名
        QFileInfo fileInfo(voronoiAnalysis->mstrInputFilePath);
        QString baseName = fileInfo.baseName() + "(VoronoiAnalysis)"; // 文件名（不包含扩展名）

        // 创建图标项并设置图标
        QIcon fileIcon(":/login/res/VoronoiAnalysis.png"); // 根据资源文件路径修改
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // 设置复选框
        item->setCheckState(Qt::Checked); // 默认选中

        VectorLayerRenderer vectorLayerRenderer(voronoiAnalysis->mstrResultPath);
        QImage voronoiImage = vectorLayerRenderer.renderToImage();

        // 将图标项添加到 listView 的 model 中
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // 获取当前项的索引
            mMapFileIndex.insert(index, voronoiAnalysis->mstrResultPath);
            mMapImageIndex.insert(index, voronoiImage); // 存储索引与 QImage 的映射
            model->appendRow(item);
            logMessage("Result of VoronoiAnalysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // 将结果加入到视图
        mpCanvas->addLayer(voronoiImage); // 传入文件路径

        logMessage("Result of VoronoiAnalysis added to canvas : " + baseName);

        logMessage("VoronoiAnalysis runs successfully on file: " + voronoiAnalysis->mstrInputFilePath);

        // 记录操作
        Action action;
        action.type = Action::Add;
        action.fileNames << voronoiAnalysis->mstrResultPath;
        mvActionHistory.push(action);

        // 删除对象
        delete command;
        voronoiAnalysis->deleteLater();
        });
}

//凸包分析（完成）
void MainWindow::showConvexHullAnalysis() {
    logMessage("Starting Convex Hull Analysis.");

    ConvexAnalysis* convexAnalysis = new ConvexAnalysis();
    convexAnalysis->show();
    logMessage("Show Convex Hull Analysis successfully.");

    mUi.progressBar->setValue(0); // 初始化进度

    ConvexAnalysisCommand* command = new ConvexAnalysisCommand(convexAnalysis);

    connect(convexAnalysis, &ConvexAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(convexAnalysis, &ConvexAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(convexAnalysis, &ConvexAnalysis::beginClicked, this, [this, command, convexAnalysis]() {
        command->execute();
        // 检查文件名是否为空
        if (convexAnalysis->mstrFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // 文件名为空，跳过后续操作
        }

        // 从文件路径中提取文件名
        QFileInfo fileInfo(convexAnalysis->mstrFilePath);
        QString baseName = fileInfo.baseName() + "(ConvexHullAnalysis)"; // 文件名（不包含扩展名）

        // 创建图标项并设置图标
        QIcon fileIcon(":/login/res/convexHullAnalysis.png"); // 根据资源文件路径修改
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // 设置复选框
        item->setCheckState(Qt::Checked); // 默认选中

        VectorLayerRenderer vectorLayerRenderer(convexAnalysis->mstrSavePath);
        QImage convexImage = vectorLayerRenderer.renderToImage();

        // 将图标项添加到 listView 的 model 中
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // 获取当前项的索引
            mMapFileIndex.insert(index, convexAnalysis->mstrSavePath);
            mMapImageIndex.insert(index, convexImage); // 存储索引与 QImage 的映射
            model->appendRow(item);
            logMessage("Result of Convex Hull Analysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // 将结果加入到视图
        mpCanvas->addLayer(convexImage); // 传入文件路径

        logMessage("Result of Convex Hull Analysis added to canvas : " + baseName);

        logMessage("Convex Hull Analysis runs successfully on file: " + convexAnalysis->mstrFilePath);

        // 记录操作
        Action action;
        action.type = Action::Add;
        action.fileNames << convexAnalysis->mstrSavePath;
        mvActionHistory.push(action);

        // 删除对象
        delete command;
        convexAnalysis->deleteLater();
        });


}


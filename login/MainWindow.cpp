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
    mpCanvas(nullptr) // ��ʼ�� canvas Ϊ nullptr
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

    // ����һ�� QStandardItemModel �����ø� listView
    QStandardItemModel* model = new QStandardItemModel(this);
    mUi.listView->setModel(model);
    connect(model, &QStandardItemModel::itemChanged, this, &MainWindow::toggleLayerVisibility);

    // ��������
    mpSearchedToolModel = new QStandardItemModel(this);
    mUi.searchedTool->setModel(mpSearchedToolModel);

    connect(mUi.searchButton, &QPushButton::clicked, this, &MainWindow::searchButtons);
    connect(mUi.searchedTool, &QListView::clicked, this, &MainWindow::handleSearchResultClick);

    // �����л�����
    connect(mUi.lightModeAct, &QAction::triggered, this, &MainWindow::switchToLightMode);
    connect(mUi.darkModeAct, &QAction::triggered, this, &MainWindow::switchToDarkMode);
    connect(mUi.blueModeAct, &QAction::triggered, this, &MainWindow::switchToBlueMode);

    //�༭ģʽ����
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

    // ������������ QAction
    mUi.undoAct->setVisible(true);
    mUi.redoAct->setVisible(true);
    connect(mUi.undoAct, &QAction::triggered, this, &MainWindow::undo);
    connect(mUi.redoAct, &QAction::triggered, this, &MainWindow::redo);

    // ��ʾ������
    mUi.progressBar->setVisible(true);
    // ���ý�������Χ
    mUi.progressBar->setRange(0, 100);
    mUi.progressBar->setValue(0); // ��ʼ������
}

MainWindow::~MainWindow()
{
}

//----------��������----------

//���ļ�
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

    // ȷ�� scrollArea �ڲ��� LayeredCanvas
    if (!mpCanvas) {
        mpCanvas = new LayeredCanvas(this);
        mUi.scrollArea->setWidget(mpCanvas);
        mUi.scrollArea->setWidgetResizable(true);
    }

    QImage loadedImage; // �����������ڴ洢���ص�ͼ��

    if (suffix == "shp") {
        VectorLayerRenderer renderer(fileName);
        loadedImage = renderer.renderToImage(); // ��Ⱦ SHP �ļ�Ϊ QImage
        mpCanvas->addLayer(loadedImage); // ���ͼ�㣬�������ļ����Ա��ȡ���귶Χ
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

    // ���ݺ�׺��ѡ��ͼ��
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

    // ���ļ�����ӵ� listView �� model ��
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());

    QString baseName = fileInfo.fileName();
    QStandardItem* item = new QStandardItem(fileIcon, baseName);
    item->setCheckable(true); // ���ø�ѡ��
    item->setCheckState(Qt::Checked); // Ĭ��ѡ��
    model->appendRow(item);

    // ��ȡ����ӵ��������
    int index = model->rowCount() - 1;
    mMapFileIndex[index] = fileName; // �洢�ļ���������
    mMapImageIndex[index] = loadedImage; // �洢 QImage �������Ķ�Ӧ��ϵ

    // ��¼����
    Action action;
    action.type = Action::Add;
    action.fileNames << fileName;
    mvActionHistory.push(action);

    logMessage("File added to list: " + baseName);
    logMessage("File opened successfully: " + fileName);
}

//�����ļ�
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

    // ����ͼ�񱣴��߼�
    QString imageSuffix = QFileInfo(newFileName).suffix().toLower();

    if (imageSuffix == "png" || imageSuffix == "jpg" || imageSuffix == "jpeg") {
        // ����ͼ�񱣴��߼�
        foreach(const QModelIndex & index, selectedIndexes) {
            QString originalFileName = mMapFileIndex[index.row()];
            QString imageSuffix = QFileInfo(newFileName).suffix().toLower();

            // ���� QImage Ϊ PNG �� JPG
            if (imageSuffix == "png" || imageSuffix == "jpg" || imageSuffix == "jpeg") {
                QImage imageToSave = mMapImageIndex[index.row()]; // ��ȡ��Ӧ�� QImage
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

                // ���� SHP �ļ���������ļ�
                if (suffix == "shp") {
                    QString baseName = QFileInfo(newFileName).baseName();

                    // �������ļ�
                    QString newShpName = newFileName;
                    if (!newShpName.endsWith(".shp", Qt::CaseInsensitive)) {
                        newShpName += ".shp";
                    }
                    QFile::copy(originalFileName, newShpName);

                    // ��������ļ�
                    QStringList extensions = { ".cpg", ".dbf", ".prj", ".sbn", ".sbx", ".shx", ".xml" };
                    foreach(const QString & ext, extensions) {
                        QString originalRelatedFileName = QFileInfo(originalFileName).absolutePath() + "/" + QFileInfo(originalFileName).baseName() + ext;
                        QString newRelatedFileName = QFileInfo(newFileName).absolutePath() + "/" + baseName + ext;
                        QFile::copy(originalRelatedFileName, newRelatedFileName);
                    }

                    logMessage("SHP file and its related files saved successfully: " + newFileName);
                }
                // ���������ļ�����
                else if (suffix == "tif") {
                    if (!newFileName.endsWith(".tif", Qt::CaseInsensitive)) {
                        newFileName += ".tif";
                    }
                    QFile::copy(originalFileName, newFileName);
                    logMessage("TIFF file saved successfully: " + newFileName);
                }
                else if (suffix == "geojson" || suffix == "json") {
                    if (!newFileName.endsWith(".geojson", Qt::CaseInsensitive) && !newFileName.endsWith(".json", Qt::CaseInsensitive)) {
                        newFileName += ".geojson"; // Ĭ����� .geojson ��׺
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

//���湤���ļ�
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

    // ���������ִ��ͼ��
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (model) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QStandardItem* item = model->item(row);
            if (item) {
                LayerInfo layer;
                layer.filePath = mMapFileIndex[row]; // ����������ȡ�ļ�·��
                layer.type = QFileInfo(layer.filePath).suffix(); // ʹ�ú�׺��Ϊ����
                xmlWriter.writeStartElement("Layer");
                xmlWriter.writeTextElement("FilePath", layer.filePath);
                xmlWriter.writeTextElement("Type", layer.type);
                xmlWriter.writeEndElement(); // ���� Layer
            }
        }
    }

    xmlWriter.writeEndElement(); // ���� Project
    xmlWriter.writeEndDocument();

    file.close();
    logMessage("Project saved successfully: " + filePath);
}

//�򿪹����ļ�
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
    mvLayers.clear(); // ���֮ǰ��ͼ����Ϣ

    //��¼����
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
                mvLayers.append(layer); // ��ͼ����Ϣ��ӵ��б���
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

    // ���ݶ�ȡ��ͼ����Ϣ���� UI ��ͼ��
    for (const LayerInfo& layer : mvLayers) {
        // �����ļ����͵��ò�ͬ�ļ����߼�
        QString suffix = QFileInfo(layer.filePath).suffix().toLower();

        // ȷ�� scrollArea �ڲ��� LayeredCanvas
        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(true);
        }

        QImage loadedImage; // �����������ڴ洢���ص�ͼ��

        if (suffix == "shp") {
            VectorLayerRenderer renderer(layer.filePath);
            loadedImage = renderer.renderToImage(); // ��Ⱦ SHP �ļ�Ϊ QImage
            mpCanvas->addLayer(loadedImage); // ���ͼ��
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

        // ���ݺ�׺��ѡ��ͼ��
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

        // ���ļ�����ӵ� listView �� model ��
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        QString baseName = QFileInfo(layer.filePath).fileName();
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // ���ø�ѡ��
        item->setCheckState(Qt::Checked); // Ĭ��ѡ��
        model->appendRow(item);

        // ��ȡ����ӵ��������
        int index = model->rowCount() - 1;
        mMapFileIndex[index] = layer.filePath; // �洢�ļ���������
        mMapImageIndex[index] = loadedImage; // �洢 QImage �������Ķ�Ӧ��ϵ

        logMessage("File added to list: " + baseName);
        logMessage("File opened successfully: " + layer.filePath);
    }
}

//��ͼ�������
void MainWindow::openLayersManage() {
    logMessage("Opening layer management dialog.");

    if (mUi.leftDock1 && !mUi.leftDock1->isVisible()) {
        mUi.leftDock1->setVisible(true);
    }

    logMessage("Layer management dialog opened successfully.");
}

//�򿪹�����
void MainWindow::openToolBox() {
    logMessage("Opening ToolBox.");

    if (mUi.leftDock2 && !mUi.leftDock2->isVisible()) {
        mUi.leftDock2->setVisible(true);
    }

    logMessage("ToolBox opened successfully");
}

//ɾ������
void MainWindow::removeSelectedItem() {
    // ��ȡ��ǰ ListView �� model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (!model)
        return;

    // ��ȡѡ�е��������
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    // ��ȡѡ������к�
    int row = selectedIndexes.first().row();

    QString removedFileName = mMapFileIndex.value(row);

    // �� listView �� model ���Ƴ�ѡ�е���
    model->removeRow(row);

    // ��鲢�� QMap ���Ƴ���Ӧ����Ŀ
    if (mMapFileIndex.contains(row)) {
        mMapFileIndex.remove(row);
        logMessage("Removed file index mapping for row: " + QString::number(row));
    }
    if (mMapImageIndex.contains(row)) {
        mMapImageIndex.remove(row);
        logMessage("Removed image index mapping for row: " + QString::number(row));
    }

    // �� LayeredCanvas ���Ƴ���Ӧ��ͼ��
    if (mpCanvas) {
        mpCanvas->removeLayer(row);
    }

    logMessage("Selected item " + QString::number(row + 1) + " removed.");

    // ���� mMapFileIndex �� mMapImageIndex �е�����
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

    // ��¼����
    Action action;
    action.type = Action::Remove;
    action.fileNames << removedFileName;
    mvActionHistory.push(action);

    logMessage("Index maps updated after removal.");
}

// ʵ�ֻ�ȡ�����ĺ���
int MainWindow::getIndexByFileName(const QString& fileName) const
{
    // ���� mMapFileIndex ���Ƿ���ڶ�Ӧ���ļ���
    if (mMapFileIndex.values().contains(fileName)) {
        // ��ȡ��һ��ƥ����ļ����ļ�ֵ����������
        return mMapFileIndex.key(fileName);
    }
    else {
        // ���û���ҵ������� -1 ��ʾ�ļ���������
        return -1;
    }
}

//��������
void MainWindow::undo() {
    if (mvActionHistory.isEmpty()) {
        logMessage("No actions to undo.");
        return;
    }

    Action lastAction = mvActionHistory.pop(); // ��ȡ���һ�����
    mvRedoHistory.push(lastAction); // �����һ�����ѹ��������ʷջ

    // ���� mMapFileIndex �� mMapImageIndex �е�����
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
        // �������Ӳ������Ƴ������ӵ�ͼ��
        for (const QString& fileName : lastAction.fileNames) {
            int row = getIndexByFileName(fileName);

            // ��ȡ��ǰ ListView �� model
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (!model)
                return;

            // �� listView �� model ���Ƴ�ѡ�е���
            model->removeRow(row);

            // ��鲢�� QMap ���Ƴ���Ӧ����Ŀ
            if (mMapFileIndex.contains(row)) {
                mMapFileIndex.remove(row);
            }
            if (mMapImageIndex.contains(row)) {
                mMapImageIndex.remove(row);
            }

            // �� LayeredCanvas ���Ƴ���Ӧ��ͼ��
            if (mpCanvas) {
                mpCanvas->removeLayer(row);
            }


            updateIndexMap(mMapFileIndex, row);
            updateImageIndexMap(mMapImageIndex, row);
        }
        break;
    }
    case Action::Remove: {
        // ������Ƴ����������������Щͼ��
        for (const QString& fileName : lastAction.fileNames) {
            QFileInfo fileInfo(fileName);
            QString suffix = fileInfo.suffix().toLower();

            // ȷ�� scrollArea �ڲ��� LayeredCanvas
            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(true);
            }

            QImage loadedImage; // �����������ڴ洢���ص�ͼ��

            if (suffix == "shp") {
                VectorLayerRenderer renderer(fileName);
                loadedImage = renderer.renderToImage(); // ��Ⱦ SHP �ļ�Ϊ QImage
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

            // ���ݺ�׺��ѡ��ͼ��
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

            // ���ļ�����ӵ� listView �� model ��
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            QString baseName = fileInfo.fileName();
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            model->appendRow(item);

            // ��ȡ����ӵ��������
            int index = model->rowCount() - 1;
            mMapFileIndex[index] = fileName;
            mMapImageIndex[index] = loadedImage;
        }
        break;
    }
    }
    logMessage("Undo successfully.");
}

//��������
void MainWindow::redo() {
    if (mvRedoHistory.isEmpty()) {
        logMessage("No actions to redo.");
        return;
    }

    Action lastAction = mvRedoHistory.pop(); // ��ȡ���һ����������

    // ���� mMapFileIndex �� mMapImageIndex �е�����
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

            // ȷ�� scrollArea �ڲ��� LayeredCanvas
            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(true);
            }

            QImage loadedImage; // �����������ڴ洢���ص�ͼ��

            if (suffix == "shp") {
                VectorLayerRenderer renderer(fileName);
                loadedImage = renderer.renderToImage(); // ��Ⱦ SHP �ļ�Ϊ QImage
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

            // ���ݺ�׺��ѡ��ͼ��
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

            // ���ļ�����ӵ� listView �� model ��
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            QString baseName = fileInfo.fileName();
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);
            model->appendRow(item);

            // ��ȡ����ӵ��������
            int index = model->rowCount() - 1;
            mMapFileIndex[index] = fileName;
            mMapImageIndex[index] = loadedImage;
        }
        break;
    }
    case Action::Remove: {
        for (const QString& fileName : lastAction.fileNames) {
            int row = getIndexByFileName(fileName);

            // ��ȡ��ǰ ListView �� model
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (!model)
                return;

            // �� listView �� model ���Ƴ�ѡ�е���
            model->removeRow(row);

            // ��鲢�� QMap ���Ƴ���Ӧ����Ŀ
            if (mMapFileIndex.contains(row)) {
                mMapFileIndex.remove(row);
            }
            if (mMapImageIndex.contains(row)) {
                mMapImageIndex.remove(row);
            }

            // �� LayeredCanvas ���Ƴ���Ӧ��ͼ��
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

//��λ����
void MainWindow::restoreSelectedLayerPosition() {
    // ��ȡѡ�е���
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to restore position.");
        return; // û��ѡ��ͼ�㣬����
    }

    // ����ֻ�ָ���һ��ѡ�е���
    int index = selectedIndexes.first().row();

    // ���� LayeredCanvas �ķ����ָ�ͼ��λ��
    mpCanvas->restoreAllPositions(index);

    logMessage("Restored position of layer: " + QString::number(index + 1));
}

// ���ع��ܲۺ���
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

//��������
void MainWindow::searchButtons() {
    QString searchText = mUi.searchEdit->text().trimmed();

    // �����������Ϊ�գ���ִ������
    if (searchText.isEmpty()) {
        logMessage("Search text is empty. Please enter a valid text.");
        // ���֮ǰ���������
        mpSearchedToolModel->clear();
        return;
    }

    logMessage("Searching for buttons with text: " + searchText);

    // ���֮ǰ���������
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

//ģ�ⰴť�����
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

//��־���
void MainWindow::logMessage(const QString& message) {
    if (mUi.log) {
        // ��ȡ��ǰʱ��
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        // ����ǰʱ������Ϣ����
        QString formattedMessage = QString("[%1] %2").arg(currentTime, message);
        // �����־
        mUi.log->append(formattedMessage);
    }
}

//�û���ʾ
void MainWindow::showUserTips() {
    UserTips* userTips = new UserTips();
    userTips->show();

    logMessage("Show User Tips successfully.");
}

//��ԭ�������ε�ͼ
void MainWindow::showTravelQuYuan() {
    TravelQuYuan* travelQuYuan = new TravelQuYuan();
    travelQuYuan->show();

    logMessage("Show tourist map of QuYuan's hometown successfully.");
}

//�������н���
void MainWindow::onAnalysisProgressUpdated(int progress) {
    // ��������һ�������������� UI Ԫ������ʾ����
    mUi.progressBar->setValue(progress); // ���½�����
}

//��������ʱ��
void MainWindow::onAnalysisProgressGoing(double elapsedTime) {
    mUi.time->setText(QString("%1 s").arg(elapsedTime));
}


//----------�л���ʽ----------

//ԭ����ɫģʽ
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

    qApp->setStyleSheet(""); // �����ǰ���ڵ���ʽ
    qApp->setStyleSheet(blueModeStylesheet); // ��������ʽ

    logMessage("Switched to original blue mode.");
}

//����ģʽ
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


    // ���֮ǰ����ʽ��
    qApp->setStyleSheet(""); // �����ǰ���ڵ���ʽ
    qApp->setStyleSheet(lightModeStylesheet); // ��������ʽ

    logMessage("Switched to daytime mode.");
}

//��ҹģʽ
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

    qApp->setStyleSheet(""); // �����ǰ���ڵ���ʽ
    qApp->setStyleSheet(darkModeStylesheet);
    logMessage("Switched to night mode.");
}


//----------�༭����----------

//��ʼ�༭
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

//�����༭
void MainWindow::endEditMode() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }

    // ����滭ģʽ�ǿ����ģ�����ر�
    if (mbIsPaintModeActive) {
        paintMode(); // ���� paintMode() �Թرջ滭ģʽ
    }

    // ��鵱ǰ�Ļ���ģʽ�Ƿ�Ϊ Move ģʽ
    if (mpCanvas->getDrawMode() == mpCanvas->Move) {
        // �����ǰ�� Move ģʽ�����л��� None ģʽ���˳� Move ģʽ��
        mpCanvas->setDrawMode(mpCanvas->None);
        logMessage("End moving.");
    }

    // �������б༭��صĶ���
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

    // ���û���ģʽΪ None����ȷ�����ڱ༭ģʽ
    mpCanvas->setDrawMode(mpCanvas->None);
    logMessage("End editing.");
}

//�޸�ʸ��ͼ����ɫ����
void MainWindow::changeColor() {
    // ��ȡ��ǰ ListView �� model
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
    if (!model)
        return;

    // ��ȡѡ�е��������
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty())
        return;

    // ��ȡѡ������к�
    int row = selectedIndexes.first().row();

    // ��ȡ��Ӧ���ļ���
    QString mFileName = mMapFileIndex[row];

    if (QFileInfo(mFileName).suffix() != "shp") {
        logMessage("No valid vector layer!");
        return;
    }
    else {
        // ѡ����ɫ
        QColor newColor = QColorDialog::getColor(Qt::black, this, tr("Select Pen Color"));
        if (!newColor.isValid()) // �û�ȡ������ɫѡ��
            return;

        // �����µ���Ⱦ�������»�����ɫ
        VectorLayerRenderer renderer = VectorLayerRenderer(mFileName);
        renderer.setPenColor(newColor); // ���»�����ɫ

        QImage updatedImage = renderer.renderToImage();

        // �� ListView ���Ƴ�ԭ����
        removeSelectedItem();

        // ����ͼ����ӵ� LayeredCanvas
        mpCanvas->addLayer(updatedImage);

        // ���� ListView �е���
        QStandardItem* item = new QStandardItem(QIcon(":/login/res/vector.png"), QFileInfo(mFileName).fileName());
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);
        model->appendRow(item);

        // �����ļ���ӳ��
        int index = model->rowCount() - 1;
        mMapFileIndex[index] = mFileName;
        mMapImageIndex[index] = updatedImage;

        logMessage("Color changed successfully.");
    }
}

//����/�˳��滭ģʽ
void MainWindow::paintMode() {
    mbIsPaintModeActive = !mbIsPaintModeActive; // �л�״̬

    if (mbIsPaintModeActive) {
        logMessage("Start painting.");
    }
    else {
        logMessage("End painting.");
        // �˳��滭ģʽʱ�� drawMode ����Ϊ None
        mpCanvas->setDrawMode(mpCanvas->None);
    }

    // ����״̬���ÿɼ���
    mUi.drawPolylineAct->setVisible(mbIsPaintModeActive);
    mUi.drawEllipseAct->setVisible(mbIsPaintModeActive);
    mUi.drawRectangleAct->setVisible(mbIsPaintModeActive);
    mUi.clearAct->setVisible(mbIsPaintModeActive);
    mUi.changePenColorAct->setVisible(mbIsPaintModeActive);
    mUi.changeThicknessAct->setVisible(mbIsPaintModeActive);
    mUi.clearLastGraphicAct->setVisible(mbIsPaintModeActive);
}

//���Ļ�����ɫ
void MainWindow::changePenColor() {
    QColor selectedColor = QColorDialog::getColor(Qt::black, this, tr("Select Pen Color"));
    if (selectedColor.isValid()) {
        mpCanvas->setPenColor(selectedColor); // ���� LayeredCanvas �Ļ�����ɫ
        logMessage("Start painting with color: " + selectedColor.name());
    }
    else {
        logMessage("Pen color change cancelled.");
    }
}

//���Ļ��ʴ�ϸ
void MainWindow::changePenWidth() {
    bool ok;
    int penWidth = QInputDialog::getInt(this, tr("Select Pen Width"),
        tr("Pen Width:"), 1, 1, 20, 1, &ok);
    if (ok) {
        mpCanvas->setPenWidth(penWidth); // ���� LayeredCanvas �Ļ��ʴ�ϸ
        logMessage("Set pen width to: " + QString::number(penWidth));
    }
    else {
        logMessage("Pen width change cancelled.");
    }
}

//������Ƶ�ͼ��
void MainWindow::clearLayer() {
    // ��ȡѡ�е���
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to clear graphics.");
        return; // û��ѡ��ͼ�㣬����
    }

    // ����ֻ��յ�һ��ѡ�е���
    int index = selectedIndexes.first().row();

    // ���� LayeredCanvas �ķ������ͼ��ͼ��
    mpCanvas->clearLayerGraphics(index);

    logMessage("Cleared graphics from layer: " + QString::number(index + 1));
}

//������һ�����Ƶ�ͼ��
void MainWindow::clearLastGraphic() {
    // ��ȡѡ�е���
    QModelIndexList selectedIndexes = mUi.listView->selectionModel()->selectedIndexes();
    if (selectedIndexes.isEmpty()) {
        logMessage("No layer selected to clear the last graphic.");
        return; // û��ѡ��ͼ�㣬����
    }

    // ����ֻ��յ�һ��ѡ�е���
    int index = selectedIndexes.first().row();

    // ���� LayeredCanvas �ķ������ͼ�����һ��ͼ��
    mpCanvas->clearLastGraphic(index);

    logMessage("Cleared the last graphic from layer: " + QString::number(index + 1));
}

//�ƶ�ͼ��
void MainWindow::on_moveAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }

    // ��鵱ǰ�Ļ���ģʽ�Ƿ�Ϊ Move ģʽ
    if (mpCanvas->getDrawMode() == mpCanvas->Move) {
        // �����ǰ�� Move ģʽ�����л��� None ģʽ���˳� Move ģʽ��
        mpCanvas->setDrawMode(mpCanvas->None);
        logMessage("Exit Move mode");
    }
    else {
        // �����ǰ���� Move ģʽ������� Move ģʽ
        mpCanvas->setDrawMode(mpCanvas->Move);
        logMessage("Enter Move mode");
    }
}

//���ƶ���
void MainWindow::on_drawPolylineAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawPolyline);
}

//������Բ
void MainWindow::on_drawEllipseAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawEllipse);
}

//���ƾ���
void MainWindow::on_drawRectangleAct_triggered() {
    if (!mpCanvas) {
        logMessage("No valid layer!");
        return;
    }
    mpCanvas->setDrawMode(mpCanvas->DrawRectangle);
}


//----------դ�����----------

//��Ĥ��ȡ����ɣ�
void MainWindow::showMaskExtraction() {
    logMessage("Starting MaskExtraction.");

    MaskExtraction* maskExtraction = new MaskExtraction();
    maskExtraction->show();

    maskExtractionCommand* command = new maskExtractionCommand(maskExtraction);

    mUi.progressBar->setValue(0); // ��ʼ������

    connect(maskExtraction, &MaskExtraction::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(maskExtraction, &MaskExtraction::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(maskExtraction, &MaskExtraction::beginClicked, this, [this, command, maskExtraction]() {
        command->execute();
        if (maskExtraction->mstrInputRasterPath.isEmpty() && maskExtraction->mstrMaskPath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ��¼����
        Action action;
        action.type = Action::Add;

        QFileInfo fileInfo(maskExtraction->mstrInputRasterPath);
        QString baseName = fileInfo.baseName() + "(Mask Extraction)";

        QIcon fileIcon(":/login/res/maskExtraction.png");
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);

        RasterViewer rasterV;

        if (!maskExtraction->mstrOutputRasterPath.isEmpty()) { // ���·���Ƿ�Ϊ��
            action.fileNames << maskExtraction->mstrOutputRasterPath;
            QImage maskExtractionImage = rasterV.showRaster(maskExtraction->mstrOutputRasterPath);
            QStandardItemModel* mode = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (mode) {
                int index = mode->rowCount(); // ��ȡ��ǰ�������
                mMapFileIndex.insert(index, maskExtraction->mstrOutputRasterPath); // �洢�ļ���������
                mMapImageIndex.insert(index, maskExtractionImage);
                mode->appendRow(item);
                logMessage("Result of Mask Extraction added to list: " + baseName);
            }

            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(false);
            }
            mpCanvas->addLayer(maskExtractionImage); // �����ļ�·��
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

//�����������ɣ�
void MainWindow::showRasterNeighborhoodStatistics() {
    logMessage("Starting Raster Neighborhood Statistics Analysis.");

    RasterNeighborhoodStatistics* rasterNeighborhoodStatistics = new RasterNeighborhoodStatistics();
    rasterNeighborhoodStatistics->show();
    logMessage("Show Raster Neighborhood Statistics Analysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    rasterNeighborhoodStatisticsCommand* command = new rasterNeighborhoodStatisticsCommand(rasterNeighborhoodStatistics);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(rasterNeighborhoodStatistics, &RasterNeighborhoodStatistics::beginClicked, this, [this, command, rasterNeighborhoodStatistics]() {
        command->execute();
        // ����ļ����Ƿ�Ϊ��
        if (rasterNeighborhoodStatistics->mstrInputRasterPath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ��¼����
        Action action;
        action.type = Action::Add;

        if (!rasterNeighborhoodStatistics->mstrOutputMaxRasterPath.isEmpty()) {
            // ���ֵ
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMaxRasterPath);
            QString baseName = fileInfo.baseName() + "(Max)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMaxRasterPath.isEmpty()) { // ���·���Ƿ�Ϊ��
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMaxRasterPath;
                QImage maxImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMaxRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // ��ȡ��ǰ�������
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMaxRasterPath); // �洢�ļ���������
                    mMapImageIndex.insert(index, maxImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Max) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(maxImage); // �����ļ�·��
                logMessage("Result of Raster Neighborhood Statistics Analysis(Max) added to canvas: " + baseName);
            }
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputMinRasterPath.isEmpty()) {
            // ��Сֵ
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMinRasterPath);
            QString baseName = fileInfo.baseName() + "(Min)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMinRasterPath.isEmpty()) { // ���·���Ƿ�Ϊ��
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMinRasterPath;
                QImage minImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMinRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // ��ȡ��ǰ�������
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMinRasterPath); // �洢�ļ���������
                    mMapImageIndex.insert(index, minImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Min) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(minImage); // �����ļ�·��
                logMessage("Result of Raster Neighborhood Statistics Analysis(Min) added to canvas: " + baseName);
            }
        }
        else {
            return;
        }

        if (!rasterNeighborhoodStatistics->mstrOutputMeanRasterPath.isEmpty()) {
            // ��ֵ
            QFileInfo fileInfo(rasterNeighborhoodStatistics->mstrOutputMeanRasterPath);
            QString baseName = fileInfo.baseName() + "(Mean)";

            QIcon fileIcon(":/login/res/domainAnalysis.png");
            QStandardItem* item = new QStandardItem(fileIcon, baseName);
            item->setCheckable(true);
            item->setCheckState(Qt::Checked);

            RasterViewer rasterV;

            if (!rasterNeighborhoodStatistics->mstrOutputMeanRasterPath.isEmpty()) { // ���·���Ƿ�Ϊ��
                action.fileNames << rasterNeighborhoodStatistics->mstrOutputMeanRasterPath;
                QImage meanImage = rasterV.showRaster(rasterNeighborhoodStatistics->mstrOutputMeanRasterPath);
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
                if (model) {
                    int index = model->rowCount(); // ��ȡ��ǰ�������
                    mMapFileIndex.insert(index, rasterNeighborhoodStatistics->mstrOutputMeanRasterPath); // �洢�ļ���������
                    mMapImageIndex.insert(index, meanImage);
                    model->appendRow(item);
                    logMessage("Result of Raster Neighborhood Statistics Analysis(Mean) added to list: " + baseName);
                }

                if (!mpCanvas) {
                    mpCanvas = new LayeredCanvas(this);
                    mUi.scrollArea->setWidget(mpCanvas);
                    mUi.scrollArea->setWidgetResizable(false);
                }
                mpCanvas->addLayer(meanImage); // �����ļ�·��
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

//դ�񲨶η�������ɣ�
void MainWindow::showRasterBandAnalysis()
{
    logMessage("Starting Raster Band Analysis.");

    RasterBandAnalysis* processRaster = new RasterBandAnalysis();
    processRaster->show();
    logMessage("Show Raster Band Analysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    // �����������
    rasterBandAnalysisCommand* command = new rasterBandAnalysisCommand(processRaster);

    connect(processRaster, &RasterBandAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(processRaster, &RasterBandAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(processRaster, &RasterBandAnalysis::beginClicked, this, [this, command, processRaster]() {
        command->execute();
        if (processRaster->mstrRasterFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ��¼����
        Action action;
        action.type = Action::Add;

        // ���ɫ��ʾ
        QFileInfo fileInfoTrue(processRaster->mstrRasterFilePath);
        QString baseName1 = fileInfoTrue.baseName() + "(True Color)";

        QIcon fileIconTrue(":/login/res/tureColor.png");
        QStandardItem* item1 = new QStandardItem(fileIconTrue, baseName1);
        item1->setCheckable(true);
        item1->setCheckState(Qt::Checked);

        RasterViewer rasterV;

        if (!processRaster->mstrTrueColorPath.isEmpty()) { // ���·���Ƿ�Ϊ��
            action.fileNames << processRaster->mstrTrueColorPath;
            QImage trueColorImage = rasterV.showRaster(processRaster->mstrTrueColorPath);
            QStandardItemModel* model1 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model1) {
                int index = model1->rowCount(); // ��ȡ��ǰ�������
                mMapFileIndex.insert(index, processRaster->mstrTrueColorPath); // �洢�ļ���������
                mMapImageIndex.insert(index, trueColorImage);
                model1->appendRow(item1);
                logMessage("Result of True Color Display added to list: " + baseName1);
            }

            if (!mpCanvas) {
                mpCanvas = new LayeredCanvas(this);
                mUi.scrollArea->setWidget(mpCanvas);
                mUi.scrollArea->setWidgetResizable(false);
            }
            mpCanvas->addLayer(trueColorImage); // �����ļ�·��
            logMessage("Result of True Color Display added to canvas: " + baseName1);
        }
        else {
            return;
        }

        //�ٲ�ɫ��ʾ
        QFileInfo fileInfoFalse(processRaster->mstrRasterFilePath);
        QString baseName2 = fileInfoFalse.baseName() + "(False Color)";

        QIcon fileIconFalse(":/login/res/tureColor.png");
        QStandardItem* item2 = new QStandardItem(fileIconFalse, baseName2);
        item2->setCheckable(true);
        item2->setCheckState(Qt::Checked);

        if (!processRaster->mstrFalseColorPath.isEmpty()) { // ���·���Ƿ�Ϊ��
            action.fileNames << processRaster->mstrFalseColorPath;
            QImage falseColorImage = rasterV.showRaster(processRaster->mstrFalseColorPath);

            QStandardItemModel* model2 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model2) {
                int index = model2->rowCount(); // ��ȡ��ǰ�������
                mMapFileIndex.insert(index, processRaster->mstrFalseColorPath);
                mMapImageIndex.insert(index, falseColorImage);
                model2->appendRow(item2);
                logMessage("Result of False Color Display added to list: " + baseName2);
            }

            mpCanvas->addLayer(falseColorImage); // �����ļ�·��
            logMessage("Result of False Color Display added to canvas: " + baseName2);
        }
        else {
            logMessage("False Color path is empty. Skipping False Color display.");
        }

        // �Ҷ�ֱ��ͼ
        QFileInfo fileInfoGray(processRaster->mstrRasterFilePath);
        QString baseName3 = fileInfoGray.baseName() + "(Gray Histogram)";

        QIcon fileIconGray(":/login/res/gray.png");
        QStandardItem* item3 = new QStandardItem(fileIconGray, baseName3);
        item3->setCheckable(true);
        item3->setCheckState(Qt::Checked);

        if (!processRaster->mstrGrayHistogramPath.isEmpty()) { // ���·���Ƿ�Ϊ��
            action.fileNames << processRaster->mstrGrayHistogramPath;
            QImage grayImage = rasterV.showRaster(processRaster->mstrGrayHistogramPath);

            QStandardItemModel* model3 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model3) {
                int index = model3->rowCount(); // ��ȡ��ǰ�������
                mMapFileIndex.insert(index, processRaster->mstrGrayHistogramPath);
                mMapImageIndex.insert(index, grayImage);
                model3->appendRow(item3);
                logMessage("Result of Gray Histogram added to list: " + baseName3);
            }

            mpCanvas->addLayer(grayImage); // �����ļ�·��
            logMessage("Result of Gray Histogram added to canvas: " + baseName3);
        }
        else {
            logMessage("Gray Histogram path is empty. Skipping Gray Histogram display.");
        }

        // ֱ��ͼ���⻯
        QFileInfo fileInfoEnhanced(processRaster->mstrRasterFilePath);
        QString baseName4 = fileInfoEnhanced.baseName() + "(Histogram Equalization)";

        QIcon fileIconEnhanced(":/login/res/enhanced.png");
        QStandardItem* item4 = new QStandardItem(fileIconEnhanced, baseName4);
        item4->setCheckable(true);
        item4->setCheckState(Qt::Checked);

        if (!processRaster->mstrEnhancedHistogramPath.isEmpty()) { // ���·���Ƿ�Ϊ��
            action.fileNames << processRaster->mstrEnhancedHistogramPath;
            QImage enhancedImage = rasterV.showRaster(processRaster->mstrEnhancedHistogramPath);

            QStandardItemModel* model4 = qobject_cast<QStandardItemModel*>(mUi.listView->model());
            if (model4) {
                int index = model4->rowCount(); // ��ȡ��ǰ�������
                mMapFileIndex.insert(index, processRaster->mstrEnhancedHistogramPath);
                mMapImageIndex.insert(index, enhancedImage);
                model4->appendRow(item4);
                logMessage("Result of Histogram Equalization added to list: " + baseName4);
            }

            mpCanvas->addLayer(enhancedImage); // �����ļ�·��
            logMessage("Result of Histogram Equalization added to canvas: " + baseName4);
        }
        else {
            logMessage("Enhanced Histogram path is empty. Skipping Histogram Equalization display.");
        }

        logMessage("RasterBandAnalysis runs successfully on file: " + processRaster->mstrRasterFilePath);

        mvActionHistory.push(action);

        // ɾ������
        processRaster->deleteLater();
        delete  command;
        });
}


//----------ʸ������----------

//��������������ɣ�
void MainWindow::showVectorLayerBufferAnalysis() {
    logMessage("Starting Vector Layer Buffer Analysis.");

    VectorLayerBufferAnalysis* vectorLayerBufferAnalysis = new VectorLayerBufferAnalysis();
    vectorLayerBufferAnalysis->show();
    logMessage("Show Vector Layer Buffer Analysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    VectorLayerBufferAnalysisCommand* command = new VectorLayerBufferAnalysisCommand(vectorLayerBufferAnalysis);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(vectorLayerBufferAnalysis, &VectorLayerBufferAnalysis::beginClicked, this, [this, command, vectorLayerBufferAnalysis]() {
        command->execute();
        if (vectorLayerBufferAnalysis->mstrFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ���ļ�·������ȡ�ļ���
        QFileInfo fileInfo(vectorLayerBufferAnalysis->mstrFilePath);
        QString baseName = fileInfo.baseName() + "(Buffer)"; // �ļ�������������չ����

        // ����ͼ�������ͼ��
        QIcon fileIcon(":/login/res/bufferAnalysis.png");
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true);
        item->setCheckState(Qt::Checked);

        VectorLayerRenderer vectorLayerRenderer(vectorLayerBufferAnalysis->mstrResultPath);
        QImage resultImage = vectorLayerRenderer.renderToImage();

        // ��ͼ������ӵ� listView �� model ��
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // ��ȡ��ǰ�������
            mMapFileIndex.insert(index, vectorLayerBufferAnalysis->mstrResultPath);
            mMapImageIndex.insert(index, resultImage);
            model->appendRow(item);

            logMessage("Result of Vector Layer Buffer Analysis added to list: " + baseName);
        }

        // ����Ⱦ��ͼ����ӵ� scrollArea �е� LayeredCanvas
        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }
        mpCanvas->addLayer(resultImage); // �����ļ�·��
        logMessage("Result of Vector Layer Buffer Analysis added to canvas:" + baseName);

        logMessage("Vector Layer Buffer Analysis runs successfully on file: " + vectorLayerBufferAnalysis->mstrFilePath);

        // ��¼����
        Action action;
        action.type = Action::Add;
        action.fileNames << vectorLayerBufferAnalysis->mstrResultPath;
        mvActionHistory.push(action);

        // ������Դ
        delete vectorLayerBufferAnalysis;
        delete command;
        });
}

//ͳ�Ʒ�������ɣ�
void MainWindow::showStatisticAnalysis() {
    logMessage("Starting Vector Statistic Analysis.");

    StatisticAnalysis* statisticAnalysis = new StatisticAnalysis();
    statisticAnalysis->show();
    logMessage("Show Vector Statistic Analysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    StatisticAnalysisCommand* command = new StatisticAnalysisCommand(statisticAnalysis);

    connect(statisticAnalysis, &StatisticAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(statisticAnalysis, &StatisticAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(statisticAnalysis, &StatisticAnalysis::beginClicked, this, [this, command, statisticAnalysis]() {
        command->execute();
        if (statisticAnalysis->mstrFileName.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        logMessage("Result of Vector Statistic Analysis saved successfully: " + statisticAnalysis->mstrSaveFileName);

        logMessage("Vector Statistic Analysis runs successfully on file: " + statisticAnalysis->mstrFileName);

        delete command;
        delete statisticAnalysis;
        });
}

//��������������ɣ�
void MainWindow::showTriangulation() {
    logMessage("Starting TriangularNetworkAnalysis.");

    Triangulation* triangulation = new Triangulation();
    triangulation->show();
    logMessage("Show TriangularNetworkAnalysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    TriangulationCommand* command = new TriangulationCommand(triangulation);

    connect(triangulation, &Triangulation::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(triangulation, &Triangulation::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(triangulation, &Triangulation::beginClicked, this, [this, command, triangulation]() {
        command->execute();
        // ����ļ����Ƿ�Ϊ��
        if (triangulation->mstrInputFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ���ļ�·������ȡ�ļ���
        QFileInfo fileInfo(triangulation->mstrInputFilePath);
        QString baseName = fileInfo.baseName() + "(TriangularNetwork)"; // �ļ�������������չ����

        // ����ͼ�������ͼ��
        QIcon fileIcon(":/login/res/Triangulation.png"); // ������Դ�ļ�·���޸�
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // ���ø�ѡ��
        item->setCheckState(Qt::Checked); // Ĭ��ѡ��

        VectorLayerRenderer vectorLayerRenderer(triangulation->mstrResultPath);
        QImage resultImage = vectorLayerRenderer.renderToImage();

        // ��ͼ������ӵ� listView �� model ��
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // ��ȡ��ǰ�������
            mMapFileIndex.insert(index, triangulation->mstrResultPath);
            mMapImageIndex.insert(index, resultImage); // �洢������ QImage ��ӳ��
            model->appendRow(item);
            logMessage("Result of TriangularNetworkAnalysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // ��������뵽��ͼ
        mpCanvas->addLayer(resultImage); // �����ļ�·��
        logMessage("Result of TriangularNetworkAnalysis added to canvas : " + baseName);

        logMessage("TriangularNetworkAnalysis runs successfully on file: " + triangulation->mstrInputFilePath);

        // ��¼����
        Action action;
        action.type = Action::Add;
        action.fileNames << triangulation->mstrResultPath;
        mvActionHistory.push(action);

        // ɾ�� triangulation ����
        delete command;
        triangulation->deleteLater();
        });
}

//Voronoiͼ����ɣ�
void MainWindow::showVoronoiAnalysis() {
    logMessage("Starting VoronoiAnalysis.");

    VoronoiAnalysis* voronoiAnalysis = new VoronoiAnalysis();
    voronoiAnalysis->show();
    logMessage("Show VoronoiAnalysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    VoronoiAnalysisCommand* command = new VoronoiAnalysisCommand(voronoiAnalysis);

    connect(voronoiAnalysis, &VoronoiAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(voronoiAnalysis, &VoronoiAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(voronoiAnalysis, &VoronoiAnalysis::beginClicked, this, [this, command, voronoiAnalysis]() {
        command->execute();
        // ����ļ����Ƿ�Ϊ��
        if (voronoiAnalysis->mstrInputFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ���ļ�·������ȡ�ļ���
        QFileInfo fileInfo(voronoiAnalysis->mstrInputFilePath);
        QString baseName = fileInfo.baseName() + "(VoronoiAnalysis)"; // �ļ�������������չ����

        // ����ͼ�������ͼ��
        QIcon fileIcon(":/login/res/VoronoiAnalysis.png"); // ������Դ�ļ�·���޸�
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // ���ø�ѡ��
        item->setCheckState(Qt::Checked); // Ĭ��ѡ��

        VectorLayerRenderer vectorLayerRenderer(voronoiAnalysis->mstrResultPath);
        QImage voronoiImage = vectorLayerRenderer.renderToImage();

        // ��ͼ������ӵ� listView �� model ��
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // ��ȡ��ǰ�������
            mMapFileIndex.insert(index, voronoiAnalysis->mstrResultPath);
            mMapImageIndex.insert(index, voronoiImage); // �洢������ QImage ��ӳ��
            model->appendRow(item);
            logMessage("Result of VoronoiAnalysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // ��������뵽��ͼ
        mpCanvas->addLayer(voronoiImage); // �����ļ�·��

        logMessage("Result of VoronoiAnalysis added to canvas : " + baseName);

        logMessage("VoronoiAnalysis runs successfully on file: " + voronoiAnalysis->mstrInputFilePath);

        // ��¼����
        Action action;
        action.type = Action::Add;
        action.fileNames << voronoiAnalysis->mstrResultPath;
        mvActionHistory.push(action);

        // ɾ������
        delete command;
        voronoiAnalysis->deleteLater();
        });
}

//͹����������ɣ�
void MainWindow::showConvexHullAnalysis() {
    logMessage("Starting Convex Hull Analysis.");

    ConvexAnalysis* convexAnalysis = new ConvexAnalysis();
    convexAnalysis->show();
    logMessage("Show Convex Hull Analysis successfully.");

    mUi.progressBar->setValue(0); // ��ʼ������

    ConvexAnalysisCommand* command = new ConvexAnalysisCommand(convexAnalysis);

    connect(convexAnalysis, &ConvexAnalysis::progressUpdated, this, &MainWindow::onAnalysisProgressUpdated);

    connect(convexAnalysis, &ConvexAnalysis::analysisProgressGoing, this, &MainWindow::onAnalysisProgressGoing);

    connect(convexAnalysis, &ConvexAnalysis::beginClicked, this, [this, command, convexAnalysis]() {
        command->execute();
        // ����ļ����Ƿ�Ϊ��
        if (convexAnalysis->mstrFilePath.isEmpty()) {
            logMessage("No file found. Operation skipped.");
            return; // �ļ���Ϊ�գ�������������
        }

        // ���ļ�·������ȡ�ļ���
        QFileInfo fileInfo(convexAnalysis->mstrFilePath);
        QString baseName = fileInfo.baseName() + "(ConvexHullAnalysis)"; // �ļ�������������չ����

        // ����ͼ�������ͼ��
        QIcon fileIcon(":/login/res/convexHullAnalysis.png"); // ������Դ�ļ�·���޸�
        QStandardItem* item = new QStandardItem(fileIcon, baseName);
        item->setCheckable(true); // ���ø�ѡ��
        item->setCheckState(Qt::Checked); // Ĭ��ѡ��

        VectorLayerRenderer vectorLayerRenderer(convexAnalysis->mstrSavePath);
        QImage convexImage = vectorLayerRenderer.renderToImage();

        // ��ͼ������ӵ� listView �� model ��
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mUi.listView->model());
        if (model) {
            int index = model->rowCount(); // ��ȡ��ǰ�������
            mMapFileIndex.insert(index, convexAnalysis->mstrSavePath);
            mMapImageIndex.insert(index, convexImage); // �洢������ QImage ��ӳ��
            model->appendRow(item);
            logMessage("Result of Convex Hull Analysis added to list: " + baseName);
        }

        if (!mpCanvas) {
            mpCanvas = new LayeredCanvas(this);
            mUi.scrollArea->setWidget(mpCanvas);
            mUi.scrollArea->setWidgetResizable(false);
        }

        // ��������뵽��ͼ
        mpCanvas->addLayer(convexImage); // �����ļ�·��

        logMessage("Result of Convex Hull Analysis added to canvas : " + baseName);

        logMessage("Convex Hull Analysis runs successfully on file: " + convexAnalysis->mstrFilePath);

        // ��¼����
        Action action;
        action.type = Action::Add;
        action.fileNames << convexAnalysis->mstrSavePath;
        mvActionHistory.push(action);

        // ɾ������
        delete command;
        convexAnalysis->deleteLater();
        });


}


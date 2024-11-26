#include "ConvexAnalysis.h"
#include <iostream>
#include <chrono>

ConvexAnalysis::ConvexAnalysis(QWidget* parent)
{
	mUi.setupUi(this);

	setWindowTitle("Convex Hull Analysis");
	setWindowIcon(QIcon(":/login/res/convexHullAnalysis.png"));

	connect(mUi.openFile, &QPushButton::clicked, this, &ConvexAnalysis::onBrowseButtonClicked);
	connect(mUi.outputVectorButton, &QPushButton::clicked, this, &ConvexAnalysis::onSaveShpFileButtonClicked);
	connect(mUi.begin, &QPushButton::clicked, this, &ConvexAnalysis::beginClicked);
}

//打开文件
void ConvexAnalysis::onBrowseButtonClicked()
{
	mstrFilePath = QFileDialog::getOpenFileName(
		this,
		tr("Open File"),
		"/home",
		tr("Shapefile (*.shp)"));

	if (mstrFilePath.isEmpty())
		return;

	QFileInfo fileInfo(mstrFilePath);
	QString baseName = fileInfo.fileName();
	mUi.lineEditVector->setText(baseName);
}

//保存文件
void ConvexAnalysis::onSaveShpFileButtonClicked()
{
	mstrSavePath = QFileDialog::getSaveFileName(this,
		tr("Save File"),
		"/home",
		tr("Shapefile (*.shp)"));

	if (mstrSavePath.isEmpty())
		return;

	mUi.lineEditOutputVector->setText(mstrSavePath);
}

//开始分析
void ConvexAnalysis::onAnalyzeButtonClicked()
{
	if (mstrFilePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No input file selected."));
		return;
	}

	if (mstrSavePath.isEmpty()) {
		QMessageBox::warning(this, tr("Warning"), tr("No output file selected."));
		return;
	}

	// 记录开始时间
	auto startTime = std::chrono::high_resolution_clock::now();

	// 首先加载数据
	loadAndProcessData(mstrFilePath);

	// 进行凸包分析
	performConvexHullAnalysis();

	// 保存凸包结果到指定路径
	if (!mstrSavePath.isEmpty())
	{
		saveConvexHullAsShp(mstrSavePath);
		QMessageBox::information(this, tr("Success"), tr("Convex Hull Analysis runs successfully."));
	}
	else
	{
		QMessageBox::warning(this, "Err", "Save path is empty!");
	}

	emit progressUpdated(100);

	// 记录结束时间
	auto endTime = std::chrono::high_resolution_clock::now();
	// 计算经过的时间（单位：秒）
	double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

	// 限制精度到 0.001
	elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

	// 调用主窗口的槽函数更新界面
	emit analysisProgressGoing(elapsedTime);
}

//处理数据
void ConvexAnalysis::loadAndProcessData(const QString& fileName)
{
	GDALAllRegister();

	GDALDataset* poDS = (GDALDataset*)GDALOpenEx(fileName.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

	if (poDS == NULL)
	{
		QMessageBox::warning(this, "Error", "Failed to open the file: " + fileName);
		return;
	}

	OGRLayer* poLayer = poDS->GetLayer(0);

	// 获取层的范围用于缩放
	OGREnvelope env;
	poLayer->GetExtent(&env);

	// 计算初始缩放因子
	double scaleX = width() / (env.MaxX - env.MinX);
	double scaleY = height() / (env.MaxY - env.MinY);
	double minX = env.MinX;
	double minY = env.MinY;

	emit progressUpdated(10);

	poLayer->ResetReading();
	OGRFeature* poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != NULL)
	{
		OGRGeometry* poGeometry = poFeature->GetGeometryRef();
		if (poGeometry != NULL)
		{
			renderGeometry(poGeometry, scaleX, scaleY, minX, minY, true);
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	GDALClose(poDS);

	emit progressUpdated(20);
}

//凸包分析
void ConvexAnalysis::performConvexHullAnalysis()
{
	GDALAllRegister();

	GDALDataset* poDS = (GDALDataset*)GDALOpenEx(mstrFilePath.toStdString().c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL);

	if (poDS == NULL)
	{
		QMessageBox::warning(this, "Error", "Failed to open the file for convex hull analysis: " + mstrFilePath);
		return;
	}

	OGRLayer* poLayer = poDS->GetLayer(0);

	// 获取输入图层的空间参考
	OGRSpatialReference* inputSRS = poLayer->GetSpatialRef();

	emit progressUpdated(30);

	OGRSpatialReference* outputSRS = new OGRSpatialReference();
	if (inputSRS != NULL)
	{
	}

	OGRGeometryCollection* poGeometryCollection = new OGRGeometryCollection();

	poLayer->ResetReading();
	OGRFeature* poFeature;
	while ((poFeature = poLayer->GetNextFeature()) != NULL)
	{
		OGRGeometry* poGeometry = poFeature->GetGeometryRef();
		if (poGeometry != NULL)
		{
			poGeometryCollection->addGeometry(poGeometry);
		}
		OGRFeature::DestroyFeature(poFeature);
	}

	OGRGeometry* poConvexHull = poGeometryCollection->ConvexHull();

	emit progressUpdated(40);

	// 为凸包结果设置与输入相同的空间参考
	if (outputSRS != NULL)
	{
		poConvexHull->assignSpatialReference(outputSRS);
	}

	this->mpPoConvexHull = poConvexHull;  // 保存凸包结果到成员变量


	OGRGeometryFactory::destroyGeometry(poGeometryCollection);

	GDALClose(poDS);

	emit progressUpdated(50);
}

//保存成shp
void ConvexAnalysis::saveConvexHullAsShp(const QString& filePath)
{
	GDALAllRegister();

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (poDriver == NULL)
	{
		QMessageBox::warning(this, "Error", "获取驱动失败.");
		return;
	}

	emit progressUpdated(60);

	GDALDataset* poDSOut = poDriver->Create(filePath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (poDSOut == NULL)
	{
		QMessageBox::warning(this, "Error", "创建输出数据源失败.");
		return;
	}

	emit progressUpdated(70);

	OGRLayer* poLayerOut = poDSOut->CreateLayer("convex_hull", NULL, wkbPolygon, NULL);
	if (poLayerOut == NULL)
	{
		QMessageBox::warning(this, "Error", "创建输出图层失败.");
		GDALClose(poDSOut);
		return;
	}

	emit progressUpdated(80);

	// 使用在 performConvexHullAnalysis 中保存的凸包结果，并确保其空间参考也被设置
	OGRGeometry* poConvexHull = this->mpPoConvexHull;
	if (poConvexHull->getSpatialReference() == NULL)
	{
		// 如果凸包的空间参考丢失，重新设置
		poConvexHull->assignSpatialReference(poConvexHull->getSpatialReference());
	}

	OGRFeature* poFeatureOut = OGRFeature::CreateFeature(poLayerOut->GetLayerDefn());
	poFeatureOut->SetGeometry(poConvexHull);
	if (poLayerOut->CreateFeature(poFeatureOut) != OGRERR_NONE)
	{
		QMessageBox::warning(this, "Error", "创建要素失败.");
	}

	OGRFeature::DestroyFeature(poFeatureOut);

	GDALClose(poDSOut);

	emit progressUpdated(90);
}

//绘制
void ConvexAnalysis::renderGeometry(OGRGeometry* geometry, double scaleX, double scaleY, double minX, double minY, bool isOriginal)
{
	std::cout << "Rendering geometry. Is original: " << isOriginal << std::endl;
	if (wkbFlatten(geometry->getGeometryType()) == wkbPoint)
	{
		OGRPoint* point = (OGRPoint*)geometry;
		if (isOriginal)
		{
		}
		else
		{
		}
		int x = static_cast<int>((point->getX() - minX) * scaleX);
		int y = static_cast<int>((height() - (point->getY() - minY) * scaleY));
		std::cout << "Point coordinates: (" << x << ", " << y << ")" << std::endl;
	}
	else if (wkbFlatten(geometry->getGeometryType()) == wkbLineString)
	{
		OGRLineString* line = (OGRLineString*)geometry;
		if (isOriginal)
		{
		}
		else
		{
		}
		for (int i = 0; i < line->getNumPoints(); ++i)
		{
			int x = static_cast<int>((line->getX(i) - minX) * scaleX);
			int y = static_cast<int>((height() - (line->getY(i) - minY) * scaleY));
			std::cout << "Line point coordinates: (" << x << ", " << y << ")" << std::endl;
		}
	}
	else if (wkbFlatten(geometry->getGeometryType()) == wkbPolygon)
	{
		OGRPolygon* polygon = (OGRPolygon*)geometry;
		if (isOriginal)
		{
		}
		else
		{
		}
		for (int i = 0; i < polygon->getExteriorRing()->getNumPoints(); ++i)
		{
			int x = static_cast<int>((polygon->getExteriorRing()->getX(i) - minX) * scaleX);
			int y = static_cast<int>((height() - (polygon->getExteriorRing()->getY(i) - minY) * scaleY));
			std::cout << "Polygon point coordinates: (" << x << ", " << y << ")" << std::endl;
		}
	}
}
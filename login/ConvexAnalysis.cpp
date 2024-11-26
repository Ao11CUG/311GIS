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

//���ļ�
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

//�����ļ�
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

//��ʼ����
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

	// ��¼��ʼʱ��
	auto startTime = std::chrono::high_resolution_clock::now();

	// ���ȼ�������
	loadAndProcessData(mstrFilePath);

	// ����͹������
	performConvexHullAnalysis();

	// ����͹�������ָ��·��
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

	// ��¼����ʱ��
	auto endTime = std::chrono::high_resolution_clock::now();
	// ���㾭����ʱ�䣨��λ���룩
	double elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime).count();

	// ���ƾ��ȵ� 0.001
	elapsedTime = std::round(elapsedTime * 1000.0) / 1000.0;

	// ���������ڵĲۺ������½���
	emit analysisProgressGoing(elapsedTime);
}

//��������
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

	// ��ȡ��ķ�Χ��������
	OGREnvelope env;
	poLayer->GetExtent(&env);

	// �����ʼ��������
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

//͹������
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

	// ��ȡ����ͼ��Ŀռ�ο�
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

	// Ϊ͹�����������������ͬ�Ŀռ�ο�
	if (outputSRS != NULL)
	{
		poConvexHull->assignSpatialReference(outputSRS);
	}

	this->mpPoConvexHull = poConvexHull;  // ����͹���������Ա����


	OGRGeometryFactory::destroyGeometry(poGeometryCollection);

	GDALClose(poDS);

	emit progressUpdated(50);
}

//�����shp
void ConvexAnalysis::saveConvexHullAsShp(const QString& filePath)
{
	GDALAllRegister();

	GDALDriver* poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
	if (poDriver == NULL)
	{
		QMessageBox::warning(this, "Error", "��ȡ����ʧ��.");
		return;
	}

	emit progressUpdated(60);

	GDALDataset* poDSOut = poDriver->Create(filePath.toStdString().c_str(), 0, 0, 0, GDT_Unknown, NULL);
	if (poDSOut == NULL)
	{
		QMessageBox::warning(this, "Error", "�����������Դʧ��.");
		return;
	}

	emit progressUpdated(70);

	OGRLayer* poLayerOut = poDSOut->CreateLayer("convex_hull", NULL, wkbPolygon, NULL);
	if (poLayerOut == NULL)
	{
		QMessageBox::warning(this, "Error", "�������ͼ��ʧ��.");
		GDALClose(poDSOut);
		return;
	}

	emit progressUpdated(80);

	// ʹ���� performConvexHullAnalysis �б����͹���������ȷ����ռ�ο�Ҳ������
	OGRGeometry* poConvexHull = this->mpPoConvexHull;
	if (poConvexHull->getSpatialReference() == NULL)
	{
		// ���͹���Ŀռ�ο���ʧ����������
		poConvexHull->assignSpatialReference(poConvexHull->getSpatialReference());
	}

	OGRFeature* poFeatureOut = OGRFeature::CreateFeature(poLayerOut->GetLayerDefn());
	poFeatureOut->SetGeometry(poConvexHull);
	if (poLayerOut->CreateFeature(poFeatureOut) != OGRERR_NONE)
	{
		QMessageBox::warning(this, "Error", "����Ҫ��ʧ��.");
	}

	OGRFeature::DestroyFeature(poFeatureOut);

	GDALClose(poDSOut);

	emit progressUpdated(90);
}

//����
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
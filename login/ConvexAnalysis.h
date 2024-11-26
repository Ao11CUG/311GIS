#include <QObject>
#include <QImage>
#include <gdal.h>
#include <ogr_api.h>
#include <QFileDialog>
#include <QMessageBox>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <ogr_core.h>
#include <ogrsf_frmts.h>
#include <ogr_feature.h>
#include <ogr_geometry.h>
#include <QPainter>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <ui_ConvexAnalysis.h>

class ConvexAnalysis : public QWidget
{
	Q_OBJECT

public:
	ConvexAnalysis(QWidget* parent = nullptr);

	QString mstrFilePath;
	QString mstrSavePath;
	OGRGeometry* mpPoConvexHull;

public slots:
	void onBrowseButtonClicked();
	void onSaveShpFileButtonClicked();
	void onAnalyzeButtonClicked();  // �����ۺ�������

signals:
	void beginClicked();       // ��ʼ������ź�
	void progressUpdated(int progress); // ������ȸ����ź�
	void analysisProgressGoing(double elapsedTime);

private:
	Ui::ConvexAnalysisClass mUi;
	void loadAndProcessData(const QString& fileName);
	void performConvexHullAnalysis();
	void saveConvexHullAsShp(const QString& filePath);
	void renderGeometry(OGRGeometry* geometry, double scaleX, double scaleY, double minX, double minY, bool isOriginal);
};

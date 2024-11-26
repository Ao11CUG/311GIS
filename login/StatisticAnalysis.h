#include <QWidget>
#include <QTableWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <iostream>
#include <fstream>
#include <QString>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include "ui_StatisticAnalysis.h"

class StatisticAnalysis : public QWidget
{
	Q_OBJECT

public:
	StatisticAnalysis(QWidget *parent = nullptr);
	~StatisticAnalysis();

	QString mstrFileName;
	QString mstrSaveFileName;

public slots:
	void onOpenFileButtonClicked();
	void onSaveFileButtonClicked();
	void processFile();
	void saveResultsToCSV(const QString& filePath);

signals:
	void beginClicked();
	void progressUpdated(int progress); // 定义进度更新信号
	void analysisProgressGoing(double elapsedTime);

private:
	QTableWidget* mpTableWidget;
	Ui::statisticAnalysisClass mUi;
};

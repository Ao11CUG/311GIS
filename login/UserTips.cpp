#include "UserTips.h"
#include <QPalette>
#include <QPixmap>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>


UserTips::UserTips(QWidget* parent)
	: QWidget(parent)
{
	mUi.setupUi(this);

	setWindowTitle("User Tips");
	setWindowIcon(QIcon(":/login/res/UserTips.png"));

	QPixmap background1(":/login/res/311GIS.png"); // ʹ����Դ�ļ�·��
	QPixmap scaledBackground1 = background1.scaled(mUi.icon->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QPalette palette1 = mUi.icon->palette();
	palette1.setBrush(QPalette::Window, scaledBackground1);
	mUi.icon->setPalette(palette1);
	mUi.icon->setAutoFillBackground(true);

	QPixmap background2(":/login/res/cug.jpg"); // ʹ����Դ�ļ�·��
	QPixmap scaledBackground2 = background2.scaled(mUi.cug->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	QPalette palette2 = mUi.cug->palette();
	palette2.setBrush(QPalette::Window, scaledBackground2);
	mUi.cug->setPalette(palette2);
	mUi.cug->setAutoFillBackground(true);

	connect(mUi.ReadMe, &QPushButton::clicked, this, &UserTips::openPdf);
}

UserTips::~UserTips()
{}

//��pdf
void UserTips::openPdf() {
    QString pdfPath = ":/login/res/ReadMe.pdf"; // ʹ����Դ·��

    // ����Դ�ļ�
    QFile file(pdfPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open PDF file from resources.";
        return;
    }

    // ��ȡ��ʱ·��
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFilePath = QDir(tempDir).filePath("ReadMe.pdf");

    // ����Դ�ļ�����д����ʱ�ļ�
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to create temporary file.";
        return;
    }
    tempFile.write(file.readAll());
    tempFile.close();

    // ʹ��QDesktopServices��PDF�ļ�
    QDesktopServices::openUrl(QUrl::fromLocalFile(tempFilePath));
    qDebug() << "PDF opened successfully.";
}
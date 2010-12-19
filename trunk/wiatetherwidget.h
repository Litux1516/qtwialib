#ifndef WIATETHERWIDGET_H
#define WIATETHERWIDGET_H

#include <QWidget>
#include <QMap>
#include <QList>
#include <QGraphicsScene>
#include <QPixmap>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include "wia.h"
#include "ui_wiatetherwidget.h"
#include "qtwialib_global.h"
#include "dcrimage.h"

class QTWIALIB_EXPORT WIATetherWidget : public QWidget
{
	Q_OBJECT

public:
	WIATetherWidget(QWidget *parent = 0);
	~WIATetherWidget();

private:
	Ui::WIATetherWidget ui;
	QMap<int, QString> _allexposuretimes;
	QMap<int, QString> _whitebalances;
	QList<QString> _fnumbers;
	QList<QString> _isonumbers;
	QList<int> _camerasupportedexposuretimes;
	QList<int> _camerasupportedexposurecomp;
	QMap<QString, QString>_imageformats;
	QString _destinationfolder;
	bool shootingraw;
	QTimer *timer;
	QTimer *timelapsetimer;
	int timelapsefreq;
	int timelapsedelay;
	int numshotspertimeout;
	int totalnumbershots;
	int secondsdelayed;
	int currentnumberofshots;
	int last;
	bool ifsleepneeded;
	bool inunsupportedmode;

	bool hasExposureTimes;
	bool hasExposureComp;
	bool hasFNumbers;
	bool hasISO;
	bool hasWhiteBalance;

	WIA::IDevice *device;
	

	
	void setupshuttertable();
	void setupfnumbers();
	void setupexposureindexes();
	void setupexposuretimes();
	void setupexposurecompensation();
	void setupbatterystatus();
	void setupwhitebalancetable();
	void setupwhitebalance();
	void setupimageformats();
	void nocameradetected();
	void cameradetected();
private slots:
	void on_dial1_valueChanged();
	void on_hsbDial2_valueChanged();
	void on_pbShutter_clicked();
	void on_hsExposureComp_valueChanged();
	void on_tbISO_toggled(bool checked);
	void on_tbWB_toggled(bool checked);
	void on_tbFMT_toggled(bool checked);
	void on_pbsetstorage_clicked();
	void on_cbAutoOnOff_toggled(bool checked);
	void on_timer_timeout();
	void on_pbTimeLapse_clicked();
	void on_timelapse_timeout();
	void on_wiadlg_exception(int err, QString err1, QString err2, QString err3);

signals:
	void newImageDownloaded(QString filename);
//	void imageDownloaded(QString file);
};

#endif // WIATETHERWIDGET_H

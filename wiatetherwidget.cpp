#include "wiatetherwidget.h"

const QString wiaCommandTakePicture = "{AF933CAC-ACAD-11D2-A093-00C04F72DC3C}";
const QString wiaCommandSynchronize = "{9B26B7B2-ACAD-11D2-A093-00C04F72DC3C}";
const QString wiaFormatBMP = "{B96B3CAB-0728-11D3-9D7B-0000F81EF32E}";
const QString wiaFormatGIF = "{B96B3CB0-0728-11D3-9D7B-0000F81EF32E}";
const QString wiaFormatJPEG = "{B96B3CAE-0728-11D3-9D7B-0000F81EF32E}";
const QString wiaFormatPNG = "{B96B3CAF-0728-11D3-9D7B-0000F81EF32E}";
const QString wiaFormatTIFF = "{B96B3CB1-0728-11D3-9D7B-0000F81EF32E}";
const QString wiaFormatRAW = "{B96B3CA9-0728-11D3-9D7B-0000F81EF32E}";

WIATetherWidget::WIATetherWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	inunsupportedmode = false;
	device = 0;
	last = -1;
	ifsleepneeded = false;
	timer = new QTimer();
	connect(timer,SIGNAL(timeout()),this,SLOT(on_timer_timeout()));

	timelapsetimer = new QTimer();
	connect(timelapsetimer, SIGNAL(timeout()),this,SLOT(on_timelapse_timeout()));

	setupshuttertable();
	setupwhitebalancetable();
	setupimageformats();

	shootingraw = false;

	WIA::DeviceManager *wiaManager = new WIA::DeviceManager();

	if(wiaManager->DeviceInfos()->Count() <= 0)
	{
		nocameradetected();
	}
	else {

		cameradetected();
	}
}

WIATetherWidget::~WIATetherWidget()
{

}


void WIATetherWidget::nocameradetected()
{
		ui.gbcamera->setTitle("No camera detected!");
		ui.pbShutter->setEnabled(false);
		ui.tbISO->setEnabled(false);
		ui.tbWB->setEnabled(false);
		ui.tbFMT->setEnabled(false);
		ui.dial1->setEnabled(false);
		ui.hsbDial2->setEnabled(false);
		ui.hsExposureComp->setEnabled(false);
		ui.gbImagePreview->setEnabled(false);
		device = 0;
}
void WIATetherWidget::on_wiadlg_exception(int err, QString err1, QString err2, QString err3)
{
	qDebug(qPrintable(QString("%1").arg(err).arg(err1).arg(err2).arg(err3)));
}

void WIATetherWidget::cameradetected()
{
		if(device == 0)
		{
			WIA::CommonDialog *dlg = new WIA::CommonDialog();
			connect(dlg, SIGNAL(exception(int, QString, QString, QString)), this, SLOT(on_wiadlg_exception(int, QString, QString, QString)));
			device = dlg->ShowSelectDevice(WIA::CameraDeviceType, false, false);
		}
		if(device != 0 )
		{
				setupfnumbers();

				setupexposureindexes();

				setupexposuretimes();

				setupexposurecompensation();

				setupbatterystatus();

				setupwhitebalance();


				WIA::IProperty *prop = device->Properties()->Item(QVariant("Description"));

				ui.gbcamera->setTitle(prop->Value().toString());
				
				prop = device->Properties()->Item(QVariant("Format"));

				ui.lbFormat->setText(_imageformats.value(prop->Value().toString()));

				if(!inunsupportedmode){
				ui.pbShutter->setEnabled(true);
				ui.pbTimeLapse->setEnabled(true);
				ui.gbTimelapse->setChecked(true);
				ui.gbTimelapse->setEnabled(true);
				}
				ui.tbISO->setEnabled(true);
				ui.tbWB->setEnabled(true);
				ui.tbFMT->setEnabled(true);
				ui.dial1->setEnabled(true);
				ui.hsbDial2->setEnabled(true);
				ui.hsExposureComp->setEnabled(true);
				ui.gbImagePreview->setEnabled(true);

		}
		else
		{
			nocameradetected();
		}
}
void WIATetherWidget::setupwhitebalancetable()
{
	_whitebalances.clear();

	_whitebalances.insert(2,"Auto");
	_whitebalances.insert(4,"Daylight");
	_whitebalances.insert(5,"Flourescent");
	_whitebalances.insert(6,"Incandescent");
	_whitebalances.insert(7,"Flash");
	_whitebalances.insert(32784,"Cloudy");
	_whitebalances.insert(32785,"Shade");
	_whitebalances.insert(32786,"Kelvin");
	_whitebalances.insert(32787,"Custom");
}
void WIATetherWidget::setupwhitebalance()
{
	WIA::IProperty *prop = device->Properties()->Item(QVariant("White Balance"));
	ui.lbWhiteBalance->setText(_whitebalances.value(prop->Value().toInt()));

}


void WIATetherWidget::setupbatterystatus()
{
		WIA::IProperty *prop = device->Properties()->Item(QVariant("Battery Status"));

		ui.pbBattery->setValue(prop->Value().toDouble());
}


void WIATetherWidget::setupexposurecompensation()
{
	_camerasupportedexposurecomp.clear();
	WIA::IProperty *prop = device->Properties()->Item(QVariant("Exposure Compensation"));

	if(!prop->IsReadOnly())
	{
		int smallest = 0;
		int largest =0;
		for(int j=1; j <= prop->SubTypeValues()->Count(); j++)
		{

			QVariant value = prop->SubTypeValues()->Item(j);

			int fvalue = value.toInt();
			
			if(fvalue > 32768.)
			{
				fvalue -= 65536.;
			}

			if(fvalue < smallest)
				smallest = fvalue;

			if(fvalue > largest)
				largest = fvalue;


			_camerasupportedexposurecomp.append(fvalue);

			
		}
		int step = abs(abs(_camerasupportedexposurecomp.at(0)) - abs(_camerasupportedexposurecomp.at(1)));
		ui.hsExposureComp->setSingleStep(step);
		ui.hsExposureComp->setTickInterval(step * 2);

		ui.hsExposureComp->setMinimum(smallest);
		ui.hsExposureComp->setMaximum(largest);

		int current = prop->Value().toInt();

		if(current > 32768)
			current -= 65536.;

		ui.hsExposureComp->setValue(current);
		ui.hsExposureComp->setEnabled(true);
		ui.lbExposureComp->setText(QString("%1").arg((double)current/1000.));
	}
	else
	{
		ui.hsExposureComp->setEnabled(false);
		
	}




}
void WIATetherWidget::setupexposuretimes()
{
	   _camerasupportedexposuretimes.clear();
		WIA::IProperty *prop = device->Properties()->Item(QVariant("Exposure Time"));

		
		if(!prop->IsReadOnly()){
			hasExposureTimes = true;	
			int val = prop->Value().toInt();
			if(val == -1 && !inunsupportedmode){
				//if(!initializing)
				inunsupportedmode = true;
				QMessageBox::warning(this,"Camera In Bulb mode","Your camera is set to bulb/or some other unsupported mode, remote shutter control disabled");
				ui.pbShutter->setEnabled(false);
				ui.pbTimeLapse->setEnabled(false);
				ui.gbTimelapse->setChecked(false);
				ui.gbTimelapse->setEnabled(false);
			}
			else{
				inunsupportedmode = false;
				ui.pbShutter->setEnabled(true);
				ui.pbTimeLapse->setEnabled(true);
				ui.gbTimelapse->setChecked(true);
				ui.gbTimelapse->setEnabled(true);
			}
			ui.lbShutter->setText(_allexposuretimes.value(prop->Value().toInt()));

			if(prop->Value().toInt() >= 10000)
				ifsleepneeded = true;
			else
				ifsleepneeded = false;


			for(int j=1; j <= prop->SubTypeValues()->Count(); j++)
			{
				QVariant value = prop->SubTypeValues()->Item(j);
				_camerasupportedexposuretimes.append(value.toInt());
			}
			int exposuretimeselectedindex = _camerasupportedexposuretimes.indexOf(prop->Value().toInt());

			ui.hsbDial2->setMaximum(_camerasupportedexposuretimes.count() - 1);
			ui.hsbDial2->setValue(exposuretimeselectedindex);
		}
		else
		{
			hasExposureTimes = false;
		}

}
void WIATetherWidget::setupexposureindexes()
{
		WIA::IProperty *prop = device->Properties()->Item(QVariant("Exposure Index"));

		if(!prop->IsReadOnly())
		{
			hasISO = true;
			ui.lbISO->setText(QString("%1").arg(prop->Value().toInt()));
			
			_isonumbers.clear();
			for(int j=1; j <= prop->SubTypeValues()->Count(); j++)
			{
				QVariant value = prop->SubTypeValues()->Item(j);
				_isonumbers.append(QString("%1").arg(value.toInt()));
			}
			ui.tbISO->setDisabled(true);
		}
		else
		{
			hasISO = false;
			ui.tbISO->setDisabled(true);
		}

}
void WIATetherWidget::setupfnumbers()
{
	_fnumbers.clear();
	WIA::IProperty *prop = device->Properties()->Item(QVariant("F Number"));

	if(!prop->IsReadOnly())
	{
		hasFNumbers = true;
		ui.lbApeture->setText(QString("%1").arg(prop->Value().toFloat() / 100));

		QString selectedfnumber = QString("%1").arg(prop->Value().toFloat() / 100);
		
		_fnumbers.clear();
		for(int j=1; j <= prop->SubTypeValues()->Count(); j++)
		{
			QVariant value = prop->SubTypeValues()->Item(j);
			_fnumbers.append(QString("%1").arg(value.toFloat() / 100.));
		}
		ui.dial1->setDisabled(false);
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_fnumbers.count() - 1);
		ui.dial1->setValue(_fnumbers.indexOf(selectedfnumber));
	}
	else
	{
		hasFNumbers = false;
		ui.dial1->setDisabled(true);
	}


}
void WIATetherWidget::setupshuttertable()
{
	_allexposuretimes.insert(1,"6400");
	_allexposuretimes.insert(2,"4000");
	_allexposuretimes.insert(3,"3200");
	_allexposuretimes.insert(4,"2500");
	_allexposuretimes.insert(5,"2000");
	_allexposuretimes.insert(6,"1600");
	_allexposuretimes.insert(8,"1250");
	_allexposuretimes.insert(10,"1000");
	_allexposuretimes.insert(12,"800");
	_allexposuretimes.insert(13,"750");
	_allexposuretimes.insert(15,"640");
	_allexposuretimes.insert(20,"500");
	_allexposuretimes.insert(25,"400");
	_allexposuretimes.insert(28,"350");
	_allexposuretimes.insert(31,"320");
	_allexposuretimes.insert(40,"250");
	_allexposuretimes.insert(50,"200");
	_allexposuretimes.insert(55,"180");
	_allexposuretimes.insert(62,"160");
	_allexposuretimes.insert(80,"125");
	_allexposuretimes.insert(100,"100");
	_allexposuretimes.insert(111,"90");
	_allexposuretimes.insert(125,"80");
	_allexposuretimes.insert(166,"60");
	_allexposuretimes.insert(200,"50");
	_allexposuretimes.insert(222,"45");
	_allexposuretimes.insert(250,"40");
	_allexposuretimes.insert(333,"30");
	_allexposuretimes.insert(400,"25");
	_allexposuretimes.insert(500,"20");
	_allexposuretimes.insert(666,"15");
	_allexposuretimes.insert(769,"13");
	_allexposuretimes.insert(1000,"10");
	_allexposuretimes.insert(1250,"8");
	_allexposuretimes.insert(1666,"6");
	_allexposuretimes.insert(2000,"5");
	_allexposuretimes.insert(2500,"4");
	_allexposuretimes.insert(3333,"3");
	_allexposuretimes.insert(4000,"2.5");
	_allexposuretimes.insert(5000,"2");
	_allexposuretimes.insert(6250,"1.6");
	_allexposuretimes.insert(6666,"1.5");
	_allexposuretimes.insert(7692,"1.3");
	_allexposuretimes.insert(10000,"1\"");
	_allexposuretimes.insert(13000,"1.3\"");
	_allexposuretimes.insert(15000,"1.5\"");
	_allexposuretimes.insert(16000,"1.6\"");
	_allexposuretimes.insert(20000,"2\"");
	_allexposuretimes.insert(25000,"2.5\"");
	_allexposuretimes.insert(30000,"3\"");
	_allexposuretimes.insert(40000,"4\"");
	_allexposuretimes.insert(50000,"5\"");
	_allexposuretimes.insert(60000,"6\"");
	_allexposuretimes.insert(80000,"8\"");
	_allexposuretimes.insert(100000,"10\"");
	_allexposuretimes.insert(130000,"13\"");
	_allexposuretimes.insert(150000,"15\"");
	_allexposuretimes.insert(200000,"20\"");
	_allexposuretimes.insert(250000,"25\"");
	_allexposuretimes.insert(300000,"30\"");
	_allexposuretimes.insert(-1,"bulb");
}


void WIATetherWidget::setupimageformats()
{
	_imageformats.clear();
	//_imageformats.insert("{B96B3CAB-0728-11D3-9D7B-0000F81EF32E}","BMP");
	//_imageformats.insert("{B96B3CB0-0728-11D3-9D7B-0000F81EF32E}","GIF");
	_imageformats.insert("{B96B3CAE-0728-11D3-9D7B-0000F81EF32E}","JPG");
	//_imageformats.insert("{B96B3CAF-0728-11D3-9D7B-0000F81EF32E}","PNG");
	//_imageformats.insert("{B96B3CB1-0728-11D3-9D7B-0000F81EF32E}","TIFF");
	_imageformats.insert("{B96B3CA9-0728-11D3-9D7B-0000F81EF32E}","RAW");
}
void WIATetherWidget::on_hsExposureComp_valueChanged()
{
	int current = ui.hsExposureComp->value();

	int closestsmall;
	int closestlarge;
	for(int j=0; j <  _camerasupportedexposurecomp.count()-2; j++)
	{
		if(current > _camerasupportedexposurecomp.at(j) && current < _camerasupportedexposurecomp.at(j+1))
		{
			closestlarge = _camerasupportedexposurecomp.at(j+1);
			closestsmall = _camerasupportedexposurecomp.at(j);
			if(current - closestsmall > closestlarge - current)
				current = closestlarge;
			else
				current = closestsmall;
		}
	}
	
	ui.hsExposureComp->setValue(current);
	ui.lbExposureComp->setText(QString("%1").arg((double)current/1000.));


	if(current < 0)
		current += 65536;

	device->Properties()->Item(QVariant("Exposure Compensation"))->SetValue(QVariant(current));


	
}
void WIATetherWidget::on_dial1_valueChanged(){
	
	if(device == 0) return;
	int index = ui.dial1->value();

	if(ui.tbWB->isChecked() && hasWhiteBalance)
	{
		QString whitebal = _whitebalances.value(_whitebalances.keys().at(index));
		device->Properties()->Item(QVariant("White Balance"))->SetValue(QVariant(_whitebalances.keys().at(index)));
		ui.lbWhiteBalance->setText(_whitebalances.value(_whitebalances.keys().at(index)));

	}
	else if(ui.tbISO->isChecked() && hasISO)
	{
		QVariant value(_isonumbers.at(index));
		device->Properties()->Item(QVariant("Exposure Index"))->SetValue(value);
		ui.lbISO->setText(QString("%1").arg(value.toInt()));

	}
	else if(ui.tbFMT->isChecked())
	{
		QVariant value(_imageformats.keys().at(index));
		device->Properties()->Item(QVariant("Format"))->SetValue(value);

		if(value == wiaFormatRAW)
		{
			shootingraw = true;
		//	ui.gbImagePreview->setEnabled(false);
		//	if(ui.gbImagePreview->isChecked())
		//		ui.gbImagePreview->setChecked(false);
		}
		else
		{
			shootingraw = false;
		//	ui.gbImagePreview->setEnabled(true);
		}
		ui.lbFormat->setText(QString("%1").arg(_imageformats.value(_imageformats.keys().at(index))));

	}
	else if(hasExposureTimes)
	{
		ui.lbApeture->setText(QString("%1").arg(_fnumbers.at(index)));

		float fvalue =_fnumbers.at(index).toFloat() * 100.;
		QVariant value(QVariant(fvalue).toInt());
		device->Properties()->Item(QVariant("F Number"))->SetValue(value);

		setupexposuretimes();
	}
}

void WIATetherWidget::on_hsbDial2_valueChanged(){
	if(device == 0) return;

	int index = ui.hsbDial2->value();
	int selected = _camerasupportedexposuretimes.at(index);

	if(selected >= 10000)
		ifsleepneeded = true;
	else
		ifsleepneeded = false;

	QString selectedValue = _allexposuretimes.value(selected);
	ui.lbShutter->setText(selectedValue);

	device->Properties()->Item(QVariant("Exposure Time"))->SetValue(QVariant(selected));
}

void WIATetherWidget::on_tbISO_toggled(bool checked)
{
	if(checked)
	{
		if(ui.tbWB->isChecked())
			ui.tbWB->setChecked(false);
		if(ui.tbFMT->isChecked())
			ui.tbFMT->setChecked(false);

		WIA::IProperty *prop = device->Properties()->Item(QVariant("Exposure Index"));
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_isonumbers.count() - 1);
		ui.dial1->setValue(_isonumbers.indexOf(prop->Value().toString()));

	}
	else if(!ui.tbWB->isChecked() && !ui.tbFMT->isChecked())
	{
		WIA::IProperty *prop = device->Properties()->Item(QVariant("F Number"));
	
		QString selectedfnumber = QString("%1").arg(prop->Value().toFloat() / 100);
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_fnumbers.count() - 1);
		ui.dial1->setValue(_fnumbers.indexOf(selectedfnumber));
	}
}

void WIATetherWidget::on_tbWB_toggled(bool checked)
{
	if(checked)
	{
		if(ui.tbISO->isChecked())
			ui.tbISO->setChecked(false);
		if(ui.tbFMT->isChecked())
			ui.tbFMT->setChecked(false);

		WIA::IProperty *prop = device->Properties()->Item(QVariant("White Balance"));
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_whitebalances.count() - 1);
		ui.dial1->setValue(_whitebalances.keys().indexOf(prop->Value().toInt()));
		ui.lbWhiteBalance->setText(_whitebalances.value(prop->Value().toInt()));


	}
	else if(!ui.tbISO->isChecked() && !ui.tbFMT->isChecked())
	{
		WIA::IProperty *prop = device->Properties()->Item(QVariant("F Number"));
	
		QString selectedfnumber = QString("%1").arg(prop->Value().toFloat() / 100);
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_fnumbers.count() - 1);
		ui.dial1->setValue(_fnumbers.indexOf(selectedfnumber));
	}

}

void WIATetherWidget::on_tbFMT_toggled(bool checked)
{
	if(checked)
	{
		if(ui.tbISO->isChecked())
			ui.tbISO->setChecked(false);
		if(ui.tbWB->isChecked())
			ui.tbWB->setChecked(false);

		WIA::IProperty *prop = device->Properties()->Item(QVariant("Format"));
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_imageformats.count() - 1);
		ui.dial1->setValue(_imageformats.keys().indexOf(prop->Value().toString()));
		ui.lbFormat->setText(_imageformats.value(prop->Value().toString()));



	}
	else if(!ui.tbISO->isChecked() && !ui.tbWB->isChecked())
	{
		WIA::IProperty *prop = device->Properties()->Item(QVariant("F Number"));
	
		QString selectedfnumber = QString("%1").arg(prop->Value().toFloat() / 100);
		ui.dial1->setMinimum(0);
		ui.dial1->setMaximum(_fnumbers.count() - 1);
		ui.dial1->setValue(_fnumbers.indexOf(selectedfnumber));
	}
}

void WIATetherWidget::on_pbShutter_clicked()
{
	device->ExecuteCommand(wiaCommandTakePicture);

	if(ifsleepneeded){
		qDebug("Sleeping...");
		Sleep(30000);
	}

	if(ui.gbImagePreview->isChecked())
	{

		int lastIndex = device->Items()->Count();
		

		QFileInfo previewFile;
		
		if(shootingraw)
		{
			previewFile = QFileInfo("c:/temp/preview.nef");
		}
		else
		{
			previewFile = QFileInfo("c:/temp/preview.jpg");
		}

		if(previewFile.exists())
		{
			previewFile.absoluteDir().remove(previewFile.fileName());
		}

		WIA::IItem *item = device->Items()->Item(lastIndex);
		WIA::IImageFile *vimage;
		WIA::CommonDialog dlg;

		if(shootingraw)
			vimage = dlg.ShowTransfer(item, wiaFormatTIFF);
		else
			vimage = item->Transfer(wiaFormatJPEG);

	
		vimage->SaveFile(previewFile.absoluteFilePath());
		QPixmap preview;

		if(shootingraw)
		{
			DcRImage dcraw;

			//incase the of long shutter exposure and camera hasn't finished writing and it grabs the pic before and it turns out to be jpg
			if(dcraw.isRaw(previewFile.absoluteFilePath())){
				dcraw.load(previewFile.absoluteFilePath());

				//QByteArray *image =dcraw.GetImage(previewFile.absoluteFilePath());

				preview = QPixmap::fromImage(dcraw.getimage()); //.loadFromData(*image);
			}
			else
				preview.load(previewFile.absoluteFilePath());
		}
		else
		{
			preview.load(previewFile.absoluteFilePath());
		}

		QGraphicsScene *scene = new QGraphicsScene();
		ui.graphicsView->setScene(scene);

		scene->addPixmap(preview.scaled(ui.graphicsView->width(),ui.graphicsView->height(),Qt::KeepAspectRatio,Qt::SmoothTransformation));
		ui.graphicsView->show();

	}

}

void WIATetherWidget::on_pbsetstorage_clicked()
{
	_destinationfolder = QFileDialog::getExistingDirectory(this);
}
void WIATetherWidget::on_cbAutoOnOff_toggled(bool checked)
{

	if(checked)
		timer->start(ui.sbAutoSeconds->value() * 1000);
	else
		timer->stop();
}

void WIATetherWidget::on_timer_timeout()
{
	WIA::DeviceManager *wiaManager = new WIA::DeviceManager();

	cameradetected();

	if(device != 0 && wiaManager->DeviceInfos()->Count() > 0)
	{
		if(!_destinationfolder.isNull() && !_destinationfolder.isEmpty())
		{
			//Get pictures from camera
			int count = device->Items()->Count();

			if(last == -1) 
			{
				last = count;
			}

			for (int i=0; i < count - last; i++){
				WIA::IItem *item = device->Items()->Item(last + i);
				WIA::IImageFile *image = item->Transfer();

				QString ext = image->FileExtension();

				if(ext.isEmpty() || ext.isNull())
					ext = "raw";

				QString filename(QString("%1/%2.%3").arg(_destinationfolder).arg(item->Properties()->Item(QVariant("Item Name"))->Value().toString()).arg(ext));

				if(!QFile::exists(filename))
				{
					image->SaveFile(filename);
					emit newImageDownloaded(filename);
				}
			}
			last = count;
		}
	}
	else
	{

		if(wiaManager->DeviceInfos()->Count() <= 0)
		{
			nocameradetected();
		}
		else {

			WIA::CommonDialog *dlg = new WIA::CommonDialog();

			device = dlg->ShowSelectDevice(WIA::CameraDeviceType, false, false);
			if(device == 0)
			{
				nocameradetected();
			}
			else
			{
				cameradetected();
			}
		}

	}
}
void WIATetherWidget::on_pbTimeLapse_clicked()
{
	if(timelapsetimer->isActive())
	{
		timelapsetimer->stop();
		ui.pbTimeLapse->setText("Start Time Lapse");
	}
	else
	{
		timelapsefreq = ui.sbtimelapsefreq->value();
		timelapsedelay = ui.sbtimelapsedelay->value();
		numshotspertimeout = ui.sbtimelapseshotsper->value();
		totalnumbershots = ui.sbtimelapsetotal->value();
		secondsdelayed = 0;
		currentnumberofshots = 0;
		timelapsetimer->start(timelapsefreq * 1000);

		ui.pbTimeLapse->setText("Stop Time Lapse");
	}
}

void WIATetherWidget::on_timelapse_timeout()
{
	if(timelapsedelay <= secondsdelayed)
	{
		secondsdelayed += timelapsefreq;

		for(int i=0; i < numshotspertimeout; i++)
		{
			device->ExecuteCommand(wiaCommandTakePicture);
		}

		currentnumberofshots += numshotspertimeout;

		if(currentnumberofshots >= totalnumbershots)
		{
			timelapsetimer->stop();
			ui.pbTimeLapse->setText("Start Time Lapse");
		}
	}
	else
		secondsdelayed += timelapsefreq;

}
#include "DPM_MainWindow.h"
#include "ui_DPM_MainWindow.h"
#include <QDateTime>
#include <QFile>
#include <QMessageBox>
#include <QPainter>
#include <QRgb>
#include <QScreen>
#include <QSettings>
#include <QTextBrowser>
#include <QtMath>
#include "Windows.h"

DPM_MainWindow::DPM_MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DPM_MainWindow),
	mImagePatternSelector(new DPM_ImagePatternSelector(this)),
	mHelp(this)
{
	ui->setupUi(this);
	this->mImagePatternSelector->hide();
	
	connect(this->mImagePatternSelector,	SIGNAL(patternSelected(QPixmap)),
					this,													  SLOT(setPattern(QPixmap)));

	// move pattern matching object to separate thread
	this->mPatternMatcher.moveToThread(&this->mPatternFinderThread);
	// connect signals
	connect(this,										SIGNAL(setPatternImage(QImage)),
					&this->mPatternMatcher,	  SLOT(setPatternImage(QImage)));
	connect(&this->mPatternMatcher,	SIGNAL(compareResult(QImage)),
					this,										  SLOT(compareResult(QImage)));
	connect(&this->mPatternMatcher,	SIGNAL(took(qint64)),
					this,										  SLOT(compareTook(qint64)));
	connect(&this->mPatternMatcher,	SIGNAL(matchFound(int,int)),
					this,										  SLOT(matchFound(int,int)));
	connect(&this->mFindPatternTimer,			SIGNAL(timeout()),
					&this->mPatternMatcher,	  SLOT(grabScreenShotAndCheckPattern()));
	this->mPatternFinderThread.start();

	{ // help window
		this->mHelp.setWindowTitle("DPM Help");
		this->mHelp.setMinimumWidth(600);
		this->mHelp.setMinimumHeight(400);
		QTextBrowser *const helpBrowser = new QTextBrowser(&this->mHelp);
		QHBoxLayout *const helpLayout = new QHBoxLayout(&this->mHelp);
		helpLayout->setMargin(0);
		helpLayout->addWidget(helpBrowser);

		helpBrowser->setText("<H2>How to use Desktop Pattern Matcher (DPM)</H2>"
												 "<ol>"
												 "<li>Click \""+this->ui->setImagePatternFromScreenShotButton->text()
												 +"\" to grab screenshot and select pattern from it.<br>"
													"Zooming in and out on screenshot is available. "
													"Use <b>crop</b> mode to <b>select pattern</b>.<br>"
													"Another way is to create <i>pattern.png</i> image in <b>application directory</b> or replace existing one.</li>"
													"<li>Set desired <b>check interval</b> (how often should desktop be checked for patterns) "
													"and enable checking process with disabled \""+this->ui->clickCheck->text()+
												 "\" checkbox</li>"
												 "<li>Watch <b>load</b> in the upper right corner. Red means that it takes <b>too long</b> for pattern matching process to finish "
												 "(longer than check interval).<br>"
												 "You can either make check interval <b>longer</b> or fiddle with threshold 1 and 2. Or both.<br>"
												 "The lower the thresholds, the better the performance but fewer potential matches (yellow ones) will be selected.</li>"
												 "<li>After getting potential matches right, it's time to make green matches work (real ones).<br>"
												 "By default, exact matching is performed. If needed, tolerant matching can be enabled.<br>"
												 "Try setting as <b>low</b> threshold as possible while still matching desired patterns.</li>"
												 "<li>Enable \""+this->ui->clickCheck->text()+
												 "\" checkbox to have DPM clicked in the <b>center</b> of matched patterns (displayed as green crosses in green rectangles).</li>"
												 "</ol>"
												 "Happy matching (:<br>"
												 "<br>"
												 "contact: ka2rpa(at)gmail.com");
	}

	this->settingsLoad();
}

DPM_MainWindow::~DPM_MainWindow()
{
	delete this->mImagePatternSelector;
	this->mPatternFinderThread.quit();
	delete ui;
}

void DPM_MainWindow::setPattern(QPixmap pattern)
{
	emit this->setPatternImage(pattern.toImage());
	this->mPattern = pattern;

	// mark pattern with few colourful pixels so it never gets matched
	QImage modifiedPattern = pattern.toImage();
	for(int x = 0; x < 8; ++x){
		if(x >= modifiedPattern.width()){
			break;
		}
		switch(x){
			case 0:
				modifiedPattern.setPixelColor(x,0,Qt::red);
				break;
			case 1:
				modifiedPattern.setPixelColor(x,0,Qt::green);
				break;
			case 2:
				modifiedPattern.setPixelColor(x,0,Qt::blue);
				break;
			case 3:
				modifiedPattern.setPixelColor(x,0,Qt::white);
				break;
			case 4:
				modifiedPattern.setPixelColor(x,0,Qt::black);
				break;
			case 5:
				modifiedPattern.setPixelColor(x,0,Qt::yellow);
				break;
			case 6:
				modifiedPattern.setPixelColor(x,0,Qt::magenta);
				break;
			case 7:
				modifiedPattern.setPixelColor(x,0,Qt::cyan);
				break;
		}
	}
	this->ui->patternLabel->setPixmap(QPixmap::fromImage(modifiedPattern));
}

void DPM_MainWindow::compareResult(QImage compareResult)
{
	if(this->ui->enableCheck->isChecked()){
		this->ui->desktopCompareLabel->setPixmap(
					QPixmap::fromImage(compareResult
														 .scaled(
															 this->ui->desktopCompareLabel->width(),
															 this->ui->desktopCompareLabel->height(),
															 Qt::KeepAspectRatio,
															 Qt::SmoothTransformation)
														 )
					);
	}
}

void DPM_MainWindow::compareTook(qint64 milliseconds)
{
	if(this->ui->enableCheck->isChecked()){
		// how long is check interval
		const int checkInterval = this->mFindPatternTimer.interval();
		// show load value in percents
		this->ui->loadLabel->setText(
					QString(" %1% ").arg(100*milliseconds/checkInterval));
		if(milliseconds > checkInterval){
			// pattern matching took longer than check interval, not good
			this->ui->loadLabel->setStyleSheet("background-color: red;");
		} else {
			// ok
			this->ui->loadLabel->setStyleSheet("background-color: lime;");
		}
	}
}

void DPM_MainWindow::matchFound(int x, int y)
{
	if(this->ui->clickCheck->isChecked()){
		// moving cursor to [x,y]
		QCursor::setPos(x,y);

		// clicking left mouse button
		INPUT Inputs[2] = {0};

		Inputs[0].type = INPUT_MOUSE;
		Inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

		Inputs[1].type = INPUT_MOUSE;
		Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

		SendInput(2, Inputs, sizeof(INPUT));
	}
}

void DPM_MainWindow::settingsSave()
{
	QSettings settings("DesktopPatternMatcher.ini", QSettings::IniFormat, this);

	settings.setValue("mwGeometry", this->saveGeometry());
	settings.setValue("imagePatternSelectorGeometry", this->mImagePatternSelector->saveGeometry());
	settings.setValue("splitterState", this->ui->splitter->saveState());
	settings.setValue("checkInterval", this->ui->checkIntervalValue->value());
	settings.setValue("threshold1", this->ui->threshold1Value->value());
	settings.setValue("threshold2", this->ui->threshold2Value->value());
	settings.setValue("tolerantMatchThreshold", this->ui->tolerantThresholdValue->value());
	settings.setValue("tolerantMatch", this->ui->tolerantMatchGroup->isChecked());
	settings.setValue("clickMatches", this->ui->clickCheck->isChecked());
	if(!this->mPattern.isNull()){
		this->mPattern.save("pattern.png");
	}

}

void DPM_MainWindow::settingsLoad()
{
	QSettings settings("DesktopPatternMatcher.ini", QSettings::IniFormat, this);

	this->restoreGeometry(settings.value("mwGeometry").toByteArray());
	this->mImagePatternSelector->restoreGeometry(settings.value("imagePatternSelectorGeometry").toByteArray());
	this->ui->splitter->restoreState(settings.value("splitterState").toByteArray());
	this->ui->checkIntervalValue->setValue(settings.value("checkInterval", 0.1).toDouble());
	this->on_checkIntervalValue_valueChanged(this->ui->checkIntervalValue->value());
	this->ui->threshold1Value->setValue(settings.value("threshold1", 100).toInt());
	this->on_threshold1Value_valueChanged(this->ui->threshold1Value->value());
	this->ui->threshold2Value->setValue(settings.value("threshold2", 20).toInt());
	this->on_threshold2Value_valueChanged(this->ui->threshold2Value->value());
	this->ui->tolerantThresholdValue->setValue(settings.value("tolerantMatchThreshold", 100).toInt());
	this->on_tolerantThresholdValue_valueChanged(this->ui->tolerantThresholdValue->value());
	this->ui->tolerantMatchGroup->setChecked(settings.value("tolerantMatch", false).toBool());
	this->on_tolerantMatchGroup_toggled(this->ui->tolerantMatchGroup->isChecked());
	this->ui->clickCheck->setChecked(settings.value("clickMatches", false).toBool());

	if(QFile::exists("pattern.png")){
		QPixmap pattern;
		if(pattern.load("pattern.png")){
			this->setPattern(pattern);
		}
	}
}

void DPM_MainWindow::closeEvent(QCloseEvent*event)
{
	this->settingsSave();
	QMainWindow::closeEvent(event);
}

void DPM_MainWindow::on_setImagePatternFromScreenShotButton_clicked()
{
	// hide all app windows
	this->hide();
	this->mImagePatternSelector->hide();

	// get screenshot
	QTimer::singleShot(500, this->mImagePatternSelector,	SLOT(grabScreenshot()));
	// show back image pattern selector
	QTimer::singleShot(550, this->mImagePatternSelector,	SLOT(show()));
	// show back main window
	QTimer::singleShot(550, this,	SLOT(show()));
}


void DPM_MainWindow::on_enableCheck_toggled(bool checked)
{
	if(checked){
		// check that there is any pattern to match
		if(this->ui->patternLabel->pixmap() == Q_NULLPTR){
			QMessageBox::critical(this,
														"No pattern",
														"No image pattern available!\n"
														"Select pattern from screen-shot first");
			this->ui->enableCheck->setChecked(false);
			return;
		}

		this->ui->checkIntervalValue->setEnabled(false);
		this->ui->setImagePatternFromScreenShotButton->setEnabled(false);
		// start timer for regular scree
		QTimer::singleShot(500, &this->mFindPatternTimer,	SLOT(start()));
	} else {
		// pattern matching disabled
		this->ui->loadLabel->setText("--");
		this->ui->loadLabel->setStyleSheet("");
		this->ui->desktopCompareLabel->clear();
		this->ui->desktopCompareLabel->setText("nothing to show");
		this->mFindPatternTimer.stop();
		this->ui->checkIntervalValue->setEnabled(true);
		this->ui->setImagePatternFromScreenShotButton->setEnabled(true);
	}
}

void DPM_MainWindow::on_checkIntervalValue_valueChanged(double arg1)
{
	this->mFindPatternTimer.setInterval(arg1 * 1000);
}

void DPM_MainWindow::on_threshold1Value_valueChanged(int arg1)
{
	// pass value to pattern matching object
	this->mPatternMatcher.threshold1 = arg1;
}

void DPM_MainWindow::on_threshold2Value_valueChanged(int arg1)
{
	// pass value to pattern matching object
	this->mPatternMatcher.threshold2 = arg1;
}

void DPM_MainWindow::on_tolerantMatchGroup_toggled(bool arg1)
{
	if(arg1){
		this->ui->tolerantThresholdValue->setEnabled(true);
		// pass value to pattern matching object
		this->mPatternMatcher.tolerantMatchThreshold = this->ui->tolerantThresholdValue->value();
	} else {
		this->ui->tolerantThresholdValue->setEnabled(false);
		// disable tolerant matching by setting threshold to 0
		this->mPatternMatcher.tolerantMatchThreshold = 0;
	}
}

void DPM_MainWindow::on_tolerantThresholdValue_valueChanged(int arg1)
{
	// pass value to pattern matching object
	this->mPatternMatcher.tolerantMatchThreshold = arg1;
}

void DPM_MainWindow::on_actionHelp_triggered()
{
	this->mHelp.show();
}

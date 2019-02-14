#include "DPM_ImagePatternSelector.h"
#include "ui_DPM_ImagePatternSelector.h"
#include <QScreen>

DPM_ImagePatternSelector::DPM_ImagePatternSelector(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::DPM_ImagePatternSelector),
	mZoom(1)
{
	ui->setupUi(this);

	this->ui->graphicsView->setScene(new QGraphicsScene());
}

DPM_ImagePatternSelector::~DPM_ImagePatternSelector()
{
	delete ui;
}

void DPM_ImagePatternSelector::grabScreenshot()
{
	// take screenshot from screen
	this->ui->graphicsView->scene()->clear();
	this->mScreenShot = QGuiApplication::primaryScreen()->grabWindow(0);
	this->ui->graphicsView->scene()->addPixmap(this->mScreenShot);
}

void DPM_ImagePatternSelector::on_graphicsView_rubberBandChanged(const QRect &viewportRect,
																																 const QPointF &fromScenePoint,
																																 const QPointF &toScenePoint)
{
	if(viewportRect.x() == 0 &&
		 viewportRect.y() == 0 &&
		 viewportRect.width() == 0 &&
		 viewportRect.height() == 0){ // mouse button released
		// copy part of screenshot that was selected last (before mouse button was released)
		QPixmap selectedPattern = this->mScreenShot.copy(this->mLastRubberBandRect);
		this->hide();
		emit this->patternSelected(selectedPattern);
	}

	// save current selection rectangle
	if(fromScenePoint.x() < toScenePoint.x()){
		this->mLastRubberBandRect.setRect(fromScenePoint.x(),
																			fromScenePoint.y(),
																			viewportRect.width()/this->mZoom,
																			viewportRect.height()/this->mZoom);
	} else {
		this->mLastRubberBandRect.setRect(toScenePoint.x(),
																			toScenePoint.y(),
																			viewportRect.width()/this->mZoom,
																			viewportRect.height()/this->mZoom);
	}
}

void DPM_ImagePatternSelector::on_moveRadio_toggled(bool checked)
{
	if(checked){
		this->ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
	} else {
		this->ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
	}
}

void DPM_ImagePatternSelector::on_zoomInButton_clicked()
{
	this->ui->graphicsView->scale(1.25,1.25);
	this->mZoom *= 1.25;
	this->ui->zoomLabel->setText(QString("zoom: %1%")
															 .arg(this->mZoom*100, 0, 'f', 2));
}

void DPM_ImagePatternSelector::on_zoomOutButton_clicked()
{
	this->ui->graphicsView->scale(0.8,0.8);
	this->mZoom *= 0.8;
	this->ui->zoomLabel->setText(QString("zoom: %1%")
															 .arg(this->mZoom*100, 0, 'f', 2));
}

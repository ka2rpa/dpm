#include "DPM_ScreenShotPatternMatcher.h"
#include <QDateTime>
#include <QGuiApplication>
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QScreen>

DPM_ScreenShotPatternMatcher::DPM_ScreenShotPatternMatcher(QObject *parent) : QObject(parent),
	threshold1(1000),
	threshold2(20),
	tolerantMatchThreshold(0)
{

}

void DPM_ScreenShotPatternMatcher::setPatternImage(QImage pattern)
{
	// store pattern
	this->mFullResolutionPattern = pattern;
	// also make pattern that is 4 times smaller
	this->mQuaterResolutionPattern = pattern.scaled(pattern.width()/4,
																									pattern.height()/4,
																									Qt::KeepAspectRatio,
																									Qt::FastTransformation);
}

void DPM_ScreenShotPatternMatcher::grabScreenShotAndCheckPattern()
{
//	QMap<QString, qint64> times;
	const qint64 start = QDateTime::currentMSecsSinceEpoch();
//	qint64 timesStart = start;

	// get screenshot
	const QImage screenShot = QGuiApplication::primaryScreen()->grabWindow(0).toImage();
	QImage compareResult(screenShot);
	QPainter painter(&compareResult);
	painter.fillRect(compareResult.rect(), QColor(0,0,0,150));

//	times.insert("01 screenshot grab", QDateTime::currentMSecsSinceEpoch()-timesStart);
//	timesStart = QDateTime::currentMSecsSinceEpoch();

	// set painter for drawing potential matches
	painter.setPen(QPen(Qt::yellow, 1));
	painter.setBrush(QBrush(QColor(255,255,0,100)));

	// softly compare quater resolution screen-shot and quater resolution pattern
	const QImage quaterResolutionScreenShot = screenShot.scaled(
																							screenShot.width()/4,
																							screenShot.height()/4,
																							Qt::KeepAspectRatio,
																							Qt::FastTransformation);
	const int quaterPatternPixelCount = this->mQuaterResolutionPattern.width()*this->mQuaterResolutionPattern.width();
	QList<QPoint> approximateMatchPoints;
	for(int x = 0; x < (quaterResolutionScreenShot.width()-this->mQuaterResolutionPattern.width()); ++x){
		for(int y = 0; y < (quaterResolutionScreenShot.height()-this->mQuaterResolutionPattern.height()); ++y){
			quint64 distanceValue = 0;
			bool thresholdOneOverflow = false;
			for(int x2 = 0; x2 < this->mQuaterResolutionPattern.width(); ++x2){
				for(int y2 = 0; y2 < this->mQuaterResolutionPattern.height(); ++y2){
					const QRgb screenShotPixel = quaterResolutionScreenShot.pixel(x+x2, y+y2);
					const QRgb patternPixel =    this->mQuaterResolutionPattern.pixel(x2, y2);
					distanceValue += qAbs(qRed(screenShotPixel) - qRed(patternPixel));
					distanceValue += qAbs(qGreen(screenShotPixel) - qGreen(patternPixel));
					distanceValue += qAbs(qBlue(screenShotPixel) - qBlue(patternPixel));
					if(x2 > 3 &&
						 y2 > 3 &&
						 (distanceValue/(x2*y2)/3) > this->threshold1){
						thresholdOneOverflow = true;
						break;
					}
				}
				if(thresholdOneOverflow){
					break;
				}
			}
			if(!thresholdOneOverflow){
				// normalize
				const quint64 normalizedDistanceValue = distanceValue/quaterPatternPixelCount/3;
				if(normalizedDistanceValue < this->threshold2){
					approximateMatchPoints.append((QPoint(x*4, y*4)));
					// draw yellow half-transparent rectangle for match
					painter.drawRect(x*4,
													 y*4,
													 this->mFullResolutionPattern.width()-4,
													 this->mFullResolutionPattern.height()-4);
				}
			}
		}
	}

//	times.insert("02 1/4 resolution near match", QDateTime::currentMSecsSinceEpoch()-timesStart);
//	timesStart = QDateTime::currentMSecsSinceEpoch();

	QList<QPoint> exactMatches;
	foreach(QPoint matchpoint, approximateMatchPoints){
		// search in 10x10 neighbourhood around match points for exact match
		int minDifference = 255* // max value per color
												3* // colors
												this->mFullResolutionPattern.width()*this->mFullResolutionPattern.height(); // pixels
		int minDifferenceX = -1;
		int minDifferenceY = -1;
		bool exactMatchFound = false;
		for(int x = -10; x <= 10; ++x){
			for(int y = -10; y <= 10; ++y){
				const QImage screenShotPart = screenShot.copy(matchpoint.x()+x,
																											matchpoint.y()+y,
																											this->mFullResolutionPattern.width(),
																											this->mFullResolutionPattern.height());
				if(screenShotPart == this->mFullResolutionPattern){ // exact match
					QPoint exactMatch(matchpoint.x()+x+this->mFullResolutionPattern.width()/2,
														matchpoint.y()+y+this->mFullResolutionPattern.height()/2);
					if(!exactMatches.contains(exactMatch)){
						exactMatches.append(exactMatch);
						exactMatchFound = true;
					}
					// break both cycles
					x = 10;
					y = 10;
				} else { // exact match didn't happen, try tolerant (approximate) match if wanted by user
					if(this->tolerantMatchThreshold > 0){
						int difference = 0;
						for(int lineIndex = 0; lineIndex < screenShotPart.height(); ++lineIndex){
							const QRgb *screenShotPartLine = (QRgb*)screenShotPart.scanLine(0);
							const QRgb *patternLine =        (QRgb*)this->mFullResolutionPattern.scanLine(0);
							for(int colIndex = 0; colIndex < screenShotPart.width(); ++colIndex){
								difference += qAbs(qRed  (screenShotPartLine[colIndex]) - qRed  (patternLine[colIndex]));
								difference += qAbs(qGreen(screenShotPartLine[colIndex]) - qGreen(patternLine[colIndex]));
								difference += qAbs(qBlue (screenShotPartLine[colIndex]) - qBlue (patternLine[colIndex]));
							}
						}

						if(difference < minDifference){ // lowest difference for current approximate
							minDifference = difference;
							minDifferenceX = matchpoint.x()+x;
							minDifferenceY = matchpoint.y()+y;
						}
					}
				}
			}
		}
		if(this->tolerantMatchThreshold > 0 && // tolerant match is enabled
			 !exactMatchFound && // no exact match was found
			 minDifferenceX != -1 && // some tolerant match was found
			 minDifferenceY != -1 && // some tolerant match was found
			 (minDifference/(this->mFullResolutionPattern.width()*this->mFullResolutionPattern.height())) < this->tolerantMatchThreshold){
			QPoint tolerantMatch(minDifferenceX+this->mFullResolutionPattern.width()/2
													 , minDifferenceY+this->mFullResolutionPattern.height()/2);
			if(!exactMatches.contains(tolerantMatch)){
				exactMatches.append(tolerantMatch);
			}
		}
	}

	//	times.insert("03 full resolution exact match", QDateTime::currentMSecsSinceEpoch()-timesStart);
	//	timesStart = QDateTime::currentMSecsSinceEpoch();

	{ // draw green crosses and rectangles on matches
		painter.setPen(QPen(Qt::green, 3));
		foreach(QPoint exactMatch, exactMatches){
			painter.drawLine(exactMatch.x()-this->mFullResolutionPattern.width()/2,
											 exactMatch.y(),
											 exactMatch.x()+this->mFullResolutionPattern.width()/2,
											 exactMatch.y());
			painter.drawLine(exactMatch.x(),
											 exactMatch.y()-this->mFullResolutionPattern.height()/2,
											 exactMatch.x(),
											 exactMatch.y()+this->mFullResolutionPattern.height()/2);
			painter.drawRect(exactMatch.x()-this->mFullResolutionPattern.width()/2,
											 exactMatch.y()-this->mFullResolutionPattern.height()/2,
											 this->mFullResolutionPattern.width(),
											 this->mFullResolutionPattern.height());
			emit this->matchFound(exactMatch.x(), exactMatch.y());
		}
	}

	// send match result to be displayed in main window
	emit this->compareResult(compareResult);

//	times.insert("04 compare result display", QDateTime::currentMSecsSinceEpoch()-timesStart);

//	foreach(QString timesLabel, times.keys()){
//		qDebug("%s took %lld ms",
//					 timesLabel.toStdString().data(),
//					 times.value(timesLabel));
//	}

//	qDebug("took %lld ms", QDateTime::currentMSecsSinceEpoch()-start);
	emit this->took(QDateTime::currentMSecsSinceEpoch()-start);
}

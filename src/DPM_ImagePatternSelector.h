#ifndef DIC_IMAGEPATTERNSELECTOR_H
#define DIC_IMAGEPATTERNSELECTOR_H

#include <QMainWindow>

namespace Ui {
	class DPM_ImagePatternSelector;
}

class DPM_ImagePatternSelector : public QMainWindow
{
		Q_OBJECT

	public:
		explicit DPM_ImagePatternSelector(QWidget *parent = 0);
		~DPM_ImagePatternSelector();

	public slots:
		void grabScreenshot();

	signals:
		void patternSelected(QPixmap pattern);

	private slots:
		void on_graphicsView_rubberBandChanged(const QRect &viewportRect,
																					 const QPointF &fromScenePoint,
																					 const QPointF &toScenePoint);
		void on_moveRadio_toggled(bool checked);
		void on_zoomInButton_clicked();
		void on_zoomOutButton_clicked();

	private:
		Ui::DPM_ImagePatternSelector *ui;
		double mZoom;
		QRect mLastRubberBandRect;
		QPixmap mScreenShot;
};

#endif // DIC_IMAGEPATTERNSELECTOR_H

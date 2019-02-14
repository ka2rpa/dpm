#ifndef DIC_MAINWINDOW_H
#define DIC_MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
#include <QThread>
#include <QTimer>
#include "DPM_ImagePatternSelector.h"
#include "DPM_ScreenShotPatternMatcher.h"

namespace Ui {
	class DPM_MainWindow;
}

struct MatchPointCluster {
		// sum of all x coordinates
		int sumX;
		// sum of all y coordinates
		int sumY;
		// number of points in cluster (for computing average x and y coordinates)
		int pointCount;
};

class DPM_MainWindow : public QMainWindow
{
		Q_OBJECT

	public:
		explicit DPM_MainWindow(QWidget *parent = 0);
		~DPM_MainWindow();

	signals:
		void setPatternImage(QImage pattern);

	public slots:
		void setPattern(QPixmap pattern);
		void compareResult(QImage compareResult);
		void compareTook(qint64 milliseconds);
		void matchFound(int x, int y);

	protected:
		// QWidget interface
		void closeEvent(QCloseEvent*event);

	private slots:
		void on_setImagePatternFromScreenShotButton_clicked();
		void on_enableCheck_toggled(bool checked);
		void on_checkIntervalValue_valueChanged(double arg1);

	private:
		Ui::DPM_MainWindow *ui;
		// for selecting image pattern
		DPM_ImagePatternSelector *const mImagePatternSelector;
		// timer that fires regular pattern finding
		QTimer mFindPatternTimer;
		// pattern matching object
		DPM_ScreenShotPatternMatcher mPatternMatcher;
		// thread for pattern matching object
		QThread mPatternFinderThread;
		QPixmap mPattern;
		QDialog mHelp;

		void settingsSave();
		void settingsLoad();

	private slots:
		void on_threshold1Value_valueChanged(int arg1);
		void on_threshold2Value_valueChanged(int arg1);
		void on_tolerantMatchGroup_toggled(bool arg1);
		void on_tolerantThresholdValue_valueChanged(int arg1);
		void on_actionHelp_triggered();
};

#endif // DIC_MAINWINDOW_H

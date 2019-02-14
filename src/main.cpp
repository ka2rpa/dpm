#include "DPM_MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	DPM_MainWindow w;
	w.show();

	return a.exec();
}

#ifndef DIC_SCREENSHOTPATTERNFINDER_H
#define DIC_SCREENSHOTPATTERNFINDER_H

#include <QObject>
#include <QImage>

class DPM_ScreenShotPatternMatcher : public QObject
{
    Q_OBJECT
  public:
    explicit DPM_ScreenShotPatternMatcher(QObject *parent = nullptr);

    int threshold1;
    int threshold2;
    int tolerantMatchThreshold;

  signals:
    void compareResult(QImage compareResult);
    void matchFound(int x, int y);
    void took(qint64 milliseconds);

  public slots:
    void grabScreenShotAndCheckPattern();
    void setPatternImage(QImage pattern);

  private:
    QImage mFullResolutionPattern;
    QImage mQuaterResolutionPattern;
};

#endif // DIC_SCREENSHOTPATTERNFINDER_H

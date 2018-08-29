#ifndef HHDETEC_THREAD_H
#define HHDETEC_THREAD_H
#include <QThread>
#include <QImage>
#include <string>
#include <opencv2/ml.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class HHDetectThread: public QThread {
	Q_OBJECT
public:
	HHDetectThread(QObject *parent = 0);
	~HHDetectThread();
signals:
	void rectReady(QRect,int);

public slots:
	void handleImageSlot(const QImage& fullImg, const QImage& EventImg);
	void handleBufferSlot(const unsigned char*full, const unsigned char*event, const unsigned char*multi);

protected:
	void run();
private:
	QImage m_QFullImg;
	QImage m_QEventImg;
	bool isReady;
	cv::Mat m_cvImg1;
	cv::Mat m_cvImg2;
	cv::Mat m_cvFullPic;
	cv::Mat m_cvEventPic;
	cv::Mat m_cvMultiPic;
	cv::Mat m_cvMultiColorPic;

};
#endif



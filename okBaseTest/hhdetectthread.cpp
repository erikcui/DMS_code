#include <time.h> 
#include <stdlib.h>
#include <QDebug>
#include "hhdetectthread.h"
#include "j_npd_d.h"
#include "j_Alig_1.h"
#include <QMediaPlayer>
#include <QDateTime>
//#include <thread>

char tmp[64];
using namespace cv;
#define face_model_path "../model/face_620model"
#define f_68_model_path "../model/68.model_3"
#define pc_area_ratio 0.9  //1.5   0.548263

#define pc_00_ofst_x 0.20   //0.15
#define pc_90_ofst_x 0.8

#define pc_ofst_x(alpha) pc_00_ofst_x+(pc_90_ofst_x-pc_00_ofst_x)*(alpha/90.0)

#define pc_00_ofst_y 0.2

#define or_L_Temple 0
#define or_R_Temple 16
#define or_nose_tip 30
#define jhh_PI 3.14159265
#define PHONE_RATIO 0.3

#define PIXELS_PER_COL  768
#define PIXELS_PER_ROW  640
#define crop(a, b, c) (((a + b) > c) ? (c-b) : (a))
j_npd_d face_detector;
j_Alig_1 alig_maker;
#define sleeping_model_path "../model/sleeping.pb"
extern int DetectPhoneAndsleep(cv::Mat img1, cv::Mat img2 , cv::Mat img3,int *phone, int *sleep);
extern int DetectSleep(cv::Mat img1, int *sleep, string model_path);
extern int DetectPhone(cv::Mat img1, cv::Mat img2, int *called);
extern int compare_face(cv::Mat face_img1, cv::Mat face_img2);
void get_face_align(cv::Mat& img, cv::Mat& shape);
void wcl_face_align(cv::Mat img, cv::Rect rt, cv::Mat& shape);
int frame_count = 0;

//thread *playThread = NULL;

HHDetectThread::HHDetectThread(QObject* parent)
	: QThread(parent), isReady(false) {
	//playThread = new std::thread(play_audio,);
	//playThread->detach();
	m_cvFullPic.create(PIXELS_PER_ROW, PIXELS_PER_COL, CV_8UC3);
	m_cvEventPic.create(PIXELS_PER_ROW, PIXELS_PER_COL, CV_8UC3);
	m_cvMultiPic.create(PIXELS_PER_ROW, PIXELS_PER_COL, CV_8UC3);
	m_cvMultiColorPic.create(PIXELS_PER_ROW, PIXELS_PER_COL, CV_8UC3);

}

HHDetectThread::~HHDetectThread()
{
}

void QImage2cvMat(const QImage& image, Mat &mat)
{
	switch (image.format())
	{
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine()).copyTo(mat);
		break;
	case QImage::Format_RGB888:
		cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine()).copyTo(mat);
		cv::cvtColor(mat, mat, CV_BGR2RGB);
		break;
	case QImage::Format_Indexed8:
		cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine()).copyTo(mat);
		break;
	}
}

void MYLCROI(Mat& input, Point &point, Point& point1,Point& point2)
{
	int x1 = point.x;
	int y1 = point.y;
	point1.x = max(x1 - 180, 0);
	point2.x = min(x1 + 20, 768);
	point1.y = max(y1 - 50, 0);
	point2.y = min(y1 + 450, 640);
}

void MYRCROI(Mat& input, Point &point, Point& point1, Point& point2)
{
	int x1 = point.x;
	int y1 = point.y;
	point1.x = max(x1 - 20, 0);
	point2.x = min(x1 + 180, 768);
	point1.y = max(y1 - 50, 0);
	point2.y = min(y1 + 450, 640);
}

void HHDetectThread::handleImageSlot(const QImage& FullImage, const QImage& EventImage)
{
	m_QFullImg = FullImage.copy();
	m_QEventImg = EventImage.copy();
	QImage2cvMat(m_QFullImg, m_cvImg1);
	QImage2cvMat(m_QEventImg, m_cvImg2);
	isReady = true;
	/*if(!m_cvImg1.empty() && !m_cvImg2.empty())
	{
		imshow("cvImg1", m_cvImg1);
		imshow("cvImg2", m_cvImg2);
		waitKey(10);
	}*/
}

//void HHDetectThread::rectReady(QRect) {
//
//}

void buff2Mat(const unsigned char*buff, Mat& Pic)
{
	for (int i = 0; i < PIXELS_PER_ROW; i++)
	{
		for (int j = 0; j < PIXELS_PER_COL; j++)
		{
			Pic.at<Vec3b>(i, j)[0] = buff[i*PIXELS_PER_COL + j];
			Pic.at<Vec3b>(i, j)[1] = buff[i*PIXELS_PER_COL + j];
			Pic.at<Vec3b>(i, j)[2] = buff[i*PIXELS_PER_COL + j];
		}
	}
}

void buff2colorMat(const unsigned char*buff, Mat& Pic)
{
	int pixeValue = 0;
	for (int i = 0; i < PIXELS_PER_ROW; i++)
	{
		for (int j = 0; j < PIXELS_PER_COL; j++)
		{
			pixeValue = buff[i*PIXELS_PER_COL + j];
			if (pixeValue == 0)
			{
				Pic.at<Vec3b>(i, j)[0] = 0;
				Pic.at<Vec3b>(i, j)[1] = 0;
				Pic.at<Vec3b>(i, j)[2] = 0;
			}
			else if (pixeValue < 3)//
			{
				Pic.at<Vec3b>(i, j)[0] = 255;
				Pic.at<Vec3b>(i, j)[1] = 0;
				Pic.at<Vec3b>(i, j)[2] = 0;
			}
			else if (pixeValue < 5)//
			{
				Pic.at<Vec3b>(i, j)[0] = 255;
				Pic.at<Vec3b>(i, j)[1] = 255;
				Pic.at<Vec3b>(i, j)[2] = 0;
			}
			else if (pixeValue < 7)//
			{
				Pic.at<Vec3b>(i, j)[0] = 0;
				Pic.at<Vec3b>(i, j)[1] = 255;
				Pic.at<Vec3b>(i, j)[2] = 0;
			}
			else if (pixeValue < 9)//
			{
				Pic.at<Vec3b>(i, j)[0] = 0;
				Pic.at<Vec3b>(i, j)[1] = 255;
				Pic.at<Vec3b>(i, j)[2] = 255;
			}
			else
			{
				Pic.at<Vec3b>(i, j)[0] = 0;
				Pic.at<Vec3b>(i, j)[1] = 0;
				Pic.at<Vec3b>(i, j)[2] = 255;
			}

		}
	}
}

void HHDetectThread::handleBufferSlot(const unsigned char*full, const unsigned char*event, const unsigned char*multi)
{
	if (full != NULL)
		buff2Mat(full, m_cvFullPic);
	if (event != NULL)
		buff2Mat(event, m_cvEventPic);
	if (multi != NULL)
		buff2Mat(multi, m_cvMultiPic);
	if (multi != NULL)
		buff2colorMat(multi, m_cvMultiColorPic);
	isReady = true;
}

bool loadModel(string face_model, string f_68_model)
{
	bool ld_res = face_detector.Load(face_model_path);
	if (!ld_res)
	{
		return false;
	}
	ld_res = alig_maker.load(f_68_model_path);
	if (!ld_res)
	{
		return false;
	}
	return true;
}

Rect Max_Area_Rect(vector<Rect>& rts)
{
	int Max_Area = 0;
	Rect rt;
	if (rts.size() <= 0)
	{
		rt.x = -1;
		rt.y = -1;
		rt.width = -1;
		rt.height = -1;
		return rt;
	}
	for (int i = 0; i < rts.size(); i++)
	{
		if (rts[i].width*rts[i].height >= Max_Area)
		{
			rt = rts[i];
			Max_Area = rts[i].width*rts[i].height;
		}
	}

	return rt;
}

void face_angle(Mat shape, float& alpha, float& belta)
{
	if (shape.rows != 68)
	{
		printf("face_angle error\n");
		return;
	}
	double R_Temple_x = shape.at<double>(or_R_Temple, 0);
	double L_Temple_x = shape.at<double>(or_L_Temple, 0);
	double r = (R_Temple_x - L_Temple_x) / 2.0;
	r = r*1.1;
	double center = (R_Temple_x + L_Temple_x) / 2.0;
	double nose_point = shape.at<double>(or_nose_tip, 0);
	double d2b = nose_point - center;
	if (d2b > r) d2b = r;
	if (d2b < -r) d2b = -r;
	//double d_b = (shape.at<double>(or_nose_tip, 0) - c) / r;
	double d_b = d2b / r;
	d_b = d_b > 1 ? 1 : d_b;
	d_b = d_b < -1 ? -1 : d_b;
	alpha = asin(d_b);
	alpha = alpha*(180 / jhh_PI);
	//
	belta = 0;
}

float shape_area(Mat shape)
{
	float area;
	float max_x = shape.at<double>(0, 0);
	float min_x = shape.at<double>(0, 0);
	float max_y = shape.at<double>(0, 1);
	float min_y = shape.at<double>(0, 1);
	for (int i = 1; i < shape.rows; i++)
	{
		if (max_x < shape.at<double>(i, 0))
		{
			max_x = shape.at<double>(i, 0);
		}
		if (min_x > shape.at<double>(i, 0))
		{
			min_x = shape.at<double>(i, 0);
		}
		if (max_y < shape.at<double>(i, 1))
		{
			max_y = shape.at<double>(i, 1);
		}
		if (min_y > shape.at<double>(i, 1))
		{
			min_y = shape.at<double>(i, 1);
		}
	}
	area = (max_x - min_x)*(max_y - min_y);
	return area;
}

void drow_facealignment_point(Mat &shape, Mat &img)
{
	Mat image;
	img.copyTo(image);
	IplImage tmppp = IplImage(image);
	CvArr *image_test = &tmppp;
	for (int i = 0; i < 68; i++) 
	{
		CvPoint centerpoint;
		centerpoint.x = int(shape.at<double>(i, 0));
		centerpoint.y = int(shape.at<double>(i, 1));
		cvCircle(image_test, centerpoint, 3, CV_RGB(0, 255, 0));
		/*if (i == 36 || i == 39 || i == 42 || i == 45)
		{
			cvCircle(image_test, centerpoint, 3, CV_RGB(0, 255, 0));
		}*/
	}
	image = cvarrToMat(image_test);
	QTime time = QTime::currentTime();
	char str_0[64] = { 0 };
	sprintf_s(str_0, "..\\data\\align\\align_%05d.png", time);
	cv::imwrite(str_0, image);
}

int calCallDirection(Mat &multiImg, Rect &patch,float& upEV, float& downEV)
{
	Mat gray;
	cv::cvtColor(multiImg, gray, CV_BGR2GRAY);
	float upArea = 0;
	float downArea = 0;
	float callRatio = 0;
	int halfHeight = 0;
	int UpSum = 0;
	int DownSum = 0;
	int pixelValue = 0;
	halfHeight = patch.height / 2;
	for (int j = patch.x; j < patch.x + patch.width; j++)
	{
		for (int i = patch.y; i < patch.y + patch.height; i++)
		{
			pixelValue = gray.at<uchar>(i, j);
			if ((i < patch.y + halfHeight) && (pixelValue > 0))
			{
				upArea += 1;
				UpSum += pixelValue;
			}
			else if (pixelValue > 0)
			{
				downArea += 1;
				DownSum += pixelValue;
			}
		}

	}
	upEV = UpSum / upArea;
	downEV = DownSum / downArea;
	cout <<"EV "<< upEV <<" "<< downEV << endl;
	if (upEV > downEV)
	{
		//cout << "UP UP" << upEV <<" "<< downEV << endl;
		return 1;
	}
	else
	{
		//cout << "Down Down " << upEV << " " << downEV << endl;
		return 0;
	}
}

bool IsShakeAccordingToLFRatio(Mat &multiImg, Rect &patch, float& LeftEV, float& RightEV,bool isLeftOrRight)
{
	Mat gray;
	cv::cvtColor(multiImg, gray, CV_BGR2GRAY);
	float leftArea = 0;
	float rightArea = 0;
	float callRatio = 0;
	int halfwidth = 0;
	int leftSum = 0;
	int rightSum = 0;
	int pixelValue = 0;
	halfwidth = patch.width / 2;
	int maxValue = 0;
#pragma omp parallel for
	for (int i = patch.y; i < patch.y + patch.height; i++)
	{
		for (int j = patch.x; j < patch.x + patch.width; j++)
		{
			pixelValue = gray.at<uchar>(i, j);
			if ((j < patch.x + halfwidth) && (pixelValue > 0))
			{
				leftArea += 1;
				leftSum += pixelValue;
			}
			else if (pixelValue > 0)
			{
				rightArea += 1;
				rightSum += pixelValue;
			}
			if (pixelValue > maxValue)
			{
				maxValue = pixelValue;
			}
		}
	}
	LeftEV = leftSum / leftArea;
	RightEV = rightSum / rightArea;
	cout << "maxValue " << maxValue << "LREV " << LeftEV << " " << RightEV << endl;
	// isLeftOrRight== true 判断左边；isLeftOrRight== false 判断右边
	if (isLeftOrRight)
	{ 
		return (LeftEV > RightEV * 1.05) ? true:false;
	}
	else
	{
		return (RightEV > LeftEV * 1.05) ? true : false;
	}

}

float calCallRatio(Mat &EventImg, Rect &patch)
{
	Mat gray;
	cv::cvtColor(EventImg, gray, CV_BGR2GRAY);
	float area = 0;
	float callRatio = 0;	
	for (int i = patch.y; i < patch.y + patch.height; i++)
	{
		for (int j = patch.x; j < patch.x + patch.width; j++)
		{
			if (gray.at<uchar>(i, j) > 0)
			{
				area += 1;
			}	
		}
	}
	callRatio = area / patch.area();
	return callRatio;
}

void play_audio(const QString& file, int time) {
	QMediaPlayer qtaudioPlayer;
	qtaudioPlayer.setMedia(QUrl::fromLocalFile(file));
	qtaudioPlayer.setVolume(100);
	qtaudioPlayer.play();
	QThread::msleep(time);//5000 
}

void getPCPatch(Mat& shape, Rect&patch_l, Rect &patch_r, Rect&patch_eye, float&angle)
{
	float alpha = 0, belta = 0;
	face_angle(shape, alpha, belta);
	angle = alpha;
	float fc_area = shape_area(shape);
	float pc_area_l;
	float pc_area_r;
	Point L_point;
	Point R_point;
	patch_eye.x = L_point.x = shape.at<double>(0, 0);//left ear x
	L_point.y = shape.at<double>(0, 1);//left ear y
	//R_point.x = shape.at<double>(15, 0);
	//R_point.y = shape.at<double>(15, 1);
	patch_eye.y = (shape.at<double>(36, 1) + shape.at<double>(42, 1)) / 2;
	R_point.x = shape.at<double>(16, 0);
	R_point.y = shape.at<double>(16, 1);
	float crop_eye_var = 10;
	float eye_y_ratio = 0.5;
	if (alpha < 0)
	{
		float angle_cot = alpha / 90.0;
		//if (angle_cot <= 0.5) angle_cot = 0.5;
		//if (angle_cot >= -0.2) angle_cot = -0.2;
		//float a = fc_area*pc_area_ratio;
		pc_area_l = fc_area*pc_area_ratio*(1.0 + (0)); // pc_area_ratio == 1.5
		pc_area_r = fc_area*pc_area_ratio*(1.0 - (angle_cot));
		patch_l.width = float(sqrt(pc_area_l ));
		patch_l.height = patch_l.width ;
		patch_r.width = float(sqrt(pc_area_r));
		patch_r.height = patch_r.width ;
		patch_l.y = float(L_point.y) - float(patch_l.height)*pc_00_ofst_y;
		patch_r.y = float(R_point.y) - float(patch_r.height)*pc_00_ofst_y;
		patch_l.x = float(L_point.x) - float(patch_l.width)*(1.0 - pc_00_ofst_x);
		patch_r.x = float(R_point.x) - float(patch_r.width)*(pc_ofst_x(-alpha));

		patch_eye.width = (R_point.x - L_point.x) -(crop_eye_var * (1.1 - angle_cot));
		patch_eye.height = float(patch_eye.width) / 2.0;
		//patch_eye.x = patch_eye.x +(crop_eye_var * (0.5 + angle_cot));
		patch_eye.y -= patch_eye.height * eye_y_ratio;
	}
	else
	{
		float angle_cot = alpha / 90.0;
		//if (angle_cot <= 0.5) angle_cot = 0.5;
		//if (angle_cot >= -0.5) angle_cot = -0.5;
		pc_area_l = fc_area*pc_area_ratio*(1.0 + (angle_cot));
		pc_area_r = fc_area*pc_area_ratio*(1.0 - (0));
		patch_l.width = float(sqrt(pc_area_l));
		patch_l.height = patch_l.width;
		patch_r.width = float(sqrt(pc_area_r));
		patch_r.height = patch_r.width;
		patch_l.y = float(L_point.y) - float(patch_l.height)*pc_00_ofst_y;
		patch_r.y = float(R_point.y) - float(patch_r.height)*pc_00_ofst_y;  
		patch_l.x = float(L_point.x) - float(patch_l.width)*(1.0 - pc_ofst_x(-alpha));
		patch_r.x = float(R_point.x) - float(patch_r.width)*pc_00_ofst_x;

		patch_eye.width = (R_point.x - L_point.x);// -(crop_eye_var * (0.5 - angle_cot));
		patch_eye.height = float(patch_eye.width) / 2.0;
		patch_eye.x += (crop_eye_var * (1.1 + angle_cot));
		patch_eye.y -= patch_eye.height * eye_y_ratio;
	}
	//printf("hello world");
	patch_l.y = max(patch_l.y, 0);
	patch_l.x = max(patch_l.x, 0);

	patch_l.width = crop(patch_l.width, patch_l.x, 768);
	patch_l.height = crop(patch_l.height, patch_l.y, 640);

	patch_r.x = crop(patch_r.x, patch_r.width, 768);
	patch_r.y = crop(patch_r.y, patch_r.height, 640);

	patch_eye.x = crop(patch_eye.x, patch_eye.width, 768);
	patch_eye.y = crop(patch_eye.y, patch_eye.height, 640);

	patch_eye.y = max(patch_eye.y, 0);
	patch_eye.x = max(patch_eye.x, 0);
}

void facedetect(Mat& fullImg, Mat& EventImg, Mat& MultiImg,Mat& MultiColorPic,int *flag, QRect *rect)
{
	Mat gray;
	//Mat test_img = cv::imread("D:\\DMS_reserch\\PRNet\\TestImages\\0.jpg");
	cv::cvtColor(fullImg, gray, CV_BGR2GRAY);
	//int nl = test_img.rows;
	//int nc = test_img.cols;
	//for (int i = 0; i < nl; i++) {
	//	uchar * data = test_img.ptr<uchar>(i);
	//	float t;
	//	for (int j = 0; j < nc; j++) {
	//		t = test_img.at<uchar>(i, j);
	//		float a = t;
	//		float b = a / 255.0;
	//		test_img.at<uchar>(i, j) = b;

	//		uchar d = test_img.at<uchar>(i, j);
	//		//float a =data[j];
	//		
	//		data[j] = b ;
	//		float c = data[j];
	//		cout << data[j] << endl;
	//	}
	//}
	//cv::imshow("sdfdsf", test_img);
	//cv::waitKey(0)
	//cv::cvtColor(test_img, gray, CV_BGR2GRAY);
	//功能一、检测人脸
	vector<Rect> rts;
	vector<float> srs;
	QTime startTime = QTime::currentTime();
	face_detector.DetectFace(gray, rts, srs);
	qDebug() << "detection face cost time" << startTime.msecsTo(QTime::currentTime());
	Rect maxReat = Max_Area_Rect(rts);
	if (maxReat.empty())
	{
		*flag = -2;
		//QRect qRect(0, 0, 0, 0);
		//*rect = qRect;
		return;
	}
	Mat face;
	gray(cv::Rect(maxReat.x, maxReat.y, maxReat.width, maxReat.height)).copyTo(face);
	cv::cvtColor(face, face, CV_GRAY2RGB);
	Mat face_soul = cv::imread("../model/model.png");
	if (face_soul.dims == 0) {
		cv::imwrite("../model/model.png", face);
		return;
	}
	//cv::cvtColor(face_soul, face_soul, CV_BGR2RGB);
	cv::resize(face_soul, face_soul, cv::Size(160, 160));
	cv::resize(face, face, cv::Size(160, 160));
	QTime secondTime = QTime::currentTime();
	int same = compare_face(face_soul, face);
	qDebug() << "compare face cost time" << secondTime.msecsTo(QTime::currentTime());
	if (same == 1) {
		*flag = 1; //1 means first person wcl
		QRect qRect(maxReat.x, maxReat.y, maxReat.width, maxReat.height);
		*rect = qRect;
		return;
	}
	if (0 == 1) {
		Mat shape;
		//wcl_face_align(gray, maxReat, shape);
		alig_maker.jhh_Face_D_A(gray, maxReat, shape);
		//drow_facealignment_point(shape, gray);
		cv::Rect LPRect;
		cv::Rect RPRect;
		cv::Rect EyeRect;
		float angle;
		getPCPatch(shape, LPRect, RPRect, EyeRect, angle);//LPRect 有错
		printf("angleOfFace:%f", angle);
		if ((angle < -60 && angle != -90) || angle > 10) //&& angle > -80
		{
			play_audio("../wma/alarm.wma", 5000);
			return;
		}
		Mat LPROI;
		Mat RPROI;
		Mat EYE;
		Mat mark_img;
		cv::cvtColor(gray, gray, CV_GRAY2BGR);
		gray(cv::Rect(LPRect.x, LPRect.y, LPRect.width, LPRect.height)).copyTo(LPROI);
		gray(cv::Rect(EyeRect.x, EyeRect.y, EyeRect.width, EyeRect.height)).copyTo(EYE);
		gray(cv::Rect(RPRect.x, RPRect.y, RPRect.width, RPRect.height)).copyTo(RPROI);
		cv::resize(EYE, EYE, cv::Size(112, 112));
		cv::resize(RPROI, RPROI, cv::Size(112, 112));
		cv::resize(LPROI, LPROI, cv::Size(112, 112));
		int PStatus = 0;
		int SStatus = 0;
		int mode_flag = -1;// 0:sleeping, 1:phonecall 2:all
		QTime timer = QTime::currentTime();
		char str_l[64] = { 0 };
		if (mode_flag == 0) {
			DetectSleep(EYE, &SStatus, sleeping_model_path);
			if (SStatus == 1)
			{
				cout << "do not sleep" << endl;
				//play_audio("../wma/sleeping.wma", 5000);
				sprintf_s(str_l, "..\\data\\test\\sleep\\sleep_%05d.png", timer);
				cv::imwrite(str_l, EYE);
				*flag = 1;
				return;
			}
			mode_flag = 1;
		}
		if (mode_flag == 1) {
			DetectPhone(LPROI, RPROI, &PStatus);
		}
		if (mode_flag == 2) {
			DetectPhoneAndsleep(LPROI, RPROI, EYE, &PStatus, &SStatus);
		}
		//cv::imwrite(str_l, fullImg);
		if (PStatus == 1)
		{
			cout << "do not call" << endl;
			//play_audio("../wma/phone.wma", 5000);
			sprintf_s(str_l, "..\\data\\test\\phone\\left_%05d.png", timer);
			cv::imwrite(str_l, LPROI);
			*flag = 0;
			return;
		}
		if (PStatus == 2)
		{
			cout << "do not call" << endl;
			//play_audio("../wma/phone.wma", 5000);
			sprintf_s(str_l, "..\\data\\test\\phone\\right_%05d.png", timer);
			cv::imwrite(str_l, RPROI);
			*flag = 0;
			return;
		}
		}
	}
#include <corecrt_io.h>
void getFile(string path, vector<string>& files, vector<string>& file_n)
{
	intptr_t   hFile = 0;
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib &  _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFile(p.assign(path).append("\\").append(fileinfo.name), files, file_n);
			}
			else
			{
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
				file_n.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void getFiles(QString Path)
{
	vector<string> tr_pos_fs;
	vector<string> tr_pos_fn;
	getFile("D:\\target\\o", tr_pos_fs, tr_pos_fn);
	char cstr_1[1024];
	int count = 0;
#pragma omp parallel for
	for (int i = 0; i < tr_pos_fs.size(); i++)
	{
		Mat img = cv::imread(tr_pos_fs[i]);
		//cv::rectangle(img, Rect(240, 160, 230, 310), Scalar(255, 0, 0), 2);//标记人脸框
		cv::rectangle(img, Rect(420, 180, 160, 320), Scalar(255, 0, 0), 2);//标记人脸框
		cv::imshow("img", img);
		sprintf_s(cstr_1, "release\\data\\o\\%s", tr_pos_fn[i]);
		cv::imwrite(cstr_1, img);
		cv::waitKey(10);
	}
}

void alarm(int index) {
	if (index == 0) {
		play_audio("../wma/phone.wma", 5000);
	}
	if (index == 1) {
		play_audio("../wma/sleeping.wma", 5000);
	}
	if (index == 2) {
		play_audio("../wma/alarm.wma", 5000);
	}
}

void wcl_face_align(Mat img, Rect rt, Mat& shape) {
	rt.x, rt.y, rt.width, rt.height;
	float old_size = (rt.width + rt.height) / 2;
	float center_x = rt.x + rt.width / 2;
	float center_y = rt.y + rt.height / 2 + old_size * 0.14;
	float size = int(old_size * 1.58);
	int x1 = center_x - size / 2;
	int x2 = (center_x + size / 2);
	int y1 = center_y - size / 2;
	int y2 = center_y + size / 2;
	Mat crop_img;
	img(cv::Rect(x1, y1, x2-x1, y2-y1)).copyTo(crop_img);
	cv::resize(crop_img,crop_img, cv::Size(256,256));
	/*cv::imshow("asdf", crop_img);
	cv::waitKey(0);*/
	get_face_align(crop_img, shape);
}

void HHDetectThread::run()
{
#if 1
	//加载模型
	if (!loadModel(face_model_path, f_68_model_path))
	{
		qDebug() << "load model failed";
		return;
	}
	qDebug() << "qDebug load model successed";
	/*strftime(tmp, sizeof(tmp), "%Y_%m_%d_%H_%M_%S", localtime(&t));

	sprintf_s(cstr_1, "mkdir ..\\data\\%s", tmp);
	system(tmp);
	sprintf_s(cstr_1, "mkdir ..\\data\\%s\\org", tmp);
	system(cstr_1);
	sprintf_s(cstr_1, "mkdir ..\\data\\%s\\pcr", tmp);
	system(cstr_1);*/
	//int alarm_counter[] = {0,0,0}; //phone sleep noface 
	//int threshold[] = {1,1,20};
	int flag;
	//std::string * dete_name;
    forever
    {
        if (isReady)
        {
			QTime startTime = QTime::currentTime();
			QRect face_rect(0, 0, 0, 0);
			flag = -1;
            facedetect(m_cvFullPic, m_cvEventPic, m_cvMultiPic, m_cvMultiColorPic, &flag, &face_rect);
			rectReady(face_rect,flag); 
			/*if (flag != -1) {
				alarm_counter[flag] ++;
				for (int i = 0; i < 3; i++) {
					if (alarm_counter[i] >= threshold[i]) {
						alarm(i);
						alarm_counter[0] = 0;
						alarm_counter[1] = 0;
						alarm_counter[2] = 0;
					}
				}
			}
			else {
				alarm_counter[0] = 0;
				alarm_counter[1] = 0;
				alarm_counter[2] = 0;
			}*/
			qDebug() << "Run detection cost time" << startTime.msecsTo(QTime::currentTime());
            isReady = false;
        }
    }
#endif
}
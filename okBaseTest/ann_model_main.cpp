#include <QDebug>
//#include "ftime.h"
#include <opencv2/highgui/highgui.hpp>
 #include <opencv2/objdetect/objdetect.hpp>
 #include <opencv2/core/core.hpp>
 #include <opencv2/imgproc/imgproc.hpp>


#define COMPILER_MSVC
#define NOMINMAX

#include <iostream>
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"
#include "ann_model_loader.h"
#include <stdlib.h>
#include <QDateTime>
using namespace tensorflow;
//extern int gettimeofday(struct timeval *tp, void *tzp);
extern char debug_filename[128];
int DetectPhoneAndsleep(cv::Mat img1, cv::Mat img2, cv::Mat img3, int *called, int *sleep) {

    std::string input_tensor_name = "inputs";
    std::string output_tensor_name = "output_node";
	std::string model_path = "../model/train_4class_model.pb";
	static Session* session;
	Status status = NewSession(SessionOptions(), &session);
	static tf_model::ANNModelLoader model;
	if (0 != model.load(session, model_path)) {
		qDebug() << "Error: Model Loading failed...";
		return -1;
	}
    // New Feature Adapter to convert vector to tensors dictionary
    tf_model::ANNFeatureAdapter input_feat;

    input_feat.assign_3(input_tensor_name, img1, img2, img3);   //Assign vec<double> to tensor

	//input_feat.assign(input_tensor_name, img2);
	//qDebug()<<"step 3 !!!\n";
    // Make New Prediction
    double pred_phone = 0.0;
	double pred_sleep = 0.0;
    /*if (0 != model.predict(session, input_feat, output_tensor_name, &prediction)) {
        qDebug() << "WARNING: Prediction failed..." << prediction;
    }*/

	if (0 != model.predict(session, input_feat, input_tensor_name, output_tensor_name, &pred_phone, &pred_sleep)) {
		//qDebug() << "WARNING: Prediction failed..." << prediction;
	}

	qDebug()<<"done!!!";
	*called = pred_phone;
	*sleep = pred_sleep;
	/*if(prediction == 1.0){
		*called = 1;
	}
	else if(prediction == 2.0){
		*sleep = 0;
	}*/
	return 0;
}

int DetectSleep(cv::Mat img1,int *sleep,std::string model_path ) {
	std::string input_tensor_name = "inputs";
	std::string output_tensor_name = "output_node";
	//std::string model_path = "../model/sleeping.pb";
	static Session* session;
	Status status = NewSession(SessionOptions(), &session);
	static tf_model::ANNModelLoader model;
	if (0 != model.load(session, model_path)) {
		qDebug() << "Error: Model Loading failed...";
		return -1;
	}
	tf_model::ANNFeatureAdapter input_feat;
	input_feat.assign_1(input_tensor_name, img1);   //Assign vec<double> to tensor
	double pred_sleep = 0.0;

	if (0 != model.predict_one(session, input_feat, input_tensor_name, output_tensor_name, &pred_sleep)) {
		qDebug() << "WARNING: Prediction failed..."  ;
	}
	*sleep = pred_sleep;
	return 0;
}

int DetectPhone(cv::Mat img1, cv::Mat img2, int *called) {
	std::string input_tensor_name = "inputs";
	std::string output_tensor_name = "output_node";
	std::string model_path = "../model/phonecall.pb";
	static Session* session;
	Status status = NewSession(SessionOptions(), &session);
	static tf_model::ANNModelLoader model;
	if (0 != model.load(session, model_path)) {
		qDebug() << "Error: Model Loading failed...";
		return -1;
	}

	tf_model::ANNFeatureAdapter input_feat;
	input_feat.assign_2(input_tensor_name, img1,img2);   //Assign vec<double> to tensor
	double pred_call = 0.0;

	if (0 != model.predict_one(session, input_feat, input_tensor_name, output_tensor_name, &pred_call)) {
		//qDebug() << "WARNING: Prediction failed..." << prediction;
	}

	qDebug() << "done!!!"<<pred_call;
	//*called = pred_phone;
	*called = pred_call;
	return 0;
}

void get_face_align(cv::Mat& img, cv::Mat& shape) {
	std::string input_tensor_name = "inputs";
	std::string output_tensor_name = "resfcn256/output";
	std::string model_path = "../model/new_model.pb";
	static Session* session;
	Status status = NewSession(SessionOptions(), &session);
	static tf_model::ANNModelLoader model;
	if (0 != model.load(session, model_path)) {
		qDebug() << "Error: Model Loading failed...";
		return ;
	}
	tf_model::ANNFeatureAdapter input_feat;
	input_feat.assign_1(input_tensor_name, img);   //Assign vec<double> to tensor
	double pred_call = 0.0;

	if (0 != model.predict_one(session, input_feat, input_tensor_name, output_tensor_name, &pred_call)) {
		//qDebug() << "WARNING: Prediction failed..." << prediction;
	}

	qDebug() << "done!!!" << pred_call;
	//*called = pred_phone;
	//*called = pred_call;
	return ;
}

int compare_face(cv::Mat face_img1, cv::Mat face_img2) {
	std::string input_tensor_name = "input:0";
	std::string output_tensor_name = "embeddings:0";
	std::string phase_placeholder_name = "phase_train:0";
	std::string model_path = "../model/20180408-102900.pb";
	static Session* session;
	Status status = NewSession(SessionOptions(), &session);
	static tf_model::ANNModelLoader model;
	if (0 != model.load(session, model_path)) {
		//qDebug() << "Error: Model Loading failed...";
		printf("Error: Model Loading failed...");
		return -1;
	}
	tf_model::ANNFeatureAdapter input_feat;
	input_feat.assign_2(input_tensor_name, face_img1, face_img2);   //Assign vec<double> to tensor
																	//double pred_sleep = 0.0;
	float same = model.predict_face(session, input_feat, input_tensor_name, output_tensor_name, phase_placeholder_name);
	if (same == -1) {
		printf("Error: detect failed...");
		return -1;
	}
	printf("same%fsame%fsame%fsame%f",same,same,same,same);
	if (same < 0.2) {
		return 1;
	}
	return 0;
}
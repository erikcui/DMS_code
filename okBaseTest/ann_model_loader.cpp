#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>
#include <QString>
#define COMPILER_MSVC
#define NOMINMAX
#include <iostream>
#include <vector>
#include <map>
#include "ann_model_loader.h"
#include <QDateTime>
//#include <tensor_shape.h>
char cstr_debugname[1024];

char debug_filename[128]={0};

using namespace tensorflow;
namespace tf_model {

	/**
	 * ANNFeatureAdapter Implementation
	 * */
	ANNFeatureAdapter::ANNFeatureAdapter() {

	}

	ANNFeatureAdapter::~ANNFeatureAdapter() {

	}
	/*
	 * @brief: Feature Adapter: convert 1-D double vector to Tensor, shape [1, ndim]
	 * @param: std::string tname, tensor name;
	 * @parma: std::vector<double>*, input vector;
	 * */

	void ANNFeatureAdapter::assign_1(std::string tname, cv::Mat img1) {
		int size = img1.cols;
		Tensor inputImg(DT_FLOAT, TensorShape({ 1,size,size,3 }));
		float *p = inputImg.flat<float>().data();
		cv::Mat cameraImg(size, size, CV_32FC3, p);
		img1.convertTo(cameraImg, CV_32FC3);
		/*if (img1.cols != size || img1.rows != size)
		{
			cv::Mat img11;
			cv::resize(img1, img11, cv::Size(224, 224));
			img11.convertTo(cameraImg, CV_32FC3);
		}
		else
		{
			img1.convertTo(cameraImg, CV_32FC3);
		}*/

		input_new.push_back((tensorflow::Tensor)inputImg);
	}

	void ANNFeatureAdapter::assign_2(std::string tname, cv::Mat img1, cv::Mat img2) {
		int size = img1.cols;
		Tensor inputImg(DT_FLOAT, TensorShape({ 1,size,size,3 }));
		float *p = inputImg.flat<float>().data();
		cv::Mat cameraImg(size, size, CV_32FC3, p);
		img1.convertTo(cameraImg, CV_32FC3);
		//if (img1.cols != 224 || img1.rows != 224)
		//{
		//	cv::Mat img11;
		//	//cv::resize(img, img2, cv::Size(100, 250));
		//	cv::resize(img1, img11, cv::Size(224, 224));
		//	img11.convertTo(cameraImg, CV_32FC3);
		//}
		//else
		//{
		//	img1.convertTo(cameraImg, CV_32FC3);
		//}

		Tensor inputImg1(DT_FLOAT, TensorShape({ 1,size,size,3 }));
		float *p1 = inputImg1.flat<float>().data();
		cv::Mat cameraImg1(size, size, CV_32FC3, p1);
		img2.convertTo(cameraImg1, CV_32FC3);
		/*if (img2.cols != 224 || img2.rows != 224)
		{
			cv::Mat img22;
			cv::resize(img2, img22, cv::Size(224, 224));
			img22.convertTo(cameraImg1, CV_32FC3);
		}
		else
		{
			img2.convertTo(cameraImg1, CV_32FC3);
		}*/

		input_new.push_back((tensorflow::Tensor)inputImg);
		input_new.push_back((tensorflow::Tensor)inputImg1);
	}

	void ANNFeatureAdapter::assign_3(std::string tname, cv::Mat img1, cv::Mat img2, cv::Mat img3) {
		Tensor inputImg(DT_FLOAT, TensorShape({ 1,224,224,3 }));
		float *p = inputImg.flat<float>().data();
		cv::Mat cameraImg(224, 224, CV_32FC3, p);
		if (img1.cols != 224 || img1.rows != 224)
		{
			cv::Mat img11;
			//cv::resize(img, img2, cv::Size(100, 250));
			cv::resize(img1, img11, cv::Size(224, 224));
			img11.convertTo(cameraImg, CV_32FC3);
		}
		else
		{
			img1.convertTo(cameraImg, CV_32FC3);
		}

		Tensor inputImg1(DT_FLOAT, TensorShape({ 1,224,224,3 }));
		// get pointer to memory for that Tensor
		float *p1 = inputImg1.flat<float>().data();
		cv::Mat cameraImg1(224, 224, CV_32FC3, p1);

		if (img2.cols != 224 || img2.rows != 224)
		{
			cv::Mat img22;
			cv::resize(img2, img22, cv::Size(224, 224));
			img22.convertTo(cameraImg1, CV_32FC3);
		}
		else
		{
			img2.convertTo(cameraImg1, CV_32FC3);
		}

		Tensor inputImg2(DT_FLOAT, TensorShape({ 1,224,224,3 }));
		// get pointer to memory for that Tensor
		float *p2 = inputImg2.flat<float>().data();
		cv::Mat cameraImg2(224, 224, CV_32FC3, p2);

		if (img3.cols != 224 || img3.rows != 224)
		{
			cv::Mat img33;
			cv::resize(img3, img33, cv::Size(224, 224));
			img33.convertTo(cameraImg2, CV_32FC3);
		}
		else
		{
			img3.convertTo(cameraImg2, CV_32FC3);
		}
		//input.push_back(std::pair<std::string, tensorflow::Tensor>(tname, inputImg));
		//input.push_back(std::pair<std::string, tensorflow::Tensor>(tname, inputImg1));

		input_new.push_back((tensorflow::Tensor)inputImg);
		input_new.push_back((tensorflow::Tensor)inputImg1);
		input_new.push_back((tensorflow::Tensor)inputImg2);
	}

	/**
	 * ANN Model Loader Implementation
	 * */
	ANNModelLoader::ANNModelLoader() {

	}

	ANNModelLoader::~ANNModelLoader() {

	}

	/**
	 * @brief: load the graph and add to Session
	 * @param: Session* session, add the graph to the session
	 * @param: model_path absolute path to exported protobuf file *.pb
	 * */

	int ANNModelLoader::load(tensorflow::Session* session, const std::string model_path) {
		//Read the pb file into the grapgdef member
		tensorflow::Status status_load = ReadBinaryProto(Env::Default(), model_path, &graphdef);
		if (!status_load.ok()) {
			std::cout << "ERROR: Loading model failed..." << model_path << std::endl;
			std::cout << status_load.ToString() << "\n";
			return -1;
		}

		// Add the graph to the session
		tensorflow::Status status_create = session->Create(graphdef);
		if (!status_create.ok()) {
			std::cout << "ERROR: Creating graph in session failed..." << status_create.ToString() << std::endl;
			return -1;
		}
		return 0;
	}

	/**
	 * @brief: Making new prediction
	 * @param: Session* session
	 * @param: FeatureAdapterBase, common interface of input feature
	 * @param: std::string, output_node, tensorname of output node
	 * @param: double, prediction values
	 * */

	int ANNModelLoader::predict(tensorflow::Session* session, const FeatureAdapterBase& input_feature,
		const std::string input_name, std::string output_node, double* pred_phone, double* pred_sleep)
	{
		// The session will initialize the outputs
		std::vector<tensorflow::Tensor> outputs;         //shape  [batch_size]

		int batch_size = input_feature.input_new.size();
		qDebug() << "input size = " << batch_size;

		std::vector<tensorflow::Tensor> inputList;
		if (batch_size > 1)
		{
			auto root = tensorflow::Scope::NewRootScope();
			std::string concat_name = "concatenated";
			std::vector<tensorflow::Input> ops_inputs;

			for (int i = 0; i < batch_size; i++)
			{
				ops_inputs.push_back(std::move(tensorflow::Input(input_feature.input_new[i])));
			}

			tensorflow::gtl::ArraySlice<tensorflow::Input> ipl(&ops_inputs[0], ops_inputs.size());
			tensorflow::InputList toil(ipl);
			auto concatout = tensorflow::ops::Concat(root.WithOpName(concat_name), toil, 0);
			std::unique_ptr<tensorflow::Session> concat_session(tensorflow::NewSession(tensorflow::SessionOptions()));
			tensorflow::GraphDef graph;
			root.ToGraphDef(&graph);
			concat_session->Create(graph);
			tensorflow::Status concat_run_status = concat_session->Run({}, { concat_name }, {}, &inputList);
			if (!concat_run_status.ok())
			{
				std::cout << concat_run_status.ToString() << std::endl;
			}
		}
		else
		{
			inputList = input_feature.input_new;
		}


		// @output_node: std::string, name of the output node op, defined in the protobuf file
		QTime startTime = QTime::currentTime();
		tensorflow::Status status = session->Run({ { input_name,*(inputList.begin()) } }, { output_node }, {}, &outputs);
		qDebug() << "session->Run cost time" << startTime.msecsTo(QTime::currentTime());

		if (!status.ok()) {
			qDebug() << "ERROR: prediction failed..." << QString::fromStdString(status.ToString());
			return -1;
		}
		//Fetch output value
		qDebug() << "Output tensor size:" << outputs.size();
		for (std::size_t i = 0; i < outputs.size(); i++) {
			qDebug() << QString::fromStdString(outputs[i].DebugString()) << " ";
		}
		std::cout << std::endl;

		Tensor t = outputs[0];
		int ndim = t.shape().dims();
		std::cout << "ndim:" << ndim << "\n";
		auto tmap = t.tensor<float, 2>();
		int output_dim = t.shape().dim_size(1);
		qDebug() << "output_dim:" << output_dim;
		int size = t.shape().dim_size(0);
		qDebug() << "output_size:" << size;
		std::vector<double> tout;
		double output_prob = 0.0;
		int output_list[3];
		for (int j = 0; j < size; j++) {
			qDebug() << "Item " << j << " value:" << tmap(j, 0) << "," << tmap(j, 1) << "," << tmap(j, 2) << "," << tmap(j, 3);
			float max_value = -100;
			int count = 0;
			for (int k = 0; k < output_dim; k++) {
				if (tmap(j, k) > 0) {
					count++;
				}
				if (tmap(j, k) > max_value) {
					max_value = tmap(j, k);
					output_list[j] = k;
				}

			}
			if (max_value < 8 || count >1) {
				output_list[j] = -1;
			}

			if (output_list[0] == 0) {
				(*pred_phone) = 1.0;
			}
			if (output_list[1] == 0) {
				(*pred_phone) = 2.0;
			}
			if (output_list[2] == 3) {
				(*pred_sleep) = 1.0;
			}
			return 0;
		}
	}

	int ANNModelLoader::predict_one(tensorflow::Session* session, const FeatureAdapterBase& input_feature,
		const std::string input_name, std::string output_node, double* pred_log)
	{
		// The session will initialize the outputs
		std::vector<tensorflow::Tensor> outputs;         //shape  [batch_size]

		int batch_size = input_feature.input_new.size();
		qDebug() << "input size = " << batch_size;

		std::vector<tensorflow::Tensor> inputList;
		if (batch_size > 1)
		{
			auto root = tensorflow::Scope::NewRootScope();
			std::string concat_name = "concatenated";
			std::vector<tensorflow::Input> ops_inputs;

			for (int i = 0; i < batch_size; i++)
			{
				ops_inputs.push_back(std::move(tensorflow::Input(input_feature.input_new[i])));
			}

			tensorflow::gtl::ArraySlice<tensorflow::Input> ipl(&ops_inputs[0], ops_inputs.size());
			tensorflow::InputList toil(ipl);
			auto concatout = tensorflow::ops::Concat(root.WithOpName(concat_name), toil, 0);
			std::unique_ptr<tensorflow::Session> concat_session(tensorflow::NewSession(tensorflow::SessionOptions()));
			tensorflow::GraphDef graph;
			root.ToGraphDef(&graph);
			concat_session->Create(graph);
			tensorflow::Status concat_run_status = concat_session->Run({}, { concat_name }, {}, &inputList);
			if (!concat_run_status.ok())
			{
				std::cout << concat_run_status.ToString() << std::endl;
			}
		}
		else
		{
			inputList = input_feature.input_new;
		}


		// @output_node: std::string, name of the output node op, defined in the protobuf file
		QTime startTime = QTime::currentTime();
		tensorflow::Status status = session->Run({ { input_name,*(inputList.begin()) } }, { output_node }, {}, &outputs);
		qDebug() << "session->Run cost time" << startTime.msecsTo(QTime::currentTime());

		if (!status.ok()) {
			qDebug() << "ERROR: prediction failed..." << QString::fromStdString(status.ToString());
			return -1;
		}
		//Fetch output value
		qDebug() << "Output tensor size:" << outputs.size();
		for (std::size_t i = 0; i < outputs.size(); i++) {
			qDebug() << QString::fromStdString(outputs[i].DebugString()) << " ";
		}
		std::cout << std::endl;
		Tensor t = outputs[0];
		int ndim = t.shape().dims();
		std::cout << "ndim:" << ndim << "\n";
		auto tmap = t.tensor<float, 2>();
		int output_dim = t.shape().dim_size(1);
		qDebug() << "output_dim:" << output_dim;
		int size = t.shape().dim_size(0);
		qDebug() << "output_size:" << size;
		std::vector<double> tout;
		double output_prob = 0.0;
		int output_list[2] = {-1,-1 };
		for (int j = 0; j < size; j++) {
			qDebug() << "Item " << j << " value:" << tmap(j, 0) << "," << tmap(j, 1) ;
			float max_value = -100;
			int count = 0;
			for (int k = 0; k < output_dim; k++) {
				if (tmap(j, k) > 0) {
					count++;
				}
				if (tmap(j, k) > max_value) {
					max_value = tmap(j, k);
					output_list[j] = k;
				}

			}
			if (max_value < 8 || count >1 ) {
				output_list[j] = -1;
			}
		}
		if (output_list[0] == 1 ) {
				(*pred_log) = 1.0;
			}	
		if (output_list[1] == 1) {
				(*pred_log) = 2.0;
			}	
			return 0;
		
	}
	float ANNModelLoader::predict_face(tensorflow::Session* session, const FeatureAdapterBase& input_feature,
		const std::string input_name, const std::string output_node, const std::string phase_placeholder) {

		std::vector<tensorflow::Tensor> outputs;         
		int batch_size = input_feature.input_new.size();
		std::vector<tensorflow::Tensor> inputList;
		if (batch_size > 1)
		{
			auto root = tensorflow::Scope::NewRootScope();
			std::string concat_name = "concatenated";
			std::vector<tensorflow::Input> ops_inputs;

			for (int i = 0; i < batch_size; i++)
			{
				ops_inputs.push_back(std::move(tensorflow::Input(input_feature.input_new[i])));
			}

			tensorflow::gtl::ArraySlice<tensorflow::Input> ipl(&ops_inputs[0], ops_inputs.size());
			tensorflow::InputList toil(ipl);
			auto concatout = tensorflow::ops::Concat(root.WithOpName(concat_name), toil, 0);
			std::unique_ptr<tensorflow::Session> concat_session(tensorflow::NewSession(tensorflow::SessionOptions()));
			tensorflow::GraphDef graph;
			root.ToGraphDef(&graph);
			concat_session->Create(graph);
			tensorflow::Status concat_run_status = concat_session->Run({}, { concat_name }, {}, &inputList);
			if (!concat_run_status.ok())
			{
				std::cout << concat_run_status.ToString() << std::endl;
			}
		}
		else
		{
			inputList = input_feature.input_new;
		}
		Tensor train(DT_BOOL, TensorShape());
		train.scalar<bool>()() = false;
		std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
			{ input_name,*(inputList.begin()) },
			{ phase_placeholder,train },
		};
		tensorflow::Status status = session->Run(inputs, { output_node }, {}, &outputs);//{ {input_name,*(inputList.begin())}}
		if (!status.ok())
		{
			std::cout << status.ToString() << "\n";
			return -1;
		}
		Tensor t = outputs[0];
		auto tmap = t.tensor<float, 2>();
		/*float sum_v_multi = 0;
		float sum_v1_square = 0;
		float sum_v2_square = 0;
		for (int i = 0; i < 512; i++) {
			sum_v_multi += tmap(0, i) * tmap(1, i);
			sum_v1_square += tmap(0, i) * tmap(0, i);
			sum_v2_square += tmap(1, i) * tmap(1, i);
		}
		float nrom1 = std::sqrt(sum_v1_square);
		float nrom2 = std::sqrt(sum_v2_square);
		float cos = sum_v_multi / nrom1*nrom2;*/
		float sum_square = 0;
		for (int i = 0; i < 512; i++) {
			float sub = tmap(0, i) - tmap(1, i);
			sum_square += sub*sub;
		}
		float dis = std::sqrt(sum_square);
		return dis;
	}

}
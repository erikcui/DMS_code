
#ifndef MODEL_LOADER_BASE_H_
#define MODEL_LOADER_BASE_H_

#include <iostream>
#include <vector>
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"

using namespace tensorflow;

namespace tf_model {

/**
 * Base Class for feature adapter, common interface convert input format to tensors
 * */
class FeatureAdapterBase{
public:
    FeatureAdapterBase() {};

    virtual ~FeatureAdapterBase() {};

	virtual void assign_1(std::string, cv::Mat img1) = 0;

	virtual void assign_2(std::string, cv::Mat img1, cv::Mat img2) = 0;

    virtual void assign_3(std::string, cv::Mat img1, cv::Mat img2,cv::Mat img3) = 0;  // tensor_name, tensor_double_vector

    std::vector<std::pair<std::string, tensorflow::Tensor> > input;

	std::vector<tensorflow::Tensor> input_new;

};

class ModelLoaderBase {
public:

    ModelLoaderBase() {};

    virtual ~ModelLoaderBase() {};

    virtual int load(tensorflow::Session*, const std::string) = 0;     //pure virutal function load method

    virtual int predict(tensorflow::Session*, const FeatureAdapterBase&, const std::string, std::string, double*, double*) = 0;

	virtual int predict_one(tensorflow::Session*, const FeatureAdapterBase&, const std::string, std::string, double*) = 0;

	virtual float predict_face(tensorflow::Session*, const FeatureAdapterBase&, const std::string, const std::string, const std::string) = 0;

    tensorflow::GraphDef graphdef; //Graph Definition for current model

};

}
#endif /* MODEL_LOADER_BASE_H_ */

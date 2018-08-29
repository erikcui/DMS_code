
#ifndef ANN_MODEL_LOADER_H_
#define ANN_MODEL_LOADER_H_

#include "model_loader_base.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/platform/env.h"

#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/graph/default_device.h" 
#include <algorithm> 
using namespace tensorflow;

namespace tf_model {

/**
 * @brief: Model Loader for Feed Forward Neural Network
 * */
class ANNFeatureAdapter: public FeatureAdapterBase {
public:

    ANNFeatureAdapter();

    ~ANNFeatureAdapter();

	void assign_1(std::string tname, cv::Mat img1) override;

	void assign_2(std::string tname, cv::Mat img1, cv::Mat img2) override;

    void assign_3(std::string tname, cv::Mat img1, cv::Mat img2, cv::Mat img3) override; // (tensor_name, tensor)

};

class ANNModelLoader: public ModelLoaderBase {
public:
    ANNModelLoader();

    ~ANNModelLoader();

    int load(tensorflow::Session*, const std::string) override;    //Load graph file and new session

    int predict_one(tensorflow::Session*, const FeatureAdapterBase&, const std::string, const std::string, double*) override;

	int predict(tensorflow::Session*, const FeatureAdapterBase&, const std::string, const std::string, double*, double*) override;

	float predict_face(tensorflow::Session*, const FeatureAdapterBase&, const std::string, const std::string, const std::string) override;


};

}

#endif /* ANN_MODEL_LOADER_H_ */

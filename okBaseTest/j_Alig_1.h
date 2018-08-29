#pragma once
#include <vector>
#include <string>
#include <opencv.hpp>
using namespace std;
using namespace cv;


#define SIMILARITY_TRANSFORM(x, y, scale, rotate)  {            \
        double x_tmp = scale * (rotate(0, 0)*x + rotate(0, 1)*y); \
        double y_tmp = scale * (rotate(1, 0)*x + rotate(1, 1)*y); \
        x = x_tmp; y = y_tmp;                                     \
    } 

class BBox;

class RandomTree {
public:
	RandomTree() {};
	~RandomTree() {};
	//RandomTree(const RandomTree &other);
	//RandomTree &operator=(const RandomTree &other);

public:
	void Init(int landmark_id, int depth);
	//void Read(FILE *fd);
	//void Write(FILE *fd);

public:
	int depth;
	int nodes_n;
	int landmark_id;
	Mat_<double> feats;
	vector<int> thresholds;
};

class RandomForest {
public:
	RandomForest() {};
	~RandomForest() {};
	//RandomForest(const RandomForest &other);
	//RandomForest &operator=(const RandomForest &other);

public:
	void Init(int landmark_n, int trees_n, int tree_depth);
	Mat GenerateLBF(Mat &img, Mat &current_shape, BBox &bbox, Mat &mean_shape);
	//Mat GenerateLBF(Mat &img, Mat &current_shape);//jhh

	//void Read(FILE *fd);
	//void Write(FILE *fd);

public:
	int landmark_n;
	int trees_n, tree_depth;
	std::vector<std::vector<RandomTree> > random_trees;
};

class BBox {
public:
	BBox() {};
	~BBox() {};
	BBox(double x, double y, double w, double h);

public:
	Mat Project(const Mat &shape) const;
	Mat ReProject(const Mat &shape) const;
public:
	double x, y;
	double x_center, y_center;
	double x_scale, y_scale;
	double width, height;
};

class j_Alig_1
{
private:
	int stages_n;
	int tree_n;
	int tree_depth;
	int landmark_n;
	vector<RandomForest> random_forests;
	Mat mean_shape;
	vector<Mat> gl_regression_weights;
	//
private:
	void Init(int stages_n);
	Mat GlobalRegressionPredict(const Mat &lbf, int stage);
	Mat Predict_Face_Detection(Mat &img, BBox &bbox);
	Mat Predict_Last_Predict(Mat &img, BBox &bbox,Mat &point);
public:
	bool load(string p_68_model);
	void jhh_Face_D_A(Mat& img, Rect rt, Mat& shape);
	//void jhh_Trck_O_A(Mat& img,Mat& per_p,Rect box, Mat& shape);
public:
	j_Alig_1(string p_68_model);
	j_Alig_1();
	~j_Alig_1();
};

void calcSimilarityTransform(const Mat &shape1, const Mat &shape2, double &scale, Mat &rotate);







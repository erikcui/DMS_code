#include "j_Alig_1.h"

void RandomTree::Init(int landmark_id, int depth)
{
	this->landmark_id = landmark_id;
	this->depth = depth;
	nodes_n = 1 << depth;
	feats = Mat::zeros(nodes_n, 4, CV_64FC1);
	thresholds.resize(nodes_n);
}

void RandomForest::Init(int landmark_n, int trees_n, int tree_depth)
{
	random_trees.resize(landmark_n);
	for (int i = 0; i < landmark_n; i++) {
		random_trees[i].resize(trees_n);
		for (int j = 0; j < trees_n; j++) random_trees[i][j].Init(i, tree_depth);
	}
	this->trees_n = trees_n;
	this->landmark_n = landmark_n;
	this->tree_depth = tree_depth;
}

Mat RandomForest::GenerateLBF(Mat &img, Mat &current_shape, BBox &bbox, Mat &mean_shape)
{
	Mat_<int> lbf(1, landmark_n*trees_n);
	double scale;
	Mat_<double> rotate;
	calcSimilarityTransform(bbox.Project(current_shape), mean_shape, scale, rotate);

	int base = 1 << (tree_depth - 1);

#pragma omp parallel for num_threads(2)
	for (int i = 0; i < landmark_n; i++) {
		for (int j = 0; j < trees_n; j++) {
			RandomTree &tree = random_trees[i][j];
			int code = 0;
			int idx = 1;
			for (int k = 1; k < tree.depth; k++) {
				double x1 = tree.feats(idx, 0);
				double y1 = tree.feats(idx, 1);
				double x2 = tree.feats(idx, 2);
				double y2 = tree.feats(idx, 3);
				SIMILARITY_TRANSFORM(x1, y1, scale, rotate);
				SIMILARITY_TRANSFORM(x2, y2, scale, rotate);

				x1 = x1*bbox.x_scale + current_shape.at<double>(i, 0);
				y1 = y1*bbox.y_scale + current_shape.at<double>(i, 1);
				x2 = x2*bbox.x_scale + current_shape.at<double>(i, 0);
				y2 = y2*bbox.y_scale + current_shape.at<double>(i, 1);
				x1 = max(0., min(img.cols - 1., x1)); y1 = max(0., min(img.rows - 1., y1));
				x2 = max(0., min(img.cols - 1., x2)); y2 = max(0., min(img.rows - 1., y2));
				int density = img.at<uchar>(int(y1), int(x1)) - img.at<uchar>(int(y2), int(x2));
				code <<= 1;
				if (density < tree.thresholds[idx]) {
					idx = 2 * idx;
				}
				else {
					code += 1;
					idx = 2 * idx + 1;
				}
			}
			lbf(i*trees_n + j) = (i*trees_n + j)*base + code;

		}
	}
	return lbf;
}

BBox::BBox(double x, double y, double w, double h) {
	this->x = x; this->y = y;
	this->width = w; this->height = h;
	this->x_center = x + w / 2.;
	this->y_center = y + h / 2.;
	this->x_scale = w / 2.;
	this->y_scale = h / 2.;
}

// Project absolute shape to relative shape binding to this bbox
Mat BBox::Project(const Mat &shape) const {
	Mat_<double> res(shape.rows, shape.cols);
	const Mat_<double> &shape_ = (Mat_<double>)shape;
	for (int i = 0; i < shape.rows; i++) {
		res(i, 0) = (shape_(i, 0) - x_center) / x_scale;
		res(i, 1) = (shape_(i, 1) - y_center) / y_scale;
	}
	return res;
}

// Project relative shape to absolute shape binding to this bbox
Mat BBox::ReProject(const Mat &shape) const {
	Mat_<double> res(shape.rows, shape.cols);
	const Mat_<double> &shape_ = (Mat_<double>)shape;
	for (int i = 0; i < shape.rows; i++) {
		res(i, 0) = shape_(i, 0)*x_scale + x_center;
		res(i, 1) = shape_(i, 1)*y_scale + y_center;
	}
	return res;
}

void j_Alig_1::Init(int stages_n)
{
	random_forests.resize(stages_n);
	for (int i = 0; i < stages_n; i++) random_forests[i].Init(landmark_n, tree_n, tree_depth);
	mean_shape.create(landmark_n, 2, CV_64FC1);
	gl_regression_weights.resize(stages_n);
	int F = landmark_n * tree_n * (1 << (tree_depth - 1));
	for (int i = 0; i < stages_n; i++) 
	{
		gl_regression_weights[i].create(2 * landmark_n, F, CV_64FC1);
	}
}

Mat j_Alig_1::GlobalRegressionPredict(const Mat &lbf, int stage) {
	const Mat_<double> &weight = (Mat_<double>)gl_regression_weights[stage];
	Mat_<double> delta_shape(weight.rows / 2, 2);
	const double *w_ptr = NULL;
	const int *lbf_ptr = lbf.ptr<int>(0);

	#pragma omp parallel for num_threads(2) private(w_ptr)
	for (int i = 0; i < delta_shape.rows; i++) {
		w_ptr = weight.ptr<double>(2 * i);
		double y = 0;
		for (int j = 0; j < lbf.cols; j++)
			y += w_ptr[lbf_ptr[j]];
		delta_shape(i, 0) = y;

		w_ptr = weight.ptr<double>(2 * i + 1);
		y = 0;
		for (int j = 0; j < lbf.cols; j++)
			y += w_ptr[lbf_ptr[j]];
		delta_shape(i, 1) = y;
	}
	return delta_shape;
}

Mat j_Alig_1::Predict_Last_Predict(Mat &img, BBox &bbox, Mat &point)
{
	Mat current_shape = point;
	double scale;
	Mat rotate;
	for (int k = 0; k < stages_n; k++) {
		// generate lbf
		Mat lbf = random_forests[k].GenerateLBF(img, current_shape, bbox, mean_shape);
		// update current_shapes
		Mat delta_shape = GlobalRegressionPredict(lbf, k);
		delta_shape = delta_shape.reshape(0, landmark_n);
		calcSimilarityTransform(bbox.Project(current_shape), mean_shape, scale, rotate);
		current_shape = bbox.ReProject(bbox.Project(current_shape) + scale * delta_shape * rotate.t());
	}
	return current_shape;
}

Mat j_Alig_1::Predict_Face_Detection(Mat &img, BBox &bbox)
{
	Mat current_shape = bbox.ReProject(mean_shape);
	double scale;
	Mat rotate;
	for (int k = 0; k < stages_n; k++) {
		// generate lbf
		Mat lbf = random_forests[k].GenerateLBF(img, current_shape, bbox, mean_shape);
		// update current_shapes
		Mat delta_shape = GlobalRegressionPredict(lbf, k);
		delta_shape = delta_shape.reshape(0, landmark_n);
		calcSimilarityTransform(bbox.Project(current_shape), mean_shape, scale, rotate);
		current_shape = bbox.ReProject(bbox.Project(current_shape) + scale * delta_shape * rotate.t());
	}
	return current_shape;
}

bool j_Alig_1::load(string p_68_model)
{
	FILE *fd = fopen(p_68_model.c_str(), "rb");
	//
	fread(&stages_n, sizeof(int), 1, fd);
	fread(&tree_n, sizeof(int), 1, fd);
	fread(&tree_depth, sizeof(int), 1, fd);
	fread(&landmark_n, sizeof(int), 1, fd);
	// initialize
	Init(stages_n);
	// mean_shape
	double *ptr = NULL;
	for (int i = 0; i < mean_shape.rows; i++) {
		ptr = mean_shape.ptr<double>(i);
		fread(ptr, sizeof(double), mean_shape.cols, fd);
	}
	// every stages
	for (int k = 0; k < stages_n; k++) 
	{
		//random_forests[k].Read(fd);
		for (int i = 0; i < landmark_n; i++) {
			for (int j = 0; j < random_forests[k].trees_n; j++) {
				//random_trees[i][j].Read(fd);
				for (int i2 = 1; i2 < random_forests[k].random_trees[i][j].nodes_n / 2; i2++) {
					fread(random_forests[k].random_trees[i][j].feats.ptr<double>(i2), sizeof(double), 4, fd);
					fread(&(random_forests[k].random_trees[i][j].thresholds[i2]), sizeof(int), 1, fd);
				}
			}
		}
		//
		for (int i = 0; i < 2 * landmark_n; i++) 
		{
			ptr = gl_regression_weights[k].ptr<double>(i);
			fread(ptr, sizeof(double), gl_regression_weights[k].cols, fd);
		}
	}
	//
	fclose(fd);
	return true;
}

void j_Alig_1::jhh_Face_D_A(Mat& img,Rect rt, Mat& shape)
{
	BBox bbox(rt.x, rt.y, rt.width, rt.height);
	shape = Predict_Face_Detection(img, bbox);
}

//void j_Alig_1::jhh_Trck_O_A(Mat& img, Mat& per_p, Rect box, Mat& shape)
//{
//	BBox bbox(box.x, box.y, box.width, box.height);
//	shape = Predict_Last_Predict(img, bbox, per_p);
//}


j_Alig_1::j_Alig_1(string p_68_model)
{
	load(p_68_model);
}


j_Alig_1::j_Alig_1()
{
}


j_Alig_1::~j_Alig_1()
{
}


void calcSimilarityTransform(const Mat &shape1, const Mat &shape2, double &scale, Mat &rotate) 
{
	Mat_<double> rotate_(2, 2);
	double x1_center, y1_center, x2_center, y2_center;
	x1_center = cv::mean(shape1.col(0))[0];
	y1_center = cv::mean(shape1.col(1))[0];
	x2_center = cv::mean(shape2.col(0))[0];
	y2_center = cv::mean(shape2.col(1))[0];

	Mat temp1(shape1.rows, shape1.cols, CV_64FC1);
	Mat temp2(shape2.rows, shape2.cols, CV_64FC1);
	temp1.col(0) = shape1.col(0) - x1_center;
	temp1.col(1) = shape1.col(1) - y1_center;
	temp2.col(0) = shape2.col(0) - x2_center;
	temp2.col(1) = shape2.col(1) - y2_center;

	Mat_<double> covar1, covar2;
	Mat_<double> mean1, mean2;
	calcCovarMatrix(temp1, covar1, mean1, CV_COVAR_COLS);
	calcCovarMatrix(temp2, covar2, mean2, CV_COVAR_COLS);

	double s1 = sqrt(cv::norm(covar1));
	double s2 = sqrt(cv::norm(covar2));
	scale = s1 / s2;
	temp1 /= s1;
	temp2 /= s2;

	double num = temp1.col(1).dot(temp2.col(0)) - temp1.col(0).dot(temp2.col(1));
	double den = temp1.col(0).dot(temp2.col(0)) + temp1.col(1).dot(temp2.col(1));
	double normed = sqrt(num*num + den*den);
	double sin_theta = num / normed;
	double cos_theta = den / normed;
	rotate_(0, 0) = cos_theta; rotate_(0, 1) = -sin_theta;
	rotate_(1, 0) = sin_theta; rotate_(1, 1) = cos_theta;
	rotate = rotate_;
}
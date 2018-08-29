#pragma once
#include <stdio.h>
#include <string>
#include <vector>
#include <opencv.hpp>

#include <omp.h>

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
//#define j_use_EffSubSea

using namespace std;
using namespace cv;

#ifdef j_use_EffSubSea
struct ess_s_Box
{
	int left;
	int top;
	int right;
	int bottom;
	inline ess_s_Box() :left(0), top(0), right(0), bottom(0) {}
	inline void s_assign(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
};

class ess_Box
{
public:
	ess_s_Box upper;
	ess_s_Box lower;
	float prior;
public:
	int split(ess_Box& r1,ess_Box& r2);
	bool is_one();
	inline bool operator<(const ess_Box& r) const { return prior<r.prior; }
	void assign(int o_l, int o_t, int o_r, int o_b,int u_l, int u_t, int u_r, int u_b);
	ess_Box(int o_l, int o_t, int o_r, int o_b, int u_l, int u_t, int u_r, int u_b);
	ess_Box(int width,int height);
	ess_Box();
	~ess_Box();
};

#endif // j_use_EffSubSea


struct j_npd_d_opt
{
	int objSize;
	int treeLevel;
	/* \breif max number of stages */
	int maxNumWeaks;
	/* \breif threads to use */
	int numThreads;
	/* \breif recall of positive in every stages */
	double minDR;
	/* \breif end condition of the training */
	double maxFAR;
	/* \breif max value of weight */
	int maxWeight;
	/* \breif factor for decide leaf number */
	double minLeafFrac;
	/* \breif minimum leaf number */
	int minLeaf;
	/* \breif factor to decide how many samples should be filter befor training a stage */
	double trimFrac;
	/* \breif minimum samples required */
	int minSamples;
	/* \breif data augment or not */
	bool augment;
	/* \breif step of stages to save the model */
	int saveStep;
	/* \breif generate init neg if need */
	bool generate_hd;
	/* \breif use for resize box */
	float enDelta;
	/* \use hd or not */
	bool useInitHard;
};
class j_npd_d
{
private:
	j_npd_d_opt opt;
	int DetectSize;
	int stages;
	int numBranchNodes;
	vector<int> treeIndex;
	vector<float> thresholds;
	vector<int> feaIds, leftChilds, rightChilds;
	vector<unsigned char> cutpoints;
	//vector<float> fits;
	//vector<float> thresholds;
	vector< vector<int> > points1, points2;
	const int* pWinSize;
	Mat_<uchar> ppNpdTable;
	vector<int> lpoints;
	vector<int> rpoints;
	vector<float> fits;
private:
	void GetPoints(int feaid, int *x1, int *y1, int *x2, int *y2);
	void Initialize();
	vector<int> Nms(vector<cv::Rect>& rects, vector<float>& scores, vector<int>& Srect, float overlap, Mat img);
	int Partation(Mat predicate, vector<int>& label);
	int Find(vector<int>& parent, int x);
	vector<float> Logistic(vector<float> scores, vector<int> index);
public:
	bool Load(string path);
	vector<int> DetectFace(Mat img, vector<Rect>& rects, vector<float>& scores);
	vector<int> DetectFace(Mat img, Mat_<float>& lbp_teg,int lbp_hold, vector<Rect>& rects, vector<float>& scores);
	bool is_Face(Mat img,Rect rt);//jhh
#ifdef j_use_EffSubSea
	bool DttF_ESS(Mat img,Rect& rect);
#endif
public:
	j_npd_d(string path);
	j_npd_d();
	~j_npd_d();
};


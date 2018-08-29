#include "j_npd_d.h"

int j_npd_WinSize[] = { 24,29,35,41,50,60,72,86,103,124,149,178,214,257,308,370,444,532,639,767,920,1104,1325,1590,1908,2290,2747,3297,3956 };

void j_npd_d::Initialize()
{
	{
		opt.objSize = 24;
		opt.treeLevel = 8;
		opt.maxNumWeaks = 1000;
		opt.numThreads = 16;
		opt.minDR = 1.0;
		opt.maxFAR = 0;
		opt.trimFrac = 0.005;
		opt.minLeafFrac = 0.01;
		opt.minLeaf = 100;
		opt.minSamples = 1000;
		opt.maxWeight = 100;
		opt.augment = true;
		opt.saveStep = 10;
		opt.generate_hd = false;
		opt.enDelta = 0.1;
		opt.useInitHard = false;
	}

	stages = 0;

	ppNpdTable.create(256, 256);
	for (int i = 0; i < 256; i++)
	{
		for (int j = 0; j < 256; j++)
		{
			double fea = 0.5;
			if (i > 0 || j > 0) fea = double(i) / (double(i) + double(j));
			fea = floor(256 * fea);
			if (fea > 255) fea = 255;

			ppNpdTable(i, j) = (unsigned char)fea;
		}
	}

	size_t numPixels = opt.objSize*opt.objSize;
	for (int i = 0; i < numPixels; i++)
	{
		for (int j = i + 1; j < numPixels; j++)
		{
			lpoints.push_back(i);
			rpoints.push_back(j);
		}
	}

	points1.resize(29);
	points2.resize(29);
}

void j_npd_d::GetPoints(int feaid, int *x1, int *y1, int *x2, int *y2)
{
	int lpoint = lpoints[feaid];
	int rpoint = rpoints[feaid];
	//use the model trained by yourself
	*y1 = lpoint%opt.objSize;
	*x1 = lpoint / opt.objSize;
	*y2 = rpoint%opt.objSize;
	*x2 = rpoint / opt.objSize;
}

bool j_npd_d::Load(string path)
{
	FILE* file;
	if ((file = fopen(path.c_str(), "rb")) == NULL)
		return false;
	fread(&DetectSize, sizeof(int), 1, file);
	fread(&stages, sizeof(int), 1, file);
	fread(&numBranchNodes, sizeof(int), 1, file);
	printf("stages num :%d\n", stages);

	int *_tree = new int[stages];
	float *_threshold = new float[stages];
	fread(_tree, sizeof(int), stages, file);
	fread(_threshold, sizeof(float), stages, file);
	for (int i = 0; i<stages; i++) {
		treeIndex.push_back(_tree[i]);
		thresholds.push_back(_threshold[i]);
	}
	delete[]_tree;
	delete[]_threshold;

	int *_feaId = new int[numBranchNodes];
	int *_leftChild = new int[numBranchNodes];
	int *_rightChild = new int[numBranchNodes];
	unsigned char* _cutpoint = new unsigned char[2 * numBranchNodes];
	fread(_feaId, sizeof(int), numBranchNodes, file);
	fread(_leftChild, sizeof(int), numBranchNodes, file);
	fread(_rightChild, sizeof(int), numBranchNodes, file);
	fread(_cutpoint, sizeof(unsigned char), 2 * numBranchNodes, file);

	for (int i = 0; i<numBranchNodes; i++) {
		feaIds.push_back(_feaId[i]);
		leftChilds.push_back(_leftChild[i]);
		rightChilds.push_back(_rightChild[i]);
		cutpoints.push_back(_cutpoint[2 * i]);
		cutpoints.push_back(_cutpoint[2 * i + 1]);
		for (int j = 0; j<29; j++) {
			int x1, y1, x2, y2;
			GetPoints(_feaId[i], &x1, &y1, &x2, &y2);
			float factor = (float)pWinSize[j] / (float)DetectSize;
			int p1x = x1*factor;
			int p1y = y1*factor;
			int p2x = x2*factor;
			int p2y = y2*factor;
			points1[j].push_back(p1y*pWinSize[j] + p1x);
			points2[j].push_back(p2y*pWinSize[j] + p2x);
		}
	}
	delete[]_feaId;
	delete[]_leftChild;
	delete[]_rightChild;
	delete[]_cutpoint;

	int numLeafNodes = numBranchNodes + stages;
	float *_fit = new float[numLeafNodes];
	fread(_fit, sizeof(float), numLeafNodes, file);
	for (int i = 0; i<numLeafNodes; i++) {
		fits.push_back(_fit[i]);
	}
	delete[]_fit;

	fclose(file);
	return true;
}

int j_npd_d::Find(vector<int>& parent, int x) {
	int root = parent[x];
	if (root != x)
		root = Find(parent, root);
	return root;
}

int j_npd_d::Partation(Mat predicate, vector<int>& label) {
	int N = predicate.cols;
	vector<int> parent;
	vector<int> rank;
	for (int i = 0; i<N; i++) {
		parent.push_back(i);
		rank.push_back(0);
	}

	for (int i = 0; i<N; i++) {
		for (int j = 0; j<N; j++) {
			if (predicate.at<bool>(i, j) == 0)
				continue;
			int root_i = Find(parent, i);
			int root_j = Find(parent, j);

			if (root_j != root_i) {
				if (rank[root_j] < rank[root_i])
					parent[root_j] = root_i;
				else if (rank[root_j] > rank[root_i])
					parent[root_i] = root_j;
				else {
					parent[root_j] = root_i;
					rank[root_i] = rank[root_i] + 1;
				}
			}
		}
	}

	int nGroups = 0;
	label.resize(N);
	for (int i = 0; i<N; i++) {
		if (parent[i] == i) {
			label[i] = nGroups;
			nGroups++;
		}
		else label[i] = -1;
	}

	for (int i = 0; i<N; i++) {
		if (parent[i] == i)
			continue;
		int root_i = Find(parent, i);
		label[i] = label[root_i];
	}

	return nGroups;
}

vector<float> j_npd_d::Logistic(vector<float> scores, vector<int> index) {
	vector<float> Y;
	for (int i = 0; i<index.size(); i++) {
		float tmp_Y = log(1 + exp(scores[index[i]]));
		if (isinf(tmp_Y))
			tmp_Y = scores[index[i]];
		Y.push_back(tmp_Y);
	}
	return Y;
}


vector<int> j_npd_d::Nms(vector<Rect>& rects, vector<float>& scores, vector<int>& Srect, float overlap, Mat Img) {
	int numCandidates = rects.size();
	Mat predicate = Mat::eye(numCandidates, numCandidates, IPL_DEPTH_1U);
	for (int i = 0; i<numCandidates; i++) {
		for (int j = i + 1; j<numCandidates; j++) {
			int h = min(rects[i].y + rects[i].height, rects[j].y + rects[j].height) - max(rects[i].y, rects[j].y);
			int w = min(rects[i].x + rects[i].width, rects[j].x + rects[j].width) - max(rects[i].x, rects[j].x);
			int s = max(h, 0)*max(w, 0);

			if ((float)s / (float)(rects[i].width*rects[i].height + rects[j].width*rects[j].height - s) >= overlap) {
				predicate.at<bool>(i, j) = 1;
				predicate.at<bool>(j, i) = 1;
			}
		}
	}

	vector<int> label;

	int numLabels = Partation(predicate, label);

	vector<Rect> Rects;
	Srect.resize(numLabels);
	vector<int> neighbors;
	neighbors.resize(numLabels);
	vector<float> Score;
	Score.resize(numLabels);

	for (int i = 0; i<numLabels; i++) {
		vector<int> index;
		for (int j = 0; j<numCandidates; j++) {
			if (label[j] == i)
				index.push_back(j);
		}
		vector<float> weight;
		weight = Logistic(scores, index);
		float sumScore = 0;
		for (int j = 0; j<weight.size(); j++)
			sumScore += weight[j];
		Score[i] = sumScore;
		neighbors[i] = index.size();

		if (sumScore == 0) {
			for (int j = 0; j<weight.size(); j++)
				weight[j] = 1 / sumScore;
		}
		else {
			for (int j = 0; j<weight.size(); j++)
				weight[j] = weight[j] / sumScore;
		}

		float size = 0;
		float col = 0;
		float row = 0;
		for (int j = 0; j<index.size(); j++) {
			size += rects[index[j]].width*weight[j];
		}
		Srect[i] = (int)floor(size);
		for (int j = 0; j<index.size(); j++) {
			col += (rects[index[j]].x + rects[index[j]].width / 2)*weight[j];
			row += (rects[index[j]].y + rects[index[j]].width / 2)*weight[j];
		}
		int x = floor(col - size / 2);
		int y = floor(row - size / 2);
		Rect roi(x, y, Srect[i], Srect[i]);
		Rects.push_back(roi);
	}


	predicate = Mat::zeros(numLabels, numLabels, IPL_DEPTH_1U);

	for (int i = 0; i<numLabels; i++) {
		for (int j = i + 1; j<numLabels; j++) {
			int h = min(Rects[i].y + Rects[i].height, Rects[j].y + Rects[j].height) - max(Rects[i].y, Rects[j].y);
			int w = min(Rects[i].x + Rects[i].width, Rects[j].x + Rects[j].width) - max(Rects[i].x, Rects[j].x);
			int s = max(h, 0)*max(w, 0);

			if ((float)s / (float)(Rects[i].width*Rects[i].height) >= overlap || (float)s / (float)(Rects[j].width*Rects[j].height) >= overlap)
			{
				predicate.at<bool>(i, j) = 1;
				predicate.at<bool>(j, i) = 1;
			}
		}
	}

	vector<int> flag;
	flag.resize(numLabels);
	for (int i = 0; i<numLabels; i++)
		flag[i] = 1;

	for (int i = 0; i<numLabels; i++) {
		vector<int> index;
		for (int j = 0; j<numLabels; j++) {
			if (predicate.at<bool>(j, i) == 1)
				index.push_back(j);
		}
		if (index.size() == 0)
			continue;

		float s = 0;
		for (int j = 0; j<index.size(); j++) {
			if (Score[index[j]]>s)
				s = Score[index[j]];
		}
		if (s>Score[i])
			flag[i] = 0;
	}

	vector<int> picked;
	for (int i = 0; i<numLabels; i++) {
		if (flag[i]) {
			picked.push_back(i);
		}
	}

	int height = Img.rows;
	int width = Img.cols;

	for (int i = 0; i<picked.size(); i++) {
		int idx = picked[i];
		if (Rects[idx].x<0)
			Rects[idx].x = 0;

		if (Rects[idx].y<0)
			Rects[idx].y = 0;

		if (Rects[idx].y + Rects[idx].height>height)
			Rects[idx].height = height - Rects[idx].y;

		if (Rects[idx].x + Rects[idx].width>width)
			Rects[idx].width = width - Rects[idx].x;
	}

	rects = Rects;
	scores = Score;
	return picked;
}

vector<int> j_npd_d::DetectFace(Mat img, vector<Rect>& rects, vector<float>& scores)
{
	int minFace = 150;//20
	int maxFace = 550;//3000
	omp_set_num_threads(opt.numThreads);

	int height = img.rows;
	int width = img.cols;
	const unsigned char *O = (unsigned char *)img.data;
	unsigned char *I = new unsigned char[width*height];
	int k = 0;
	for (int i = 0; i<width; i++) {
		for (int j = 0; j<height; j++) {
			I[k] = *(O + j*width + i);
			k++;
		}
	}

	minFace = max(minFace, opt.objSize);
	//maxFace = min(maxFace, min(height, width)); //jhh
	maxFace = min(maxFace, min(height / 2, width / 2));

	vector<int> picked;
	if (min(height, width) < minFace)
	{
		return picked;
	}
	for (int k = 0; k < 29; k++) // process each scale
	{
		if (pWinSize[k] < minFace) continue;
		else if (pWinSize[k] > maxFace) break;

		// determine the step of the sliding subwindow
		int winStep = (int)floor(pWinSize[k] * 0.1);
		if (pWinSize[k] > 40) winStep = (int)floor(pWinSize[k] * 0.05);

		// calculate the offset values of each pixel in a subwindow
		// pre-determined offset of pixels in a subwindow
		vector<int> offset(pWinSize[k] * pWinSize[k]);
		int pp1 = 0, pp2 = 0, gap = height - pWinSize[k];

		for (int j = 0; j < pWinSize[k]; j++) // column coordinate
		{
			for (int i = 0; i < pWinSize[k]; i++) // row coordinate
			{
				offset[pp1++] = pp2++;
			}

			pp2 += gap;
		}
		int colMax = width - pWinSize[k] + 1;
		int rowMax = height - pWinSize[k] + 1;

		// process each subwindow
#pragma omp parallel for
		for (int c = 0; c < colMax; c += winStep) // slide in column
		{
			const unsigned char *pPixel = I + c * height;

			for (int r = 0; r < rowMax; r += winStep, pPixel += winStep) // slide in row
			{
				float _score = 0;
				int s;


				// test each tree classifier
				for (s = 0; s < stages; s++)
				{
					int node = treeIndex[s];

					// test the current tree classifier
					while (node > -1) // branch node
					{
						unsigned char p1 = pPixel[offset[points1[k][node]]];
						unsigned char p2 = pPixel[offset[points2[k][node]]];
						unsigned char fea = ppNpdTable.at<uchar>(p1, p2);

						if (fea < cutpoints[2 * node] || fea > cutpoints[2 * node + 1]) node = leftChilds[node];
						else node = rightChilds[node];

					}

					node = -node - 1;
					_score = _score + fits[node];

					if (_score < thresholds[s]) {
						break; // negative samples
					}
				}

				if (s == stages) // a face detected
				{
					Rect roi(c, r, pWinSize[k], pWinSize[k]);
#pragma omp critical // modify the record by a single thread
					{
						rects.push_back(roi);
						scores.push_back(_score);
					}
				}
			}
		}
	}
	vector<int> Srect;
	picked = Nms(rects, scores, Srect, 0.5, img);

	int imgWidth = img.cols;
	int imgHeight = img.rows;


	//you should set the parameter by yourself
	for (int i = 0; i<picked.size(); i++) {
		int idx = picked[i];
		int delta = floor(Srect[idx] * opt.enDelta);
		int y0 = max(rects[idx].y - floor(3.0 * delta), 0);
		int y1 = min(rects[idx].y + Srect[idx], imgHeight);
		int x0 = max(rects[idx].x + floor(0.25 * delta), 0);
		int x1 = min(rects[idx].x + Srect[idx] - floor(0.25 * delta), imgWidth);

		rects[idx].y = y0;
		rects[idx].x = x0;
		rects[idx].width = x1 - x0 + 1;
		rects[idx].height = y1 - y0 + 1;
	}


	delete[]I;
	return picked;
}

vector<int> j_npd_d::DetectFace(Mat img, Mat_<float>& lbp_teg, int lbp_hold, vector<Rect>& rects, vector<float>& scores)
{
	int minFace = 20;
	int maxFace = 3000;

	omp_set_num_threads(opt.numThreads);

	int height = img.rows;
	int width = img.cols;
	const unsigned char *O = (unsigned char *)img.data;
	unsigned char *I = new unsigned char[width*height];
	int k = 0;
	for (int i = 0; i<width; i++) {
		for (int j = 0; j<height; j++) {
			I[k] = *(O + j*width + i);
			k++;
		}
	}

	minFace = max(minFace, opt.objSize);
	//maxFace = min(maxFace, min(height, width)); //jhh
	maxFace = min(maxFace, min(height / 2, width / 2));

	vector<int> picked;
	if (min(height, width) < minFace)
	{
		return picked;
	}

//#define j_npd_d_Tactical_Statistics
#ifdef j_npd_d_Tactical_Statistics
	int sliding_win_num = 0;
	int LBP_Interception_Win = 0;
#endif // j_npd_d_Tactical_Statistics

	for (int k = 0; k < 29; k++) // process each scale
	{
		if (pWinSize[k] < minFace) continue;
		else if (pWinSize[k] > maxFace) break;

		// determine the step of the sliding subwindow
		int winStep = (int)floor(pWinSize[k] * 0.1);
		if (pWinSize[k] > 40) winStep = (int)floor(pWinSize[k] * 0.05);

		// calculate the offset values of each pixel in a subwindow
		// pre-determined offset of pixels in a subwindow
		vector<int> offset(pWinSize[k] * pWinSize[k]);
		int pp1 = 0, pp2 = 0, gap = height - pWinSize[k];

		for (int j = 0; j < pWinSize[k]; j++) // column coordinate
		{
			for (int i = 0; i < pWinSize[k]; i++) // row coordinate
			{
				offset[pp1++] = pp2++;
			}

			pp2 += gap;
		}
		int colMax = width - pWinSize[k] + 1;
		int rowMax = height - pWinSize[k] + 1;
		//Tactical Statistics


		// process each subwindow
#pragma omp parallel for
		for (int c = 0; c < colMax; c += winStep) // slide in column
		{
			const unsigned char *pPixel = I + c * height;

			for (int r = 0; r < rowMax; r += winStep, pPixel += winStep) // slide in row
			{
				//<jhh>
#ifdef j_npd_d_Tactical_Statistics
				sliding_win_num ++;
#endif // j_npd_d_Tactical_Statistics
				Rect j_roi(c, r, pWinSize[k], pWinSize[k]);
				float lt1 = lbp_teg(j_roi.y, j_roi.x);
				float lt2 = lbp_teg(j_roi.y + j_roi.height, j_roi.x);
				float lt3 = lbp_teg(j_roi.y, j_roi.x + j_roi.width);
				float lt4 = lbp_teg(j_roi.y + j_roi.height, j_roi.x + j_roi.width);
				float lt_sum = lt4 - lt2 - lt3 + lt1;
				lt_sum = lt_sum / (j_roi.height*j_roi.width);
				if (lt_sum <= lbp_hold)
				{
#ifdef j_npd_d_Tactical_Statistics
					LBP_Interception_Win++;
#endif // j_npd_d_Tactical_Statistics
					continue;
				}
				//<\jhh>
				float _score = 0;
				int s;
				// test each tree classifier
				for (s = 0; s < stages; s++)
				{
					int node = treeIndex[s];

					// test the current tree classifier
					while (node > -1) // branch node
					{
						unsigned char p1 = pPixel[offset[points1[k][node]]];
						unsigned char p2 = pPixel[offset[points2[k][node]]];
						unsigned char fea = ppNpdTable.at<uchar>(p1, p2);

						if (fea < cutpoints[2 * node] || fea > cutpoints[2 * node + 1]) node = leftChilds[node];
						else node = rightChilds[node];

					}

					node = -node - 1;
					_score = _score + fits[node];

					if (_score < thresholds[s]) {
						break; // negative samples
					}
				}

				if (s == stages) // a face detected
				{
					Rect roi(c, r, pWinSize[k], pWinSize[k]);
#pragma omp critical // modify the record by a single thread
					{
						rects.push_back(roi);
						scores.push_back(_score);
					}
				}
			}
		}
	}
#ifdef j_npd_d_Tactical_Statistics
	printf("sliding_win_num=%d ; LBP_Interception_Win=%d\n", sliding_win_num, LBP_Interception_Win);
#endif // j_npd_d_Tactical_Statistics
	vector<int> Srect;
	picked = Nms(rects, scores, Srect, 0.5, img);

	int imgWidth = img.cols;
	int imgHeight = img.rows;


	//you should set the parameter by yourself
	for (int i = 0; i<picked.size(); i++) {
		int idx = picked[i];
		int delta = floor(Srect[idx] * opt.enDelta);
		int y0 = max(rects[idx].y - floor(3.0 * delta), 0);
		int y1 = min(rects[idx].y + Srect[idx], imgHeight);
		int x0 = max(rects[idx].x + floor(0.25 * delta), 0);
		int x1 = min(rects[idx].x + Srect[idx] - floor(0.25 * delta), imgWidth);

		rects[idx].y = y0;
		rects[idx].x = x0;
		rects[idx].width = x1 - x0 + 1;
		rects[idx].height = y1 - y0 + 1;
	}


	delete[]I;
	return picked;
}


bool j_npd_d::is_Face(Mat img, Rect rt)
{
	Mat img2(img, rt);
	//int height = img2.rows;
	//int width = img2.cols;
	
	int k_s = 7;
	Mat img3;
	resize(img2, img3, Size(pWinSize[k_s], pWinSize[k_s]));

	const unsigned char *O = (unsigned char *)img3.data;
	unsigned char *I = new unsigned char[pWinSize[k_s] * pWinSize[k_s]];
	int k = 0;
	for (int i = 0; i<pWinSize[k_s]; i++) {
		for (int j = 0; j<pWinSize[k_s]; j++) {
			I[k] = *(O + j*pWinSize[k_s] + i);
			k++;
		}
	}

	vector<int> offset(pWinSize[k_s] * pWinSize[k_s]);
	int pp1 = 0, pp2 = 0, gap =0;

	for (int j = 0; j < pWinSize[k_s]; j++) // column coordinate
	{
		for (int i = 0; i < pWinSize[k_s]; i++) // row coordinate
		{
			offset[pp1++] = pp2++;
		}

		pp2 += gap;
	}

	const unsigned char *pPixel = I;
	float _score = 0;
	int s;

	for (s = 0; s < stages; s++)
	{
		int node = treeIndex[s];

		// test the current tree classifier
		while (node > -1) // branch node
		{
			unsigned char p1 = pPixel[offset[points1[k_s][node]]];
			unsigned char p2 = pPixel[offset[points2[k_s][node]]];
			unsigned char fea = ppNpdTable.at<uchar>(p1, p2);

			if (fea < cutpoints[2 * node] || fea > cutpoints[2 * node + 1]) node = leftChilds[node];
			else node = rightChilds[node];

		}

		node = -node - 1;
		_score = _score + fits[node];

		if (_score < thresholds[s]) {
			return false;
		}
	}

	return true;

}

#ifdef j_use_EffSubSea

int ess_Box::split(ess_Box& r1, ess_Box& r2)
{
	int maxi = -1, type = -1;
	int mid;
	if (is_one()) return -1; 
	if (upper.left - lower.left>maxi) { maxi = upper.left - lower.left; type = 0; }
	if (upper.top - lower.top>maxi) { maxi = upper.top - lower.top; type = 1; }
	if (upper.right - lower.right>maxi) { maxi = upper.right - lower.right; type = 2; }
	if (upper.bottom - lower.bottom>maxi) { maxi = upper.bottom - lower.bottom; type = 3; }
	//assert(type >= 0 && type <= 1);
	switch (type)
	{
	case 0:
		mid = (upper.left + lower.left) / 2;
		r1.assign(lower.left, lower.top, lower.right, lower.bottom, mid, upper.top, upper.right, upper.bottom);
		r2.assign(mid+1, lower.top, lower.right, lower.bottom, upper.left, upper.top, upper.right, upper.bottom);
		break;
	case 1:
		mid = (upper.top + lower.top) / 2;
		r1.assign(lower.left, lower.top, lower.right, lower.bottom, upper.left, mid, upper.right, upper.bottom);
		r2.assign(lower.left, mid + 1, lower.right, lower.bottom, upper.left, upper.top, upper.right, upper.bottom);
		break;
	case 2:
		mid = (upper.right + lower.right) / 2;
		r1.assign(lower.left, lower.top, lower.right, lower.bottom, upper.left, upper.top, mid, upper.bottom);
		r2.assign(lower.left, lower.top, mid + 1, lower.bottom, upper.left, upper.top, upper.right, upper.bottom);
		break;
	case 3:
		mid = (upper.bottom + lower.bottom) / 2;
		r1.assign(lower.left, lower.top, lower.right, lower.bottom, upper.left, upper.top, upper.right, mid);
		r2.assign(lower.left, lower.top, lower.right, mid + 1, upper.left, upper.top, upper.right, upper.bottom);
		break;
	default:
		break;
	}
	return type;
}

bool ess_Box::is_one()
{
	return 
		(upper.left == lower.left && 
			upper.top == lower.top &&
			upper.right == lower.right &&
			upper.bottom == lower.bottom);
}

void ess_Box::assign(int o_l, int o_t, int o_r, int o_b, int u_l, int u_t, int u_r, int u_b)
{
	upper.s_assign(u_l, u_t, u_r, u_b);
	lower.s_assign(o_l, o_t, o_r, o_b);
}

ess_Box::ess_Box(int o_l, int o_t, int o_r, int o_b, int u_l, int u_t, int u_r, int u_b)
{
	upper.s_assign(u_l, u_t, u_r, u_b);
	lower.s_assign(o_l, o_t, o_r, o_b);
	prior = 0;
}

ess_Box::ess_Box(int width, int height)
{
	lower.s_assign(1,1,1,1);
	upper.s_assign(width-1, height-1, width-1, height-1);
	prior = 0;
}

ess_Box::ess_Box()
{
	prior = 0;
}

ess_Box::~ess_Box()
{

}

bool j_npd_d::DttF_ESS(Mat img, Rect& rect)
{
	bool res = false;

	return res;
}
#endif // j_useEffSubSea



j_npd_d::j_npd_d(string path) :pWinSize(j_npd_WinSize)
{
	Initialize();
	if (!Load(path))
	{
		printf("Load_error\n");
	}
}

j_npd_d::j_npd_d() :pWinSize(j_npd_WinSize)
{
	Initialize();
}


j_npd_d::~j_npd_d()
{
}

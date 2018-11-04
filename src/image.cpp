#include"image.h"
#include<string>
#include<cmath>
#include<fstream>

using namespace std;

Image::Image(int H, int W) {
	initialize(H, W);
}

Image::~Image() {
	release();
}

void Image::initialize(int H, int W) {
	h = H, w = W;
	img = new Color*[H];
	cnt = new int*[H];
	for (int i = 0; i < H; ++ i) {
		img[i] = new Color[W];
		cnt[i] = new int[W];
	}
	for (int i = 0; i < H; ++ i)
		for (int j = 0; j < W; ++ j)
			cnt[i][j] = 0;
}

void Image::release() {
	for (int i = 0; i < h; ++ i) {
		delete[] img[i];
		delete[] cnt[i];
	}
	delete[] img;
	delete[] cnt;
}

void Image::setColor(int i, int j, Color c) {
	img[i][j] = (img[i][j] * cnt[i][j] + c) / float(cnt[i][j] + 1);
	cnt[i][j] ++;
}

Color Image::getColor(int i, int j) {
	return img[i][j];
}

Color Image::getColor(double u, double v) {
	double U = (u - floor(u)) * h;
	double V = (v - floor(v)) * w;
	int U1 = (int)floor(U) , U2 = U1 + 1;
	int V1 = (int)floor(V) , V2 = V1 + 1;
	double rat_U = U2 - U;
	double rat_V = V2 - V;
	if (U1 < 0) U1 = h - 1;
	if (U2 == h) U2 = 0;
	if (V1 < 0) V1 = w - 1; 
	if (V2 == w) V2 = 0;
	Color ret;
	ret = ret + img[U1][V1] * rat_U * rat_V;
	ret = ret + img[U1][V2] * rat_U * ( 1 - rat_V );
	ret = ret + img[U2][V1] * ( 1 - rat_U ) * rat_V;
	ret = ret + img[U2][V2] * ( 1 - rat_U ) * ( 1 - rat_V );
	return ret;
}

void Image::load(string file) {
	fstream fin(file, fstream::in);
	int H, W, c;
	float r, g, b;
	fin >> H >> W;
	initialize(H, W);
	for (int i = 0; i < H; ++ i)
		for (int j = 0; j < W; ++ j) {
			fin >> c >> r >> g >> b;
			cnt[i][j] = c;
			img[i][j] = Color(r, g, b);
		}
}

void Image::dump(string file) {
	fstream fout(file, fstream::out);
	fout << h << " " << w << endl;
	for (int i = 0; i < h; ++ i)
		for (int j = 0; j < w; ++ j) {
			fout << cnt[i][j] << ' ';
			fout << img[i][j].r << ' ';
			fout << img[i][j].g << ' ';
			fout << img[i][j].b << endl;
		}
}

void Image::merge(Image* im) {
	if (h != im -> h || w != im -> w) return;
	for (int i = 0; i < h; ++ i)
		for (int j = 0; j < w; ++ j)
			if (cnt[i][j] + im -> cnt[i][j] != 0) {
				img[i][j] = (img[i][j] * cnt[i][j] + im -> img[i][j] * im -> cnt[i][j]) / float(cnt[i][j] + im -> cnt[i][j]);
				cnt[i][j] += im -> cnt[i][j];
			}
}

Bmp* Image::toBmp() {
	Bmp* bmp = new Bmp(h, w);
	for (int i = 0; i < h; ++ i)
		for (int j = 0; j < w; ++ j)
			bmp -> SetColor(i, j, img[i][j].Confine());
	return bmp;
}

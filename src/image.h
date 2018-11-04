#ifndef _IMAGE_H_
#define _IMAGE_H_

#include"color.h"
#include"bmp.h"
#include<string>

// ͼ���࣬���ı���ʽ��¼ͼ����Ϣ
// ��¼ÿ�����ص�ĵ�����������ɫ
class Image {
	Color** img;
	int** cnt;
	int h, w;

	void initialize(int, int);
	void release();

public:
	Image(int H = 0, int W = 0);
	~Image();

	int getH() { return h; }
	int getW() { return w; }
	void setColor(int, int, Color);
	Color getColor(int, int);
	Color getColor(double, double);

	void load(std::string);
	void dump(std::string);
	Bmp* toBmp();

	void merge(Image*);
};

#endif

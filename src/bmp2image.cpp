#include "image.h"
#include "bmp.h"

using namespace std;
const float EPS = 1e-3;

int main(int argc, char** argv) {
	Bmp *bmp = new Bmp();
	bmp -> Input(argv[1]);
	Image *img = new Image(bmp -> GetH(), bmp -> GetW());
	for (int i = 0; i < bmp -> GetH(); ++ i)
		for (int j = 0; j < bmp -> GetW(); ++ j)
			img -> setColor(i, j, bmp -> GetColor(i, j));
	img -> dump(argv[2]);
	delete bmp;
	delete img;
	return 0;
}

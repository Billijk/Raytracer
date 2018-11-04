#include "scene.h"
#include "ray.h"
#include "image.h"
#include "bmp.h"
#include "color.h"
#include "vector3.h"
#include <ctime>
#include <thread>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <string>
#include <map>

const float EPS = 1e-3;
const int threads = 8;

using namespace std;

struct dtype {
	int i;
	string str;
	pair<int, int> p;
	dtype() {}
	dtype(int _i): i(_i) {}
	dtype(string _s): str(_s) {}
	dtype(pair<int, int> _p): p(_p) {}
};

map<string, dtype> parseArgs(int, char**);
void printTime();
void render(Scene*, Image*, int, int, int, int, bool);
void debug_render(Scene*, pair<int, int>, bool);

int main(int argc, char** argv) {

	printTime();
	cout << "=========================================" << endl;
	
	auto args = parseArgs(argc, argv);

	// check input args
	if ((args.count("round") == 0 && args.count("debug") == 0) ||
		((args.count("debug") > 0 || args["count"].i > 0) && args.count("input") == 0)) {
		cout << "[ERROR] Invalid arguments!" << endl;
		return -1;
	}

	// Read Scene File
	Scene *s = new Scene;
	if (args.count("input") > 0)
		s -> readSceneFile(args["input"].str, (args.count("preview") > 0));

	// continue from old one
	Image *img;
	if (args.count("continue") > 0) {
		img = new Image();
		img -> load(args["continue"].str);
	} else 
		img = new Image(s -> getH(), s -> getW());

	cout << "=========================================" << endl;
	printTime();

	// Render
	if (args.count("debug") > 0) debug_render(s, args["debug"].p, (args.count("naive") == 0));
	else {
		if (args["round"].i > 0) {
			// multithreading
			vector<thread*> t;
			vector<Image*> imgs;
			int tile = s -> getW() / threads;

			cout << "start to render" << endl;
			cout << "using " << threads << " cores" << endl;

			for (int i = 0; i < threads - 1; ++ i) {
				int start = i * tile;
				int end = (i + 1) * tile;
				Image* im = new Image(s -> getH(), s -> getW());
				imgs.push_back(im);
				t.push_back(new thread(render, s, im, start, end, i, 
							args["round"].i, (args.count("naive") == 0)));
			}
			Image* im = new Image(s -> getH(), s -> getW());
			imgs.push_back(im);
			render(s, im, (threads - 1) * tile, s -> getW(), threads - 1, 
					args["round"].i, (args.count("naive") == 0));
			for (auto i: t) i -> join();
			for (auto i: imgs) img -> merge(i);

			// release memory
			for (auto i: t) delete i;
			for (auto i: imgs) delete i;
		}

		// output to file
		if (args.count("output") > 0) {
			cout << "Output to " << args["output"].str << endl;
			img -> dump(args["output"].str);
		}

		// output to img
		if (args.count("bmp") > 0) {
			cout << "Generate bmp file." << endl;
			Bmp *bmp = img -> toBmp();
			bmp -> Output(args["bmp"].str);
			delete bmp;
		}

		cout << "finished." << endl;
	}
	printTime();

	delete img;
	delete s;
	return 0;
}

map<string, dtype> parseArgs(int argc, char** argstr) {
	map<string, dtype> args;
	for (int i = 1; i < argc; ++ i) {
		if (strcmp(argstr[i], "-p") == 0) {
			args["preview"] = dtype(1);
			args["naive"] = dtype(1);
		} else if (strcmp(argstr[i], "-n") == 0) {
			int r;
			sscanf(argstr[++i], "%d", &r);
			args["round"] = dtype(r);
		} else if (strcmp(argstr[i], "-c") == 0) {
			args["continue"] = dtype(string(argstr[++ i]));
		} else if (strcmp(argstr[i], "-i") == 0) {
			args["input"] = dtype(string(argstr[++ i]));
		} else if (strcmp(argstr[i], "-o") == 0) {
			args["output"] = dtype(string(argstr[++ i]));
		} else if (strcmp(argstr[i], "-na") == 0) {
			args["naive"] = dtype(1);
		} else if (strcmp(argstr[i], "-b") == 0) {
			args["bmp"] = dtype(string(argstr[++ i]));
		} else if (strcmp(argstr[i], "-d") == 0) {
			int x, y;
			sscanf(argstr[++i], "%d", &x);
			sscanf(argstr[++i], "%d", &y);
			args["debug"] = dtype(pair<int, int>(x, y));
		}
	}
	return args;
}

void printTime() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[90];
	time (&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer,90,"Now time: %d-%m-%Y %I:%M:%S",timeinfo);
	string str = string(buffer);
	cout << str << endl;
}

void render(Scene *s, Image* im, int start, int end, int part, int r, bool pt) {
	int H = s -> getH();
	while (r --) {
		for (int i = 0; i < H; ++ i)
			for (int j = start; j < end; ++ j) {
				Ray *ray = s -> getRay(i, j);
				Color c = ray -> rayTracing(s, 0, pt) + s -> getBackgroundColor();
				im -> setColor(i, j, c);
				delete ray;
			}
	}
}

void debug_render(Scene *s, pair<int, int> p, bool pt) {
	int x = p.first, y = p.second;
	Ray *ray = s -> getRay(x, y);
	Color c = ray -> rayTracing(s, 0, pt) + s -> getBackgroundColor();
	cout << x << " " << y << " " << c.r << " " << c.g << " " << c.b << endl;
	delete ray;
}

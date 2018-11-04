#include "scene.h"
#include "material.h"
#include "vector3.h"
#include "object.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;
using namespace tinyxml2;

Scene::~Scene() {
	if (camera) delete camera;
	delete kdtree;
}

void Scene::parseCamera(XMLElement* cameraEle) {
	float cx, cy, cz, dx, dy, dz, dd, du, a, f;
	cameraEle -> FirstChildElement("pos") -> FirstChildElement("x") -> QueryFloatText(&cx);
	cameraEle -> FirstChildElement("pos") -> FirstChildElement("y") -> QueryFloatText(&cy);
	cameraEle -> FirstChildElement("pos") -> FirstChildElement("z") -> QueryFloatText(&cz);
	cameraEle -> FirstChildElement("dir") -> FirstChildElement("x") -> QueryFloatText(&dx);
	cameraEle -> FirstChildElement("dir") -> FirstChildElement("y") -> QueryFloatText(&dy);
	cameraEle -> FirstChildElement("dir") -> FirstChildElement("z") -> QueryFloatText(&dz);
	cameraEle -> FirstChildElement("dist") -> QueryFloatText(&dd);
	cameraEle -> FirstChildElement("unit") -> QueryFloatText(&du);
	cameraEle -> FirstChildElement("aperture") -> QueryFloatText(&a);
	cameraEle -> FirstChildElement("focal") -> QueryFloatText(&f);
	camera = new Camera(Vector3(cx, cy, cz), Vector3(dx, dy, dz), dd, f, du, a, H, W);
}

void Scene::parseLightSources(XMLElement* lightsEle) {
	char type[10];
	string texPath;
	float x, y, z, r, g, b, tu, tv;
	Object* light;
	Material* mat;
	XMLElement* l = lightsEle -> FirstChildElement();
	while (l) {
		
		strcpy(type, l -> FirstChildElement("type") -> GetText());

		l -> FirstChildElement("color") -> FirstChildElement("r") -> QueryFloatText(&r);
		l -> FirstChildElement("color") -> FirstChildElement("g") -> QueryFloatText(&g);
		l -> FirstChildElement("color") -> FirstChildElement("b") -> QueryFloatText(&b);
		XMLElement* tex = l -> FirstChildElement("texture");
		if (tex) texPath = tex -> GetText();
		// for planes: texture size
		XMLElement* texU = l -> FirstChildElement("texU");
		if (texU) texU -> QueryFloatText(&tu);
		XMLElement* texV = l -> FirstChildElement("texV");
		if (texV) texV -> QueryFloatText(&tv);

		mat = new Material();	
		mat -> color = Color(r, g, b);
		mat -> setTexture(texPath, tu, tv);

		// plane light
		if (type[0] == 'P') {
			float ux, uy, uz, dx, dy, dz;
			l -> FirstChildElement("norm") -> FirstChildElement("x") -> QueryFloatText(&x);
			l -> FirstChildElement("norm") -> FirstChildElement("y") -> QueryFloatText(&y);
			l -> FirstChildElement("norm") -> FirstChildElement("z") -> QueryFloatText(&z);
			l -> FirstChildElement("ul") -> FirstChildElement("x") -> QueryFloatText(&ux);
			l -> FirstChildElement("ul") -> FirstChildElement("y") -> QueryFloatText(&uy);
			l -> FirstChildElement("ul") -> FirstChildElement("z") -> QueryFloatText(&uz);
			l -> FirstChildElement("dr") -> FirstChildElement("x") -> QueryFloatText(&dx);
			l -> FirstChildElement("dr") -> FirstChildElement("y") -> QueryFloatText(&dy);
			l -> FirstChildElement("dr") -> FirstChildElement("z") -> QueryFloatText(&dz);
			light = new Plane(Vector3(ux, uy, uz), Vector3(dx, dy, dz), Vector3(x, y, z), true);
		}
		else if (type[0] == 'S') {
			float w;
			// sphere light
			l -> FirstChildElement("center") -> FirstChildElement("x") -> QueryFloatText(&x);
			l -> FirstChildElement("center") -> FirstChildElement("y") -> QueryFloatText(&y);
			l -> FirstChildElement("center") -> FirstChildElement("z") -> QueryFloatText(&z);
			l -> FirstChildElement("r") -> QueryFloatText(&w);
			light = new Sphere(Vector3(x, y, z), w, true);
		}
		else if (type[0] == 'T') {
			float ax, ay, az, bx, by, bz, cx, cy, cz;
			l -> FirstChildElement("a") -> FirstChildElement("x") -> QueryFloatText(&ax);
			l -> FirstChildElement("a") -> FirstChildElement("y") -> QueryFloatText(&ay);
			l -> FirstChildElement("a") -> FirstChildElement("z") -> QueryFloatText(&az);
			l -> FirstChildElement("b") -> FirstChildElement("x") -> QueryFloatText(&bx);
			l -> FirstChildElement("b") -> FirstChildElement("y") -> QueryFloatText(&by);
			l -> FirstChildElement("b") -> FirstChildElement("z") -> QueryFloatText(&bz);
			l -> FirstChildElement("c") -> FirstChildElement("x") -> QueryFloatText(&cx);
			l -> FirstChildElement("c") -> FirstChildElement("y") -> QueryFloatText(&cy);
			l -> FirstChildElement("c") -> FirstChildElement("z") -> QueryFloatText(&cz);
			light = new Triangle(Vector3(ax, ay, az), Vector3(bx, by, bz), Vector3(cx, cy, cz), true);
		}

		light -> setMaterial(mat);
		kdtree -> getRoot() -> addObject(light);
		lights.push_back(light);
		l = l -> NextSiblingElement();
	}
}

void Scene::parseObjects(XMLElement* objsEle, bool preview) {
	char type[10];
	string texPath;
	float x, y, z, w, r, g, b, rr, gg, bb, d, s, r1, r2, rid;
	float tu, tv;
	Material* mat;
	XMLElement* o = objsEle -> FirstChildElement();
	while (o) {
		texPath.clear();
		rr = gg = bb = d = s = r1 = r2 = rid = 0;
		tu = tv = 1;

		strcpy(type, o -> FirstChildElement("type") -> GetText());

		o -> FirstChildElement("color") -> FirstChildElement("r") -> QueryFloatText(&r);
		o -> FirstChildElement("color") -> FirstChildElement("g") -> QueryFloatText(&g);
		o -> FirstChildElement("color") -> FirstChildElement("b") -> QueryFloatText(&b);

		XMLElement* diff = o -> FirstChildElement("diff");
		XMLElement* spec = o -> FirstChildElement("spec");
		XMLElement* refl = o -> FirstChildElement("refl");
		XMLElement* refr = o -> FirstChildElement("refr");
		if (diff) diff -> QueryFloatText(&d);
		if (spec) spec -> QueryFloatText(&s);
		if (refl) refl -> QueryFloatText(&r1);
		if (refr) {
			refr -> QueryFloatText(&r2);
			o -> FirstChildElement("absorbcolor") -> FirstChildElement("r") -> QueryFloatText(&rr);
			o -> FirstChildElement("absorbcolor") -> FirstChildElement("g") -> QueryFloatText(&gg);
			o -> FirstChildElement("absorbcolor") -> FirstChildElement("b") -> QueryFloatText(&bb);
			o -> FirstChildElement("rindex") -> QueryFloatText(&rid);
		}

		XMLElement* tex = o -> FirstChildElement("texture");
		if (tex) texPath = tex -> GetText();
		// for planes: texture size
		XMLElement* texU = o -> FirstChildElement("texU");
		if (texU) texU -> QueryFloatText(&tu);
		XMLElement* texV = o -> FirstChildElement("texV");
		if (texV) texV -> QueryFloatText(&tv);

		mat = new Material();
		mat -> color = Color(r, g, b);
		mat -> absorb = Color(rr, gg, bb);
		mat -> diff = d;
		mat -> spec = s;
		mat -> refl = preview ? 0 : r1;
		mat -> refr = preview ? 0 : r2;
		mat -> r_index = rid;
		mat -> setTexture(texPath, tu, tv);
		
		if (type[0] == 'P') {
			float ux, uy, uz, dx, dy, dz;
			o -> FirstChildElement("norm") -> FirstChildElement("x") -> QueryFloatText(&x);
			o -> FirstChildElement("norm") -> FirstChildElement("y") -> QueryFloatText(&y);
			o -> FirstChildElement("norm") -> FirstChildElement("z") -> QueryFloatText(&z);
			o -> FirstChildElement("ul") -> FirstChildElement("x") -> QueryFloatText(&ux);
			o -> FirstChildElement("ul") -> FirstChildElement("y") -> QueryFloatText(&uy);
			o -> FirstChildElement("ul") -> FirstChildElement("z") -> QueryFloatText(&uz);
			o -> FirstChildElement("dr") -> FirstChildElement("x") -> QueryFloatText(&dx);
			o -> FirstChildElement("dr") -> FirstChildElement("y") -> QueryFloatText(&dy);
			o -> FirstChildElement("dr") -> FirstChildElement("z") -> QueryFloatText(&dz);
			Plane *p = new Plane(Vector3(ux, uy, uz), Vector3(dx, dy, dz), Vector3(x, y, z));
			p -> setMaterial(mat);
			kdtree -> getRoot() -> addObject(p);

		} else if (type[0] == 'S') {
			// parse sphere
			o -> FirstChildElement("center") -> FirstChildElement("x") -> QueryFloatText(&x);
			o -> FirstChildElement("center") -> FirstChildElement("y") -> QueryFloatText(&y);
			o -> FirstChildElement("center") -> FirstChildElement("z") -> QueryFloatText(&z);
			o -> FirstChildElement("r") -> QueryFloatText(&w);
			Sphere *s = new Sphere(Vector3(x, y, z), w);
			s -> setMaterial(mat);
			kdtree -> getRoot() -> addObject(s);
		} else if (type[0] == 'T') {
			//parse triangle
			float ax, ay, az, bx, by, bz, cx, cy, cz;
			o -> FirstChildElement("a") -> FirstChildElement("x") -> QueryFloatText(&ax);
			o -> FirstChildElement("a") -> FirstChildElement("y") -> QueryFloatText(&ay);
			o -> FirstChildElement("a") -> FirstChildElement("z") -> QueryFloatText(&az);
			o -> FirstChildElement("b") -> FirstChildElement("x") -> QueryFloatText(&bx);
			o -> FirstChildElement("b") -> FirstChildElement("y") -> QueryFloatText(&by);
			o -> FirstChildElement("b") -> FirstChildElement("z") -> QueryFloatText(&bz);
			o -> FirstChildElement("c") -> FirstChildElement("x") -> QueryFloatText(&cx);
			o -> FirstChildElement("c") -> FirstChildElement("y") -> QueryFloatText(&cy);
			o -> FirstChildElement("c") -> FirstChildElement("z") -> QueryFloatText(&cz);
			Triangle* t = new Triangle(Vector3(ax, ay, az), Vector3(bx, by, bz), Vector3(cx, cy, cz), true);
			t -> setMaterial(mat);
			kdtree -> getRoot() -> addObject(t);
		} else if (type[0] == 'O') {
			//parse other objects (with an .obj file)
			string objFilePath;
			o -> FirstChildElement("pos") -> FirstChildElement("x") -> QueryFloatText(&x);
			o -> FirstChildElement("pos") -> FirstChildElement("y") -> QueryFloatText(&y);
			o -> FirstChildElement("pos") -> FirstChildElement("z") -> QueryFloatText(&z);
			o -> FirstChildElement("scale") -> QueryFloatText(&r);
			objFilePath = o -> FirstChildElement("obj") -> GetText();
			parseObjFiles(objFilePath, Vector3(x, y, z), r, mat, preview);
		}
		o = o -> NextSiblingElement();
	}
}

void Scene::parseObjFiles(string file, Vector3 center, float scale, Material* mat, bool preview) {
	fstream fin(file, fstream::in);
	string str;
	vector<Vector3> vertices;
	vector<Vector3> vnorms;
	float xl = 1e100, xh = -1e100, 
		  yl = 1e100, yh = -1e100,
		  zl = 1e100, zh = -1e100;
	while (fin >> str) {
		if (str == "v") {
			float x, y, z;
			fin >> x >> y >> z;
			vertices.push_back(Vector3(x, y, z) * scale + center);
			if (x < xl) xl = x;
			if (x > xh) xh = x;
			if (y < yl) yl = y;
			if (y > yh) yh = y;
			if (z < zl) zl = z;
			if (z > zh) zh = z;
		} else if (str == "vn") {
			float x, y, z;
			fin >> x >> y >> z;
			vnorms.push_back(Vector3(x, y, z));
		} else if (str == "f" && !preview) {
			char sa[20], sb[20], sc[20];
			int a, b, c, na, nb, nc;
			fin >> sa >> sb >> sc;
			sscanf(sa, "%d//%d", &a, &na);
			sscanf(sb, "%d//%d", &b, &nb);
			sscanf(sc, "%d//%d", &c, &nc);
			Triangle* t;
			if (vnorms.size() > 0)
				t = new Triangle(vertices[a - 1], vertices[b - 1], vertices[c - 1],
						vnorms[na - 1], vnorms[nb - 1], vnorms[nc - 1]);
			else
				t = new Triangle(vertices[a - 1], vertices[b - 1], vertices[c - 1]);
			Material* m = new Material(*mat);
			t -> setMaterial(m);
			kdtree -> getRoot() -> addObject(t);
		}
	}
	if (preview) {
		xl = xl * scale + center.x;
		xh = xh * scale + center.x;
		yl = yl * scale + center.y;
		yh = yh * scale + center.y;
		zl = zl * scale + center.z;
		zh = zh * scale + center.z;
		cout << "-- File Name: " << file << endl;
		cout << "-- Boundaries: " << endl;
		cout << "-- x: " << xl << " " << xh << endl;
		cout << "-- y: " << yl << " " << yh << endl;
		cout << "-- z: " << zl << " " << zh << endl;
		auto addPlane = [&](Vector3 a, Vector3 b, Vector3 c, Vector3 d) {
			Triangle *t = new Triangle(c, b, a);
			Material *m = new Material(*mat);
			t -> setMaterial(m);
			kdtree -> getRoot() -> addObject(t);
			t = new Triangle(d, c, a);
			m = new Material(*mat);
			t -> setMaterial(m);
			kdtree -> getRoot() -> addObject(t);
		};
		addPlane(Vector3(xl, yl, zh), Vector3(xl, yl, zl), 
				Vector3(xh, yl, zl), Vector3(xh, yl, zh));
		addPlane(Vector3(xl, yh, zh), Vector3(xl, yh, zl), 
				Vector3(xh, yh, zl), Vector3(xh, yh, zh));
		addPlane(Vector3(xl, yl, zh), Vector3(xl, yl, zl), 
				Vector3(xl, yh, zl), Vector3(xl, yh, zh));
		addPlane(Vector3(xh, yl, zh), Vector3(xh, yl, zl), 
				Vector3(xh, yh, zl), Vector3(xh, yh, zh));
		addPlane(Vector3(xl, yl, zh), Vector3(xh, yl, zh), 
				Vector3(xh, yh, zh), Vector3(xl, yh, zh));
		addPlane(Vector3(xl, yl, zl), Vector3(xh, yl, zl), 
				Vector3(xh, yh, zl), Vector3(xl, yh, zl));
	}
	delete mat;
}

void Scene::readSceneFile(string file, bool preview) {

	cout << "reading scene file: " << file << endl;

	XMLDocument doc;
	doc.LoadFile(file.c_str());
	XMLElement* sceneEle = doc.FirstChildElement("scene");

	// get scene property
	sceneEle -> QueryIntAttribute("height", &H);
	sceneEle -> QueryIntAttribute("width", &W);

	// read scene size
	float xl, xh, yl, yh, zl, zh;
	XMLElement* size = sceneEle -> FirstChildElement("size");
	size -> FirstChildElement("xl") -> QueryFloatText(&xl);
	size -> FirstChildElement("xh") -> QueryFloatText(&xh);
	size -> FirstChildElement("yl") -> QueryFloatText(&yl);
	size -> FirstChildElement("yh") -> QueryFloatText(&yh);
	size -> FirstChildElement("zl") -> QueryFloatText(&zl);
	size -> FirstChildElement("zh") -> QueryFloatText(&zh);
	kdtree = new KDTree(xl, xh, yl, yh, zl, zh);

	// read background color
	float r, g, b;
	XMLElement* bg = sceneEle -> FirstChildElement("background");
	bg -> FirstChildElement("r") -> QueryFloatText(&r);
	bg -> FirstChildElement("g") -> QueryFloatText(&g);
	bg -> FirstChildElement("b") -> QueryFloatText(&b);
	backgroundColor = Color(r, g, b);
	
	parseCamera(sceneEle -> FirstChildElement("camera"));
	parseLightSources(sceneEle -> FirstChildElement("lights"));
	parseObjects(sceneEle -> FirstChildElement("objects"), preview);
	
	cout << "finished reading scene file" << endl;

	// build KD Tree
	cout << "start building KD tree" << endl;
	kdtree -> buildTree(kdtree -> getRoot());
	cout << "finished! (node count: " << TreeNode::nodecnt << ")" << endl;

}

Ray* Scene::getRay(int x, int y) {
	auto vecs = camera -> getRay(y, x);
	return new Ray(vecs.first, vecs.second);
}

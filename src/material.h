#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include "color.h"
#include "image.h"
#include <string>

struct Material {
	Color color, absorb;
	float diff, spec;
	float refl, refr;
	float r_index;

	Image* texture;
	float tu, tv;

	void setTexture(std::string path, float _tu, float _tv) {
		if (path.empty()) texture = NULL;
		else {
			texture = new Image();
			texture -> load(path);
			tu = _tu, tv = _tv;
		}
	}

	Material() {}
	Material(const Material& m) {
		color = m.color;
		absorb = m.absorb;
		diff = m.diff;
		spec = m.spec;
		refl = m.refl;
		refr = m.refr;
		r_index = m.r_index;
		texture = m.texture;
		tu = m.tu;
		tv = m.tv;
	}
};

#endif

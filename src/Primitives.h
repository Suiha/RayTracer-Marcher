#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/intersect.hpp"
#include <glm/gtc/matrix_transform.hpp>


//  General Purpose Ray class 
class Ray {
public:
	Ray(glm::vec3 p, glm::vec3 d) { this->p = p; this->d = d; }
	void draw(float t) { ofDrawLine(p, p + t * d); }

	glm::vec3 evalPoint(float t) {
		return (p + t * d);
	}

	glm::vec3 p, d; // direction (d) must be normalized
};


//  Base class for any renderable object in the scene
class SceneObject {
public:

	virtual void draw() = 0;
	virtual bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { cout << "SceneObject::intersect" << endl; return false; }
	virtual glm::vec3 getNormal(const glm::vec3& p) { return glm::vec3(0, 0, 0); }
	virtual float sdf(const glm::vec3& p) = 0;

	// gui funcions
	virtual void setupGUI() = 0;
	virtual void updateGUI() = 0;

	// currently just used for rendercam
	glm::mat4 getMatrix() {
		glm::mat4 T = glm::translate(glm::mat4(1.0), glm::vec3(position));
		glm::mat4 Rx = glm::rotate(glm::mat4(1.0), glm::radians(rotation.x), glm::vec3(1, 0, 0));
		glm::mat4 Ry = glm::rotate(glm::mat4(1.0), glm::radians(rotation.y), glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1.0), glm::radians(rotation.z), glm::vec3(0, 0, 1));
		glm::mat4 S = glm::scale(glm::mat4(1.0), scale);      // added this scale if you want to change size of object
		return T * Rz * Ry * Rx * S;
	}
	void setPosition(glm::vec3 p) {
		position = p;
		transform = getMatrix();
	}
	void resetTransform() {
		transform = getMatrix();
	}

	// any data common to all scene objects goes here
	string name;
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec3 rotation = glm::vec3(0, 0, 0);    // degrees
	glm::vec3 scale = glm::vec3(1, 1, 1);
	glm::mat4 transform; // only used for rendercam
	bool isSelectable = false;
	bool bSelected = false;

	// gui elements & functions
	ofxPanel gui;
	ofParameter<glm::vec3> objPos;
	ofxLabel texture;
	ofParameter<int> nTiles;

	// material properties
	ofColor diffuseColor = ofColor::lightGray;   
	ofColor specularColor = ofColor::white;

	// texture objects & functions
	void getTextureCoords(glm::vec3 p, float& u, float& v) {}
	string textureName = "None";
	ofImage diffuseMap;
	ofImage specularMap;
	int numTiles = 1;
};


// general light class for illuminating scene
class Light : public SceneObject {
public:
	Light(glm::vec3 p, float i) {
		position = p;
		intensity = i;
	}
	Light() { intensity = 0; }

	void setupGUI() {}
	void updateGUI() {}

	void draw() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	float sdf(const glm::vec3& p) { return 0; }

	// virtual functions - must be overloaded
	virtual int getRaySamples(glm::vec3 p, glm::vec3 norm) = 0;

	float intensity;
	vector<Ray> samples;
	vector<glm::vec3> samplesPos;

	ofParameter<float> lightIntensity;
};


// representation of ambient light
class AmbientLight : public Light {
public:
	AmbientLight(float i) {
		intensity = i;
	}
	AmbientLight() {}

	void setupGUI() {}
	void updateGUI() {}
	
	void draw() {}
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) { return false; }
	float sdf(glm::vec3 p) {}
	int getRaySamples(glm::vec3 p, glm::vec3 norm) {
		return 0;
	}
};


// point light = a point with one light ray to a given point
class PointLight : public Light {
public:
	PointLight(glm::vec3 p, float i) {
		name = string("Point Light ") + to_string(PointLight::ext++);
		position = p;
		intensity = i;
		isSelectable = true;
		setupGUI();
	}

	PointLight(glm::vec3 p) {
		name = string("Point Light ") + to_string(PointLight::ext++);
		position = p;
		intensity = 10.0;
		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(lightIntensity.set("Intensity", intensity, 0, 500));
		gui.add(objPos.set("Position", position, glm::vec3(-50, 0, -50),
			glm::vec3(50, 50, 50)));
	}

	void updateGUI() {
		intensity = lightIntensity;
		position = objPos;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (glm::intersectRaySphere(ray.p, ray.d, position, 0.2, point, normal));
	}
	float sdf(const glm::vec3& p) { return 0; }
	int getRaySamples(glm::vec3 p, glm::vec3 norm);

	static int PointLight::ext;
};


// area light = grid with "infinite" light rays
class AreaLight : public Light {
public:
	AreaLight(glm::vec3 p, float i, int w, int h, int nDW, int nDH, int samples) {
		name = string("Area Light ") + to_string(AreaLight::ext++);
		position = p;
		intensity = i;
		width = w;
		height = h;
		nDivsWidth = nDW;
		nDivsHeight = nDH;
		nSamples = samples;

		isSelectable = true;
		setupGUI();
	}

	AreaLight(glm::vec3 p) {
		name = string("Area Light ") + to_string(AreaLight::ext++);
		position = p;
		intensity = 500;
		width = 10;
		height = 10;
		nDivsWidth = 5;
		nDivsHeight = 5;
		nSamples = 1;

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(lightIntensity.set("Intensity", intensity, 0, 1000));
		gui.add(objPos.set("Position", position, glm::vec3(-50, 0, -50),
			glm::vec3(50, 50, 50)));
		gui.add(alWidth.set("Area Light Width", width, 0, 10));
		gui.add(alHeight.set("Area Light Height", height, 0, 10));
		gui.add(divsWidth.set("# Subdivisions (Width)", nDivsWidth, 0, 20));
		gui.add(divsHeight.set("# Subdivisions (Height)", nDivsHeight, 0, 20));
		gui.add(numSamples.set("# Light Samples / Cell", nSamples, 1, 5));
	}

	void updateGUI() {
		intensity = lightIntensity;
		position = objPos;
		width = alWidth;
		height = alHeight;
		nDivsWidth = divsWidth;
		nDivsHeight = divsHeight;
		nSamples = numSamples;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	float sdf(const glm::vec3& p) { return 0; }
	int getRaySamples(glm::vec3 p, glm::vec3 norm);

	static int AreaLight::ext;

	float width, height;			// overall width and height of the grid    (default (5 x 5);
	glm::vec3 position;				// x, y, z position of the grid's origin in space
	int nDivsWidth, nDivsHeight;	// number of subdivisions of grid (default 10x10): width = vertical, height = horizontal
	int nSamples;					// number of samples per grid cell (default = 1)

	ofParameter<float> alWidth, alHeight;
	ofParameter<int> divsWidth, divsHeight, numSamples;
};


//  General purpose sphere  (assume parametric)
class Sphere : public SceneObject {
public:
	Sphere(glm::vec3 p, float r, ofColor diffuse = ofColor::white) {
		name = string("Sphere ") + to_string(Sphere::ext++);
		position = p;
		radius = r;
		diffuseColor = diffuse;

		isSelectable = true;
		setupGUI();
	}

	Sphere() {
		name = string("Sphere ") + to_string(Sphere::ext++);
		isSelectable = true; 
		setupGUI(); 
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(sphereRadius.set("Radius", radius, 1, 10));
		gui.add(sphereColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));

		gui.add(texture.setup("Texture: " + string(textureName)));
		gui.add(nTiles.set("Texture Tiles", numTiles, 1, 10));
	}

	void updateGUI() {
		position = objPos;
		radius = sphereRadius;
		diffuseColor = sphereColor;
		
		texture = "Texture: " + string(textureName);
		numTiles = nTiles;
	}
	
	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal) {
		return (intersectRaySphere(ray.p, ray.d, position, radius, point, normal));
	}
	glm::vec3 getNormal(const glm::vec3& p) {
		return glm::normalize(glm::vec3(p - position));
	}
	float sdf(const glm::vec3& p) {
		float x = pow(p.x, 2);
		float y = pow(p.y, 2);
		float z = pow(p.z, 2);
		float length = std::sqrt(x + y + z);

		return length - radius;
	}
	void getTextureCoords(glm::vec3 p, float& u, float& v);

	static int Sphere::ext; // keep track of # of spheres created
	float radius = 1.0;

	ofParameter<float> sphereRadius;
	ofParameter<ofColor> sphereColor;
};


//  General purpose plane 
class Plane : public SceneObject {
public:
	Plane(glm::vec3 p, glm::vec3 n, ofColor diffuse = ofColor::white, float w = 20, float h = 20) {
		name = string("Plane ") + to_string(Plane::ext++);
		position = p; 
		normal = n;
		width = w;
		height = h;
		diffuseColor = diffuse;
		
		// plane by default faces (0, 0, 1), rotate according to normal
		if (normal.z != -1) {
			plane.rotateDeg(90, -normal.y, abs(normal.x), 0);
		}
		else {
			plane.rotateDeg(180, 1, 0, 0);
		}
		
		/*if (normal == glm::vec3(0, 1, 0))
			plane.rotateDeg(-90, 1, 0, 0);
		else if (normal == glm::vec3(0, -1, 0))
			plane.rotateDeg(90, 1, 0, 0);
		else if (normal == glm::vec3(1, 0, 0))
			plane.rotateDeg(90, 0, 1, 0);
		else if (normal == glm::vec3(-1, 0, 0))
			plane.rotateDeg(-90, 0, 1, 0);
		else if (normal == glm::vec3(0, 0, -1))
			plane.rotateDeg(180, 1, 0, 0);*/

		isSelectable = true;
		setupGUI();
	}

	Plane() {
		name = string("Plane ") + to_string(Plane::ext++);
		normal = glm::vec3(0, 1, 0);
		plane.rotateDeg(90, 1, 0, 0);

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(planeWidth.set("Width", width, 0.1, 50));
		gui.add(planeHeight.set("Height", height, 0.1, 50));

		faceUp.addListener(this, &Plane::upNormal);
		faceDown.addListener(this, &Plane::downNormal);
		faceLeft.addListener(this, &Plane::leftNormal);
		faceRight.addListener(this, &Plane::rightNormal);
		faceForward.addListener(this, &Plane::forwardNormal);
		faceBackward.addListener(this, &Plane::backwardNormal);

		normalOptions.setName("Plane Normal Options");
		normalOptions.add(faceUp.set("Normal: (0, 1, 0)", (normal == glm::vec3(0, 1, 0)) ? true : false));
		normalOptions.add(faceDown.set("Normal: (0, -1, 0)", (normal == glm::vec3(0, -1, 0)) ? true : false));
		normalOptions.add(faceLeft.set("Normal: (-1, 0, 0)", (normal == glm::vec3(-1, 0, 0)) ? true : false));
		normalOptions.add(faceRight.set("Normal: (1, 0, 0)", (normal == glm::vec3(1, 0, 0)) ? true : false));
		normalOptions.add(faceForward.set("Normal: (0, 0, 1)", (normal == glm::vec3(0, 0, 1)) ? true : false));
		normalOptions.add(faceBackward.set("Normal: (0, 0, -1)", (normal == glm::vec3(0, 0, -1)) ? true : false));
		normalOptions.add(planeColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));
		gui.add(normalOptions);

		gui.add(texture.setup("Texture: " + string(textureName)));
		gui.add(nTiles.set("Texture Tiles", numTiles, 1, 10));
	}

	void updateGUI() {
		position = objPos;
		width = planeWidth;
		height = planeHeight;
		diffuseColor = planeColor;

		texture = "Texture: " + string(textureName);
		numTiles = nTiles;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);
	glm::vec3 getNormal(const glm::vec3& p) { return this->normal; }

	// currently renders plane as an infinite plane
	float sdf(const glm::vec3& p) {
		float h = 0;
		glm::vec3 b = glm::vec3(0, 0, 0);
		float inf = std::numeric_limits<float>::infinity();

		// alter "height" of plane and border box based on normal
		if (normal.x != 0) {
			h = p.x - position.x;
			b = glm::vec3(10, height, width);
		}
		else if (normal.y != 0) {
			h = p.y - position.y;
			b = glm::vec3(width, 10, height);
		}
		else if (normal.z != 0) {
			h = p.z - position.z;
			b = glm::vec3(width, height, 10);
		}

		float infPlane = dot(p, normal) + h;
		float border = sdBox(p, b / 2);

		return max(infPlane, -border);
	}

	// box sdf function
	float sdBox(const glm::vec3& p, glm::vec3 b) {
		glm::vec3 q = abs(p) - b;
		return length(max(q, glm::vec3(0, 0, 0)) + min(max(q.x, max(q.y, q.z)), 0.0f));
	}

	void getTextureCoords(glm::vec3 p, float& u, float& v);

	// listener functions for changing normal
	void upNormal(bool& val);
	void downNormal(bool& val);
	void leftNormal(bool& val);
	void rightNormal(bool& val);
	void forwardNormal(bool& val);
	void backwardNormal(bool& val);

	ofPlanePrimitive plane;
	glm::vec3 normal;

	static int Plane::ext;
	float width = 20;
	float height = 20;

	ofParameter<float> planeWidth, planeHeight;
	ofParameter<ofColor> planeColor;
	ofParameterGroup normalOptions;
	ofParameter<bool> faceUp, faceDown, faceLeft, faceRight, faceForward, faceBackward;
};


// view plane for render camera
class  ViewPlane : public Plane {
public:
	ViewPlane(glm::vec2 p0, glm::vec2 p1) { min = p0; max = p1; }

	ViewPlane() {                         // create reasonable defaults (6x4 aspect)
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		position = glm::vec3(0, 0, 5);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getAspect() { return width() / height(); }

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void setupGUI() {}
	void updateGUI() {}

	void draw() {
		ofDrawRectangle(glm::vec3(min.x, min.y, position.z), width(), height());
	}

	float width() { return (max.x - min.x); }
	float height() { return (max.y - min.y); }

	// some convenience methods for returning the corners
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	//  To define an infinite plane, we just need a point and normal.
	//  The ViewPlane is a finite plane so we need to define the boundaries.
	//  We will define this in terms of min, max  in 2D.  
	//  (in local 2D space of the plane)
	//  ultimately, will want to locate the ViewPlane with RenderCam anywhere
	//  in the scene, so it is easier to define the View rectangle in a local'
	//  coordinate system.
	//
	glm::vec2 min, max;
};


// menger sponge class
class MengerSponge : public SceneObject {
public:
	MengerSponge(glm::vec3 pos, ofColor diffuse, int lvl, float size) {
		name = string("Mandelbrot ") + to_string(MengerSponge::ext++);
		position = pos;
		diffuseColor = diffuse;
		dimensions = glm::vec3(size);
		level = lvl;

		// faces for intersect function
		faces.push_back( new Plane(position - glm::vec3(0, size / 2, 0), glm::vec3(0, -1, 0), diffuseColor, size, size) );	// bot
		faces.push_back( new Plane(position + glm::vec3(0, size / 2, 0), glm::vec3(0, 1, 0), diffuseColor, size, size) );	// top
		faces.push_back( new Plane(position - glm::vec3(size / 2, 0, 0), glm::vec3(-1, 0, 0), diffuseColor, size, size) );	// left
		faces.push_back( new Plane(position + glm::vec3(size / 2, 0, 0), glm::vec3(1, 0, 0), diffuseColor, size, size) );	// right
		faces.push_back( new Plane(position - glm::vec3(0, 0, size / 2), glm::vec3(0, 0, -1), diffuseColor, size, size) );	// back
		faces.push_back( new Plane(position + glm::vec3(0, 0, size / 2), glm::vec3(0, 0, 1), diffuseColor, size, size) );	// front
		for (Plane* f : faces) {
			f->isSelectable = false;
		}

		isSelectable = true;
		setupGUI();
	}

	MengerSponge() {
		name = string("Mandelbrot ") + to_string(MengerSponge::ext++);
		position = glm::vec3(0, 0, 0);;
		diffuseColor = ofColor::white;
		float size = 2;
		dimensions = glm::vec3(size);
		level = 1;

		// faces for intersect function
		faces.push_back(new Plane(position - glm::vec3(0, 1, 0), glm::vec3(0, -1, 0), diffuseColor, size, size));	// bot
		faces.push_back(new Plane(position + glm::vec3(0, 1, 0), glm::vec3(0, 1, 0), diffuseColor, size, size));	// top
		faces.push_back(new Plane(position - glm::vec3(1, 0, 0), glm::vec3(-1, 0, 0), diffuseColor, size, size));	// left
		faces.push_back(new Plane(position + glm::vec3(1, 0, 0), glm::vec3(1, 0, 0), diffuseColor, size, size));	// right
		faces.push_back(new Plane(position - glm::vec3(0, 0, 1), glm::vec3(0, 0, -1), diffuseColor, size, size));	// back
		faces.push_back(new Plane(position + glm::vec3(0, 0, 1), glm::vec3(0, 0, 1), diffuseColor, size, size));	// front
		for (Plane* f : faces) {
			f->isSelectable = false;
		}

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(cubeSize.set("Cube Size", dimensions.x, 1, 10));
		gui.add(msLevel.set("Level", level, 1, 5));
		gui.add(msColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));
	}

	void updateGUI() {
		position = objPos;
		dimensions = glm::vec3(cubeSize);

		// plane's gui is never updated, directly change its parameters
		for (Plane* f : faces) {
			f->width = cubeSize;
			f->height = cubeSize;
		}
		faces[0]->position = position - glm::vec3(0, cubeSize / 2, 0);
		faces[1]->position = position + glm::vec3(0, cubeSize / 2, 0);
		faces[2]->position = position - glm::vec3(cubeSize / 2, 0, 0);
		faces[3]->position = position + glm::vec3(cubeSize / 2, 0, 0);
		faces[4]->position = position - glm::vec3(0, 0, cubeSize / 2);
		faces[5]->position = position + glm::vec3(0, 0, cubeSize / 2);

		level = msLevel;
		diffuseColor = msColor;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);

	// code source from https://iquilezles.org/articles/menger
	float sdf(const glm::vec3& p) {

		// overall cube sdf
		float dist = sdBox(p, dimensions / 2);

		float s = 1.0;

		// iterate through cube to remove boxes
		for (int i = 0; i < level; i++) {
			
			glm::vec3 a = glm::mod((p * s), 2.0f) - 1.0f;
			s *= 3;
			glm::vec3 r = abs(1.0f - 3.0f * abs(a));

			// determine sdf of "cross" inside outer cube
			float da = max(r.x, r.y);
			float db = max(r.y, r.z);
			float dc = max(r.z, r.x);
			float c = (min(da, min(db, dc)) - 1) / s;

			// remove cross from sdf
			dist = max(dist, c);
		}

		return dist;
	}

	// box sdf function
	float sdBox(const glm::vec3& p, glm::vec3 b) {
		glm::vec3 q = abs(p) - b;
		return length(max(q, glm::vec3(0, 0, 0)) + min(max(q.x, max(q.y, q.z)), 0.0f));
	}

	glm::vec3 dimensions;
	int level;
	vector<Plane*> faces;

	ofParameter<float> cubeSize;
	ofParameter<int> msLevel;
	ofParameter<ofColor> msColor;

	static int MengerSponge::ext; // keep track of # of mandelbrots created
};

// mandelbulb class - note: currently only renders if a plane object is in the scene
class Mandelbulb : public SceneObject {
public:
	Mandelbulb(glm::vec3 pos, ofColor diffuse, int it, float pow, float bail) {
		name = string("Mandelbulb ") + to_string(Mandelbulb::ext++);
		position = pos;
		diffuseColor = diffuse;
		iterations = it;
		power = pow;
		bailout = bail;

		isSelectable = true;
		setupGUI();
	}

	Mandelbulb() {
		name = string("Mandelbulb ") + to_string(Mandelbulb::ext++);
		position = glm::vec3(0, 0, 0);
		diffuseColor = ofColor::white;
		iterations = 5;
		power = 3.0f;
		bailout = 4.0f;

		isSelectable = true;
		setupGUI();
	}

	void setupGUI() {
		gui.setup(name);
		gui.add(objPos.set("Position", position, glm::vec3(-10, -10, -10),
			glm::vec3(10, 10, 10)));
		gui.add(mbIter.set("Max Iterations", iterations, 1, 20));
		gui.add(mbPower.set("Power", power, 1, 16));
		gui.add(mbBail.set("Bailout", bailout, 1, 10));
		gui.add(mbColor.set("Diffuse Color", diffuseColor, ofColor::white, ofColor::black));
	}

	void updateGUI() {
		position = objPos;

		iterations = mbIter;
		power = mbPower;
		bailout = mbBail;
		diffuseColor = mbColor;
	}

	void draw();
	bool intersect(const Ray& ray, glm::vec3& point, glm::vec3& normal);

	// source: http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations
	float sdf(const glm::vec3& p) {
		glm::vec3 z = p;
		float dr = 1.0;
		float r = 0.0;

		for (int i = 0; i < iterations; i++) {
			r = length(z);
			if (r > bailout) break;

			// convert to polar coords
			float theta = acos(z.z / r);
			float phi = atan2(z.y, z.x);
			dr = pow(r, power - 1.0) * power * dr + 1.0;

			// scale and rotate point
			float zr = pow(r, power);
			theta = theta * power;
			phi = phi * power;

			// convert back to cartesian coords
			z = zr * glm::vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
			z += p;
		}

		return 0.5 * log(r) * r / dr;
	}

	int iterations;
	float power;
	float bailout;

	ofParameter<int> mbIter;
	ofParameter<float> mbPower, mbBail;
	ofParameter<ofColor> mbColor;

	static int Mandelbulb::ext; // keep track of # of mandelbrots created
};


//  render camera - not used
class RenderCam : public SceneObject {
public:
	RenderCam() {
		position = glm::vec3(0, 0, 20);
		aim = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		transform = getMatrix();


		// view plane parameters
		//
		min = glm::vec2(-3, -2);
		max = glm::vec2(3, 2);
		normal = glm::vec3(0, 0, 1);      // viewplane currently limited to Z axis orientation
	}

	void setAim(glm::vec3 aimPos) {
		aim = aimPos;
		transform = getMatrix();
	}

	void setupGUI() {}
	void updateGUI() {}

	// some convenience methods for returning the corners of view plane
	//
	glm::vec2 topLeft() { return glm::vec2(min.x, max.y); }
	glm::vec2 topRight() { return max; }
	glm::vec2 bottomLeft() { return min; }
	glm::vec2 bottomRight() { return glm::vec2(max.x, min.y); }

	float viewWidth() {
		return (max.x - min.x);
	}
	float viewHeight() {
		return (max.y - min.y);
	}

	glm::vec3 toWorld(float u, float v);   //   (u, v) --> (x, y, z) [ world space ]

	void drawViewPlane() {
		ofDrawRectangle(glm::vec3(min.x, min.y, -viewPlaneDist), viewWidth(), viewHeight());
	}


	void setViewSize(glm::vec2 min, glm::vec2 max) { this->min = min; this->max = max; }
	float getViewAspect() { return viewWidth() / viewHeight(); }


	Ray getRay(float u, float v);


	void draw() {
		ofPushMatrix();
		ofMultMatrix(transform);
		drawViewPlane();  // viewplane is in camera space
		ofDrawBox(glm::vec3(0, 0, 0), 1.0);
		ofPopMatrix();
		drawFrustum();    // frustum is drawn in world space
	};


	void drawFrustum();

	glm::vec3 aim;

	glm::vec2 min, max;
	glm::vec3 normal = glm::vec3(0, 0, 1);
	float viewPlaneDist = 5.0;    // distance from view plane to camera

	//RenderCam() {
	//	position = glm::vec3(0, 0, 10);
	//	aim = glm::vec3(0, 0, -1);
	//	isSelectable = false;
	//}

	//void setupGUI() {}
	//void updateGUI() {}
	//void draw() { ofDrawBox(position, 1.0); };
	//float sdf(const glm::vec3& p) {}
	//void drawFrustum() {}

	//glm::vec3 aim;
	//ViewPlane view;          // The camera viewplane, this is the view that we will render 
};
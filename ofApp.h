#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "Primitives.h"
#include <glm/gtx/intersect.hpp>


class ofApp : public ofBaseApp {
public:
	void setup();

	void setupGUI() {
		gui.setup("Render Settings");

		rayTraceScene.addListener(this, &ofApp::rayTraceRender);
		rayMarchScene.addListener(this, &ofApp::rayMarchRender);
		lockCamera.addListener(this, &ofApp::toggleCamInput);
		updateRender.addListener(this, &ofApp::updateRenderCam);
		delObject.addListener(this, &ofApp::deleteSelected);

		gui.add(rayTraceScene.setup("RayTrace (R)"));
		gui.add(rayMarchScene.setup("RayMarch (M)"));
		gui.add(lockCamera.set("Lock View Camera (C)", false));
		gui.add(updateRender.setup("Update RenderCam (TAB)"));
		gui.add(delObject.setup("Delete Selection (DEL)"));
		
		createPlane.addListener(this, &ofApp::addPlane);
		createSphere.addListener(this, &ofApp::addSphere);
		createMenger.addListener(this, &ofApp::addMengerSponge);
		createMandelbulb.addListener(this, &ofApp::addMandelbulb);

		gui.add(objSettings.setup("Scene Objects", ""));
		gui.add(createPlane.setup("Create New Plane"));
		gui.add(createSphere.setup("Create New Sphere"));
		gui.add(createMenger.setup("Create Menger Sponge"));
		gui.add(createMandelbulb.setup("Create Mandelbulb"));

		createPointLight.addListener(this, &ofApp::addPointLight);
		createAreaLight.addListener(this, &ofApp::addAreaLight);

		gui.add(lightSettings.setup("Lights", ""));
		gui.add(createPointLight.setup("Create New PointLight"));
		gui.add(createAreaLight.setup("Create New AreaLight"));

		res1200x800.addListener(this, &ofApp::res12X8);
		res600x400.addListener(this, &ofApp::res6X4);

		imageSettings.setName("Render Image Options");
		imageSettings.add(bRendered.set("Show Image (I)", false));
		imageSettings.add(res1200x800.set("1200 x 800", true));
		imageSettings.add(res600x400.set("600 x 400", false));

		gui.add(imageSettings);

		lambertShading.addListener(this, &ofApp::lambertOnly);
		phongShading.addListener(this, &ofApp::phongOnly);

		shadingSettings.setName("Shading Settings");
		shadingSettings.add(lambertShading.set("Lambert Shading", false));
		shadingSettings.add(phongShading.set("Phong Shading", false));
		shadingSettings.add(ambientLightIntensity.set("Ambient Light Intensity", 0.1, 0.05, 0.5));
		shadingSettings.add(phongPower.set("Phong p value", 10, 0, 50));

		gui.add(shadingSettings);

		noTexture.addListener(this, &ofApp::applyNoTexture);
		brickWall.addListener(this, &ofApp::applyBrickWall);
		cobblestonePavement.addListener(this, &ofApp::applyCobblestone);
		garagePaving.addListener(this, &ofApp::applyGaragePaving);
		marbleFloor.addListener(this, &ofApp::applyMarbleFloor);

		textures.setName("Texture Options");
		textures.add(noTexture.set("No Texture", false));
		textures.add(brickWall.set("Brick Wall", false));
		textures.add(cobblestonePavement.set("Cobblestone Pavement", false));
		textures.add(garagePaving.set("Garage Paving", false));
		textures.add(marbleFloor.set("Marble Floor", false));

		gui.add(textures);
	}

	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	bool mouseToDragPlane(int x, int y, glm::vec3& point);
	bool objSelected() { return (selected.size() ? true : false); };

	// listener functions for gui
	void updateRenderCam() {
		// update render cam to match current cam
		renderCam.setPosition(theCam->getPosition());
		renderCam.lookAt(theCam->getLookAtDir());
		renderCam.setOrientation(theCam->getGlobalOrientation());
	}
	void toggleCamInput(bool& val) {
		if (lockCamera) {
			mainCam.disableMouseInput();
		}
		else {
			mainCam.enableMouseInput();
		}
	}
	void deleteSelected() {
		if (objSelected()) {
			removeSelectedObject(selected[0]);
		}
	}
	void res6X4(bool& val) { 
		if (val) {
			imageWidth = 600;
			imageHeight = 400;
			image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
			res1200x800 = false;
		}
	}
	void res12X8(bool& val) {
		if (val) {
			imageWidth = 1200;
			imageHeight = 800;
			image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);
			res600x400 = false;

		}
	}
	void lambertOnly(bool& val) { if (lambertShading) phongShading = false; }
	void phongOnly(bool& val) { if (phongShading) lambertShading = false; }
	void applyNoTexture(bool& val);
	void applyBrickWall(bool& val);
	void applyCobblestone(bool& val);
	void applyGaragePaving(bool& val);
	void applyMarbleFloor(bool& val);

	// raytrace functions
	void rayTraceRender();
	bool inShadow(Ray ray);

	// raymarch functions
	void rayMarchRender();
	bool rayMarch(const Ray& r, glm::vec3& p, int& obj);
	float sceneSDF(const glm::vec3& p, int& obj);
	float sceneSDF(const glm::vec3& p);
	bool inShadowRM(const Ray& r);
	glm::vec3 getNormalRM(const glm::vec3& p);

	// general rendering functions
	ofColor colorPixel(SceneObject* obj, const glm::vec3& p, glm::vec3 n);
	ofColor shading(const glm::vec3& p, const glm::vec3& norm,
		const ofColor diffuse, const ofColor specular, float power);
	
	void drawGrid() {}

	// listener functions for adding/removing objects
	void removeSelectedObject(SceneObject* obj);
	void addPlane();
	void addSphere();
	void addMengerSponge();
	void addMandelbulb();
	void addLight(Light* l) { lights.push_back(l); } 
	void addPointLight();
	void addAreaLight();

	// camera objects
	ofEasyCam mainCam;
	ofEasyCam sideCam;
	ofEasyCam renderCam;
	ofEasyCam* theCam;    // set to current camera either mainCam or sideCam


	// scene objects
	vector<SceneObject*> scene, selected;

	// light objects
	vector<Light*> lights;
	ofLight keyLight, fillLight, rimLight;
	ofLight sceneLight; // pre-render light
	AmbientLight ambientLight;
	AreaLight* areaLight;

	// render image
	ofImage image;
	int imageWidth = 1200;
	int imageHeight = 800;
	bool raytrace = false;
	bool raymarch = false;
	static int ofApp::ext;
	static int ofApp::rm;

	// ray marching
	int maxRaySteps = 1000;
	float distThreshold = 0.01;
	float maxDistance = 100;
	float normalEps = 0.01;

	// texture maps
	ofImage garageDiffuse, garageSpecular;
	ofImage brickDiffuse, brickSpecular;
	ofImage cobbleDiffuse, cobbleSpecular;
	ofImage marbleDiffuse, marbleSpecular;
	
	// state
	map<int, bool> keymap;
	bool bDrag;
	glm::vec3 lastPoint;

	// gui
	ofxPanel gui;
	bool bHide = false;
	ofParameter<bool> lockCamera;
	ofxButton updateRender;
	ofxLabel objSettings;
	ofxButton createPlane, createSphere, createMenger, createMandelbulb, delObject;
	ofxLabel lightSettings;
	ofxButton createPointLight, createAreaLight;

	// image settings
	ofParameterGroup imageSettings;
	ofParameter<bool> res600x400, res1200x800;
	ofxButton rayTraceScene, rayMarchScene;
	ofParameter<bool> bRendered;

	// renderOptions options
	ofParameterGroup shadingSettings;
	ofParameter<float> ambientLightIntensity;
	ofParameter<bool> lambertShading, phongShading;
	ofParameter<float> phongPower;

	// texture application
	ofParameterGroup textures;
	ofParameter<bool> noTexture;
	ofParameter<bool> garagePaving;
	ofParameter<bool> brickWall;
	ofParameter<bool> cobblestonePavement;
	ofParameter<bool> marbleFloor;

};



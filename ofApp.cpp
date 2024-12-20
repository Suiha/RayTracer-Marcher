#include "ofApp.h"

void ofApp::setup() {
	printf("Setup\n");
	ofSetBackgroundColor(ofColor::gray);
	theCam = &mainCam;

	// camera setup
	mainCam.setDistance(10);
	mainCam.setNearClip(.1);

	sideCam.setPosition(glm::vec3(20, 10, 0));
	sideCam.lookAt(glm::vec3(0, 0, 0));
	sideCam.setNearClip(.1);

	renderCam.setPosition(mainCam.getPosition());
	renderCam.lookAt(mainCam.getLookAtDir());
	renderCam.setNearClip(.1);
	renderCam.disableMouseInput();


	// pre-render light
	sceneLight.enable();
	sceneLight.setPosition(0, 10, 0);
	sceneLight.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	sceneLight.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	// lights
	addLight(new PointLight(glm::vec3(5, 8, 0), 200));
	addLight(new PointLight(glm::vec3(-3, 10, 0), 100));
	//addLight(new PointLight(glm::vec3(4, 20, 0)));
	areaLight = new AreaLight(glm::vec3(0, 10, 0), 10, 5, 5, 10, 10, 1);
	addLight(areaLight);

	// gui
	setupGUI();

	// allocate space for rendered image
	image.allocate(imageWidth, imageHeight, OF_IMAGE_COLOR);

	// load texture maps
	garageDiffuse.load("garage-paving/11_garage paving PBR texture_DIFF.jpg");
	garageSpecular.load("garage-paving/11_garage paving PBR texture_SPEC.jpg");
	brickDiffuse.load("brick-wall/38_brick wall_DIFF.jpg");
	brickSpecular.load("brick-wall/38_brick wall_SPEC.jpg");
	cobbleDiffuse.load("cobblestone-pavement/13_cobblestone pavement PBR texture_DIFFUSE.jpg");
	cobbleSpecular.load("cobblestone-pavement/13_cobblestone pavement PBR texture_SPEC.jpg");
	marbleDiffuse.load("marble-floor/44_marble floor_DIFF.jpg");
	marbleSpecular.load("marble-floor/44_marble floor_SPEC.jpg");


	// create scene objects (for testing)

	// test: walls & floor
	// plane: origin point, normal vector, color
	/*Plane* backWall = new Plane(glm::vec3(0, 8, -10), glm::vec3(0, 0, 1), ofColor::gray); // vertical plane, facing forward
	scene.push_back(backWall);
	Plane* leftWall = new Plane(glm::vec3(-5, 8, 0), glm::vec3(1, 0, 0), ofColor::gray); // vertical plane, facing right
	scene.push_back(leftWall);
	Plane* rightWall = new Plane(glm::vec3(5, 8, 0), glm::vec3(-1, 0, 0), ofColor::gray); // vertical plane, facing left
	scene.push_back(rightWall);*/
	Plane* floor = new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0), ofColor::darkGray); // horizontal plane, facing up
	scene.push_back(floor);

	// test: objects
	/*Sphere* sphere1 = new Sphere(glm::vec3(0, 1, -2), 2.0, ofColor::lightBlue);
	scene.push_back(sphere1);
	Sphere* sphere2 = new Sphere(glm::vec3(-2.5, 0, 0), 1.0, ofColor::pink);
	scene.push_back(sphere2);
	MengerSponge* sponge1 = new MengerSponge(glm::vec3(0, 0, 0), ofColor::orange, 1, 2);
	scene.push_back(sponge1);*/
	/*Mandelbulb* bulb1 = new Mandelbulb(glm::vec3(0,0,0), ofColor::yellow, 5, 8, 10);
	scene.push_back(bulb1);*/

	// test: assign texture maps to objects
	/*floor->diffuseMap = garageDiffuse;
	floor->specularMap = garageSpecular;
	floor->numTiles = 1;
	floor->nTiles = 1;
	floor->textureName = "Garage Paving";

	backWall->diffuseMap = brickDiffuse;
	backWall->specularMap = brickSpecular;
	backWall->numTiles = 8;
	backWall->nTiles = 8;
	backWall->textureName = "Brick Wall";

	leftWall->diffuseMap = brickDiffuse;
	leftWall->specularMap = brickSpecular;
	leftWall->numTiles = 8;
	leftWall->nTiles = 8;
	leftWall->textureName = "Brick Wall";

	rightWall->diffuseMap = brickDiffuse;
	rightWall->specularMap = brickSpecular;
	rightWall->numTiles = 8;
	rightWall->nTiles = 8;
	rightWall->textureName = "Brick Wall";

	sphere1->diffuseMap = cobbleDiffuse;
	sphere1->specularMap = cobbleSpecular;
	sphere1->numTiles = sphere1->radius;
	sphere1->nTiles = sphere1->radius;
	sphere1->textureName = "Cobblestone Pavement";

	sphere2->diffuseMap = marbleDiffuse;
	sphere2->specularMap = marbleSpecular;
	sphere2->numTiles = sphere2->radius;
	sphere2->nTiles = sphere2->radius;
	sphere2->textureName = "Marble Floor";*/
}

void ofApp::update() {
	ambientLight.intensity = ambientLightIntensity;

	if (objSelected()) {
		// update parameters based on gui
		selected[0]->updateGUI();
	}
	else {
		// turn rotate and texture to false
		noTexture = false;
		garagePaving = false;
		brickWall = false;
		cobblestonePavement = false;
		marbleFloor = false;
	}
}

void ofApp::draw() {
	ofEnableDepthTest();
	theCam->begin();
	
	// add light to make scene order more clear
	ofEnableLighting();

	// draw scene objects
	ofFill();
	ofPushMatrix();
	for (auto obj : scene) {
		obj->draw();
	}
	for (auto l : lights) {
		l->draw();
	}
	ofPopMatrix();
	ofNoFill();

	ofDisableLighting();

	ofSetColor(ofColor::lightGray);
	mainCam.draw();

	theCam->end();
	ofDisableDepthTest();

	// rendered image
	if (bRendered) {
		image.draw((ofGetWindowWidth() / 2) - (imageWidth / 2), (ofGetWindowHeight() / 2) - (imageHeight / 2), imageWidth, imageHeight);
	}

	if (!bHide) {
		gui.draw();
		// draw gui panel of selected object
		if (objSelected()) {
			selected[0]->gui.setPosition(ofGetWindowWidth() - selected[0]->gui.getWidth(), 0);
			selected[0]->gui.draw();
		}
	}
}

// listener functions for textures
// apply relevant textures to selected object & turn off other texture buttons
void ofApp::applyNoTexture(bool& val) {
	if (objSelected() && noTexture) {
		selected[0]->textureName = "None";
		selected[0]->diffuseMap.clear();
		selected[0]->specularMap.clear();

		brickWall = false;
		garagePaving = false;
		cobblestonePavement = false;
		marbleFloor = false;
	}
}
void ofApp::applyBrickWall(bool& val) {
	if (objSelected() && brickWall) {
		selected[0]->textureName = "Brick Wall";
		selected[0]->diffuseMap = brickDiffuse;
		selected[0]->specularMap = brickSpecular;

		noTexture = false;
		garagePaving = false;
		cobblestonePavement = false;
		marbleFloor = false;
	}
}
void ofApp::applyCobblestone(bool& val) {
	if (objSelected() && cobblestonePavement) {
		selected[0]->textureName = "Cobblestone Pavement";
		selected[0]->diffuseMap = cobbleDiffuse;
		selected[0]->specularMap = cobbleSpecular;

		noTexture = false;
		garagePaving = false;
		brickWall = false;
		marbleFloor = false;
	}
}
void ofApp::applyGaragePaving(bool& val) {
	if (objSelected() && garagePaving) {
		selected[0]->textureName = "Garage Paving";
		selected[0]->diffuseMap = garageDiffuse;
		selected[0]->specularMap = garageSpecular;

		noTexture = false;
		brickWall = false;
		cobblestonePavement = false;
		marbleFloor = false;
	}
}
void ofApp::applyMarbleFloor(bool& val) {
	if (objSelected() && marbleFloor) {
		selected[0]->textureName = "Marble Floor";
		selected[0]->diffuseMap = marbleDiffuse;
		selected[0]->specularMap = marbleSpecular;

		noTexture = false;
		garagePaving = false;
		brickWall = false;
		cobblestonePavement = false;
	}
}
void ofApp::keyPressed(int key) {
	keymap[key] = true;

	// (un)lock main camera
	if (keymap['c'] || keymap['C']) {
		if (mainCam.getMouseInputEnabled()) {
			mainCam.disableMouseInput();
			lockCamera = true;
		}
		else {
			mainCam.enableMouseInput();
			lockCamera = false;
		}
	}

	// show/hide gui
	if (keymap['h'] || keymap['H']) {
		bHide = !bHide;
	}

	// show/hide rendered image
	if (keymap['i'] || keymap['I']) {
		bRendered = !bRendered;
	}

	// render image with raytracing
	if (keymap['r'] || keymap['R']) {
		rayTraceRender();
	}

	// render image with raymarching
	if (keymap['m'] || keymap['M']) {
		rayMarchRender();
	}

	// update render cam to match current cam
	if (keymap[OF_KEY_TAB]) {
		updateRenderCam();
	}

	// delete selected object
	if (keymap[OF_KEY_BACKSPACE] || keymap[OF_KEY_DEL]) {
		if (objSelected()) {
			removeSelectedObject(selected[0]);
		}
	}

	// view main camera (free cam)
	if (keymap[OF_KEY_F1]) {
		theCam = &mainCam;
	}

	// view render camera
	if (keymap[OF_KEY_F2]) {
		theCam = &renderCam;
	}

	// view side camera
	if (keymap[OF_KEY_F3]) {
		theCam = &sideCam;
	}
}

void ofApp::keyReleased(int key) {
	keymap[key] = false;
}
void ofApp::mouseMoved(int x, int y) {}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3& point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

void ofApp::mouseDragged(int x, int y, int button) {
	if (objSelected() && bDrag) {
		glm::vec3 point;
		mouseToDragPlane(x, y, point);
		
		// update object position
		selected[0]->position += point - lastPoint;
		selected[0]->objPos = selected[0]->position; // change slider to reflect change

		lastPoint = point;
	}
}

void ofApp::mousePressed(int x, int y, int button) {

	// if we are moving the camera around, don't allow selection
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	for (auto obj : selected) obj->bSelected = false;
	selected.clear();

	// test if something selected
	vector<SceneObject*> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	for (int i = 0; i < scene.size(); i++) {

		glm::vec3 point, norm;

		// we hit an object
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}

	// check for selection of lights
	for (int i = 0; i < lights.size(); i++) {

		glm::vec3 point, norm;

		// we hit a light
		if (lights[i]->isSelectable && lights[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(lights[i]);
		}
	}

	// if we selected more than one, pick nearest
	SceneObject* selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}
		}
	}

	if (selectedObj) {
		selected.push_back(selectedObj);
		selectedObj->bSelected = true;
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

void ofApp::mouseReleased(int x, int y, int button) {
	bDrag = false;
}

void ofApp::mouseEntered(int x, int y) {}
void ofApp::mouseExited(int x, int y) {}
void ofApp::windowResized(int w, int h) {}
void ofApp::dragEvent(ofDragInfo dragInfo) {}
void ofApp::gotMessage(ofMessage msg) {}


void ofApp::removeSelectedObject(SceneObject* obj) {

	Light* light = dynamic_cast<Light*>(selected[0]);
	if (light) { // obj is a light
		for (int i = 0; i < lights.size(); i++) {
			if (lights[i] == obj) {
				lights.erase(lights.begin() + i);
				break;
			}
		}
	}
	else { // obj is in the scene
		for (int i = 0; i < scene.size(); i++) {
			if (scene[i] == obj) {
				scene.erase(scene.begin() + i);
				break;
			}
		}
	}

	if (selected[0]) {
		selected.erase(selected.begin());
	}
}

void ofApp::addPlane() {
	Plane* plane = new Plane();
	scene.push_back(plane);
}

void ofApp::addSphere() {
	Sphere* sphere = new Sphere();
	scene.push_back(sphere);
	
}

void ofApp::addMengerSponge() {
	MengerSponge* menger = new MengerSponge();
	scene.push_back(menger);
}

void ofApp::addMandelbulb() {
	Mandelbulb* mandel = new Mandelbulb();
	scene.push_back(mandel);
}

void ofApp::addPointLight() {
	Light* light = new PointLight(glm::vec3(0, 10, 0));
	lights.push_back(light);
}

void ofApp::addAreaLight() {
	AreaLight* light = new AreaLight(glm::vec3(0, 10, 0));
	lights.push_back(light);
}


int ofApp::ext = 0;
int ofApp::rm = 0;


// main ray trace loop, called by 'r' button
void ofApp::rayTraceRender() {
	printf("raytrace called...\n");
	raytrace = true;

	// offsets for getting ray
	float w = (ofGetWindowWidth() - imageWidth) / 2;
	float h = (ofGetWindowHeight() - imageHeight) / 2;

	// go through each pixel in image
	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {

			float u = (i + 0.5) / imageWidth;
			float v = (j + 0.5) / imageHeight;

			// render through the preview cam
			glm::vec3 tmp = renderCam.screenToWorld(glm::vec3((u * imageWidth) + w, (v * imageHeight) + h, 0));
			Ray ray = Ray(renderCam.getPosition(), glm::normalize(tmp - renderCam.getPosition()));

			// variables to store information from intersection check
			float distance = std::numeric_limits<float>::infinity();
			glm::vec3 closestPoint;
			glm::vec3 normalAtIntersect;
			SceneObject* closestObject = NULL;

			// check all objects in scene for intersection
			for (SceneObject* object : scene) {
				glm::vec3 point;
				glm::vec3 normal;

				// check intersection distance from camera
				if (object->intersect(ray, point, normal)) {
					float intersectDistance = glm::distance(ray.p, point);
					if (intersectDistance < distance) {
						closestObject = object;
						closestPoint = point;
						normalAtIntersect = normal;
						distance = intersectDistance;
					}
				}
			}

			if (closestObject) {
				// color pixel based on closestObject
				ofColor color = colorPixel(closestObject, closestPoint, normalAtIntersect);
				image.setColor(i, j, color);
				
			}
			else {
				// default to background color if no object
				image.setColor(i, j, ofGetBackgroundColor());
			}
		}
	}

	// update & save image
	image.update();
	image.save("/renderedImages/render" + to_string(ofApp::ext++) + ".png");
	bRendered = true;

	raytrace = false;
	printf("rayTrace done\n");
}

// main ray march loop
void ofApp::rayMarchRender() {
	printf("rayMarch called...\n");
	raymarch = true;

	// offsets for getting ray
	float w = (ofGetWindowWidth() - imageWidth) / 2;
	float h = (ofGetWindowHeight() - imageHeight) / 2;

	for (int i = 0; i < imageWidth; i++) {
		for (int j = 0; j < imageHeight; j++) {

			float u = (i + 0.5) / imageWidth;
			float v = (j + 0.5) / imageHeight;

			// render through the preview cam
			glm::vec3 tmp = renderCam.screenToWorld(glm::vec3((u * imageWidth) + w, (v * imageHeight) + h, 0));
			Ray ray = Ray(renderCam.getPosition(), glm::normalize(tmp - renderCam.getPosition()));


			glm::vec3 p = ray.p;
			int obj = -1;
			bool hit = rayMarch(ray, p, obj);

			SceneObject* closestObject = scene[obj]; // closest object to ray


			// we hit the object, color the pixel
			if (hit) {
				ofColor color = colorPixel(closestObject, p, getNormalRM(p));
				image.setColor(i, j, color);

			}
			else {
				image.setColor(i, j, ofGetBackgroundColor());
			}
		}
	}

	// update & save image
	image.update();
	image.save("/raymarching/render" + to_string(ofApp::rm++) + ".png");
	bRendered = true;
	
	raymarch = false;
	printf("rayMarch done\n");
}

// ray marching algorithm
bool ofApp::rayMarch(const Ray& r, glm::vec3& p, int& obj) {
	bool hit = false;
	p = r.p;
	float dist;

	for (int i = 0; i < maxRaySteps; i++) {
		dist = sceneSDF(p, obj);

		if (dist < distThreshold) {
			hit = true;
			break;
		}
		else if (dist > maxDistance) {
			break;
		}
		else {
			p = p + (r.d * dist);
		}
	}

	return hit;
}

// checking scene for closest object in the scene
float ofApp::sceneSDF(const glm::vec3& p, int& obj) {
	float closest = std::numeric_limits<float>::infinity();

	for (int i = 0; i < scene.size(); i++) {
		// push back p (ray position) by obj's position
		float dist = scene[i]->sdf(p - scene[i]->position);
		if (dist < closest) {
			closest = dist;
			obj = i;
		}
	}
	return closest;
}

float ofApp::sceneSDF(const glm::vec3& p) {
	float closest = std::numeric_limits<float>::infinity();

	for (int i = 0; i < scene.size(); i++) {
		// push back p (ray position) by obj's position
		float dist = scene[i]->sdf(p - scene[i]->position);
		if (dist < closest) {
			closest = dist;
		}
	}
	return closest;
}

glm::vec3 ofApp::getNormalRM(const glm::vec3& p) {
	float eps = normalEps;
	float dp = sceneSDF(p);
	glm::vec3 n(dp - sceneSDF(glm::vec3(p.x - eps, p.y, p.z)),
		dp - sceneSDF(glm::vec3(p.x, p.y - eps, p.z)),
		dp - sceneSDF(glm::vec3(p.x, p.y, p.z - eps)));
	return glm::normalize(n);
}

// colors the pixel based on the object at that pixel
ofColor ofApp::colorPixel(SceneObject* obj, const glm::vec3& p, glm::vec3 n) {

	// default values if object has no texture/shading type not selected
	ofColor color = obj->diffuseColor;
	float specular = phongPower;

	// check for textures obj->textureName != "None"
	if (obj->diffuseMap.isAllocated() && obj->specularMap.isAllocated()) {
		//printf("applying texture...\n");

		// check object type (only plane/sphere)
		Plane* plane = dynamic_cast<Plane*>(obj);
		Sphere* sphere = dynamic_cast<Sphere*>(obj);
		MengerSponge* menger = dynamic_cast<MengerSponge*>(obj);

		// texture coordinates depend on object type
		float texU, texV;
		if (plane) {
			plane->getTextureCoords(p, texU, texV);
		}
		if (sphere) {
			sphere->getTextureCoords(p, texU, texV);
		}
		if (menger) { // who knows if this will work
			float dist = std::numeric_limits<float>::infinity();
			int face = -1;
			vector<Plane*> faces = menger->faces;
			for (int i = 0; i < faces.size(); i++) {
				float d = distance(faces[i]->position, p);
				if (d  < dist) {
					dist = d;
					face = i;
				}
			}
			faces[face]->getTextureCoords(p, texU, texV);
		}

		// get texture color from diffuse map
		float diffuseX = texU * obj->diffuseMap.getWidth();
		float diffuseY = texV * obj->diffuseMap.getHeight();
		diffuseX = ofClamp(diffuseX, 0, obj->diffuseMap.getWidth() - 1);
		diffuseY = ofClamp(diffuseY, 0, obj->diffuseMap.getHeight() - 1);
		color = obj->diffuseMap.getColor(diffuseX, diffuseY);

		// get specular coefficient from specular map
		if (phongShading) {
			int specX = texU * obj->specularMap.getWidth();
			int specY = texV * obj->specularMap.getHeight();
			specX = ofClamp(specX, 0, obj->specularMap.getWidth() - 1);
			specY = ofClamp(specY, 0, obj->specularMap.getHeight() - 1);
			specular = obj->specularMap.getColor(specX, specY).getBrightness();
		}
		
	}
	
	// apply shading if selected
	if (lambertShading || phongShading) {
		color = shading(p, n, color, ofColor::white, specular);
	}

	return color;
}

// shading (lambert / phong)
ofColor ofApp::shading(const glm::vec3& p, const glm::vec3& norm,
	const ofColor diffuse, const ofColor specular, float power) {

	ofColor result = ambientLight.intensity * diffuse;
	float totalDiffuse = 0;
	float totalSpecular = 0;

	for (auto light : lights) {
		if (light->intensity <= 0) continue; // skip lights with no "light"

		// calculate effect of lights
		int numRays = light->getRaySamples(p, norm); // get ray(s) from light
		for (int i = 0; i < numRays; i++) {

			bool shadow = false;
			if (raytrace) shadow = inShadow(light->samples[i]);
			if (raymarch) shadow = inShadowRM(light->samples[i]);

			if (!shadow) {

				// calculate intensity of light with respect to distance
				float distance = glm::length(light->samplesPos[i] - p);
				float illumination = light->intensity / (distance * distance);

				// lambert formula
				glm::vec3 lightDirection = light->samples[i].d;
				float lambertCalc = glm::max(glm::dot(norm, lightDirection), 0.0f);
				totalDiffuse += lambertCalc * illumination;

				// specular formula
				if (phongShading) {
					glm::vec3 viewDirection = glm::normalize(renderCam.getPosition() - p);
					glm::vec3 h = glm::normalize(viewDirection + lightDirection);
					float specularCalc = glm::pow(glm::max(glm::dot(norm, h), 0.0f), power);
					totalSpecular += specularCalc * illumination;
				}

			}
		}

		result += (diffuse * (totalDiffuse / numRays)) + (specular * (totalSpecular / numRays));
	}

	return result;
}


// check if any object in the scene intersects the ray between the light and point
bool ofApp::inShadow(Ray ray) {
	for (auto obj : scene) {
		glm::vec3 intersectPoint;
		glm::vec3 normal;
		// does not account for objects "above" light
		if (obj->intersect(ray, intersectPoint, normal)) {
			return true;
		}
	}
	return false;
}

// ray marching: check to see if Point p is in a shadow cast by light shining along Ray r
bool ofApp::inShadowRM(const Ray& r) {
	for (int i = 0; i < scene.size(); i++) {
		glm::vec3 point, normal;
		int obj;
		float eps = .08;    // to avoid self intersection 
		if (rayMarch(Ray(r.p + r.d * eps, r.d), point, obj))
			return true;
	}
	return false;
}

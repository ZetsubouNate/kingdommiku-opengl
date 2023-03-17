 //
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"

#include <iostream>
#include "SkyBox.hpp"

int glWindowWidth = 1920;
int glWindowHeight = 1080;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 4096;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
GLuint normalMatrixLoc2;
glm::mat4 lightRotation;
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
				glm::vec3(0.0f, 2.0f, 5.5f), 
				glm::vec3(0.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.02f;

bool pressedKeys[1024];
float angle = 90.0f;
GLfloat lightAngle;
GLfloat spotIntens;
GLfloat darkLoc;
GLfloat darkTranspLoc;

gps::Model3D kingdomValley;
gps::Model3D mikuKnight;
gps::Model3D mikuStar;
gps::Model3D mikuPersona;
gps::Model3D accessories;
gps::Model3D leaves;
gps::Model3D bushes;
gps::Model3D water;
gps::Model3D waterfall;
gps::Model3D greenEmerald;
gps::Model3D blueEmerald;
gps::Model3D grass;
gps::Model3D screenQuad;
gps::Model3D rain;

gps::SkyBox mySkyBox;
std::vector<const GLchar*> facesDay;
std::vector<const GLchar*> facesNight;
float dark = 1.0f;
float fog = 0.0f;
bool fogEnable = false;
bool rainEnable = false;
float rainIntensity = 1.0f;
std::vector<glm::vec3> raindrops;

gps::Shader skyboxShader;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader transparencyShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO	
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		if (spotIntens == 0.5f) {
			spotIntens = 0.0f;
		}
		else
		{
			spotIntens = 0.5f;

		}
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		if (fogEnable) {
			fogEnable = false;
			fog = 0.0f;
		}
		else
		{
			fogEnable = true;
			fog = 0.05f;

		}
	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		if (rainEnable) {
			rainEnable = false;
		}
		else
		{
			rainEnable = true;
		}
	}

	if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
		cameraSpeed = 0.06f;
	}
	else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
		cameraSpeed = 0.02f;
	}
}

bool firstMouse = true;
float lastX, lastY, yaw = -90.0f, pitch = 0.0f, limiter = 0.0f, sensitivity = 0.001f;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	else {
		yaw = xpos - lastX;
		pitch = lastY - ypos;
		if (limiter - pitch < 1300.f && limiter - pitch > -1300.f) {
			limiter -= pitch;
			myCamera.rotate(sensitivity * pitch, sensitivity * yaw);
		}
		lastX = xpos;
		lastY = ypos;
	}
}

void processMovement()
{	
	if (angle > 10000.0f)
	{
		angle = 0.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);	
		if (myCamera.getCameraPosition().y >= 40.0) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		}
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);	
		if (myCamera.getCameraPosition().y >= 40.0) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		}
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);		
	}

	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed);
		if (myCamera.getCameraPosition().y >= 40.0) {
			myCamera.move(gps::MOVE_DOWN, cameraSpeed);
		}
	}

	if (pressedKeys[GLFW_KEY_LEFT_ALT]) {
		myCamera.move(gps::MOVE_DOWN, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_N]) {

		angle += 0.01f;
		dark = abs(sin(angle)) + 0.05f;
		
		lightRotation = glm::rotate(glm::mat4(1.0f), 0.01f, glm::vec3(1.0f, 0.5f, 0.0f));
		lightDir = lightRotation * glm::vec4(lightDir, 1.f);
	}

	if (glfwGetKey(glWindow, GLFW_KEY_L)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (glfwGetKey(glWindow, GLFW_KEY_P)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (glfwGetKey(glWindow, GLFW_KEY_Q)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_BLEND);  //enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	kingdomValley.LoadModel("models/Kingdom Valley/kingdomValley.obj");
	greenEmerald.LoadModel("models/chaos_emerald/greenEmerald.obj");
	blueEmerald.LoadModel("models/chaos_emerald/blueEmerald.obj");
	mikuKnight.LoadModel("models/YYB miku Crown Knight/mikuCrownKnight.obj");
	mikuStar.LoadModel("models/Star Vocalist/mikuStarVocalist.obj");
	mikuPersona.LoadModel("models/P4D style v1.01/mikuP4D.obj");
	leaves.LoadModel("models/Kingdom Valley/treeLeaves.obj");
	bushes.LoadModel("models/Kingdom Valley/bushesOuter.obj");
	water.LoadModel("models/Kingdom Valley/water.obj");
	waterfall.LoadModel("models/Kingdom Valley/waterfalls.obj");
	grass.LoadModel("models/Kingdom Valley/grass.obj");
	screenQuad.LoadModel("models/quad/quad.obj");
	rain.LoadModel("models/Kingdom Valley/rain.obj");
	accessories.LoadModel("models/P4D style v1.01/accessories.obj");

	for (int i = 0; i < 1500; i++) {
		int rX = rand() % 35 - 12;
		int rY = rand() % 40 - 10;
		int rZ = rand() % 20 - 8;
		raindrops.push_back(glm::vec3(rX, rY, rZ));
	}

	facesDay.push_back("models/powderpeak/powderpeak_rt.tga");
	facesDay.push_back("models/powderpeak/powderpeak_lf.tga");
	facesDay.push_back("models/powderpeak/powderpeak_up.tga");
	facesDay.push_back("models/powderpeak/powderpeak_dn.tga");
	facesDay.push_back("models/powderpeak/powderpeak_bk.tga");
	facesDay.push_back("models/powderpeak/powderpeak_ft.tga");
	facesNight.push_back("models/interstellar/interstellar_rt.tga");
	facesNight.push_back("models/interstellar/interstellar_lf.tga");
	facesNight.push_back("models/interstellar/interstellar_up.tga");
	facesNight.push_back("models/interstellar/interstellar_dn.tga");
	facesNight.push_back("models/interstellar/interstellar_bk.tga");
	facesNight.push_back("models/interstellar/interstellar_ft.tga");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	transparencyShader.loadShader("shaders/blendShader.vert", "shaders/blendShader.frag");
	transparencyShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
	depthMapShader.useShaderProgram();
	mySkyBox.Load(facesDay, facesNight);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	darkLoc = glGetUniformLocation(myCustomShader.shaderProgram, "dark");
	glUniform1f(darkLoc, dark);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	transparencyShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(transparencyShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(glGetUniformLocation(transparencyShader.shaderProgram, "lightDir"), 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));
	glUniform3fv(glGetUniformLocation(transparencyShader.shaderProgram, "lightColor"), 1, glm::value_ptr(lightColor));
	darkTranspLoc = glGetUniformLocation(transparencyShader.shaderProgram, "dark");
	glUniform1f(darkTranspLoc, dark);

	skyboxShader.useShaderProgram();
	glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "blendFactor"), 1.0f);
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	myCustomShader.useShaderProgram();
	glm::vec3 spotLightsPos[6] = {
		glm::vec3(19.404f / 2, 12.4685f / 2, -3.86056f / 2 ),
		glm::vec3(19.404f / 2, 12.4685f / 2, -3.86056f / 2),
		glm::vec3(19.404f / 2, 12.4685f / 2, -3.86056f / 2),
		glm::vec3(19.404f / 2 , 12.4685f / 2, -3.86056f / 2),
		glm::vec3(3.43252f / 2, 1.30254f / 2, 0.519019f / 2),
		glm::vec3(3.43252f / 2, 1.30254f / 2, 0.519019f / 2)
	};

	glm::vec3 spotLightsColor[6] = {
		glm::vec3(46.0 / 255.0, 204.0 / 255.0, 113.0 / 255.0),
		glm::vec3(46.0 / 255.0, 204.0 / 255.0, 113.0 / 255.0),
		glm::vec3(46.0 / 255.0, 204.0 / 255.0, 113.0 / 255.0),
		glm::vec3(46.0 / 255.0, 204.0 / 255.0, 113.0 / 255.0),
		glm::vec3(0.0f, 0.0f,  1.0f),
		glm::vec3(0.0f, 0.0f,  1.0f)
	};

	glm::vec3 spotLightsDir[6] = {
		glm::vec3(0.0, 0.0, -1.0),
		glm::vec3(0.0, 0.0, 1.0),
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(1.0, 0.0, 0.0),
		glm::vec3(1.0f, 1.0f,  0.0f),
		glm::vec3(-1.0f, -1.0f,  0.0f)
	};

	GLfloat spotLightsInner[6] = {
		glm::radians(5.0),
		glm::radians(5.0),
		glm::radians(5.0),
		glm::radians(5.0),
		glm::radians(5.0),
		glm::radians(5.0)
	};

	GLfloat spotLightsOuter[6] = {
		glm::radians(10.0),
		glm::radians(10.0),
		glm::radians(10.0),
		glm::radians(10.0),
		glm::radians(10.0),
		glm::radians(10.0)
	};

	// get uniform locations
	GLuint spotLightsPosLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotPos");
	GLuint spotLightsColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotColor");
	GLuint spotLightsDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotDir");
	GLuint spotLightsInnerLoc = glGetUniformLocation(myCustomShader.shaderProgram, "innerCutoff");
	GLuint spotLightsOuterLoc = glGetUniformLocation(myCustomShader.shaderProgram, "outerCutoff");
	GLuint spotIntensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotIntensity");
	// set uniform values
	glUniform3fv(spotLightsPosLoc, 6, glm::value_ptr(spotLightsPos[0]));
	glUniform3fv(spotLightsColorLoc, 6, glm::value_ptr(spotLightsColor[0]));
	glUniform3fv(spotLightsDirLoc, 6, glm::value_ptr(spotLightsDir[0]));
	glUniform1fv(spotLightsInnerLoc, 6, spotLightsInner);
	glUniform1fv(spotLightsOuterLoc, 6, spotLightsOuter);
	spotIntens = 0.0f;
	glUniform1f(spotIntensityLoc, spotIntens);
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID 
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);


	glDrawBuffer(GL_NONE); 
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//TODO - Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(glm::mat3(lightRotation) * lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.01f, far_plane = 40.0f; 
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	return lightSpaceTrMatrix;
}

float delta1 = 0; float delta2 = 0; float delta3 = 0; // units per second 
float updateDelta(float delta, double elapsedSeconds, float movementSpeed) { 
	return delta + movementSpeed * elapsedSeconds; 
} 
double lastTimeStamp = glfwGetTime();

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();

	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(glGetUniformLocation(shader.shaderProgram, "fogDensity"), fog);
	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	GLuint spotIntensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "spotIntensity");
	glUniform1f(spotIntensityLoc, spotIntens);

	glUniform1f(darkLoc, dark);
	kingdomValley.Draw(shader);
	mikuKnight.Draw(shader);
	mikuStar.Draw(shader);
	mikuPersona.Draw(shader);
	accessories.Draw(shader);

	// get current time 
	double currentTimeStamp = glfwGetTime(); 
	delta1 = updateDelta(delta1, currentTimeStamp - lastTimeStamp, 25);
	
	model = glm::mat4(1.0f);
	glm::vec3 origin = glm::vec3(19.404f / 2.0f, -12.4685f / 2.0f, -3.86056f / 2.0f);
	float amplitude = 0.003f;
	float displacement = sin(delta1 * 0.05f * glm::pi<float>()) * amplitude;
	float newY = origin.y + displacement;
	model = glm::translate(model, origin);
	model = glm::translate(model, glm::vec3(0.0f, newY - origin.y, 0.0f));
	model = glm::rotate(model, glm::radians(delta1), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, -origin);
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	greenEmerald.Draw(shader);
	
	//nu mai vreau
	delta2 = updateDelta(delta2, currentTimeStamp - lastTimeStamp, 1);
	
	model = glm::mat4(1.0f);
	amplitude = 0.000825f;
	displacement = sin(delta2 * 1.2f * glm::pi<float>()) * amplitude;
	newY = 1.30254f + displacement;
	model = glm::translate(model, glm::vec3(0.0f, newY, 0.0f));
	model = glm::translate(model, glm::vec3(0.0f, -1.30254f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	blueEmerald.Draw(shader);
	
	delta3 = updateDelta(delta3, currentTimeStamp - lastTimeStamp, 1);
	float radius = 0.009f;
	amplitude = 0.009f; 
	float speed = 1.2f;
	float xPos = radius * cos(delta3 * speed);
	float yPos = amplitude * sin(delta3 * speed);
	model = glm::mat4(1.0f);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, yPos, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	leaves.Draw(shader);

	radius = 0.01f;
	amplitude = 0.01f;
	speed = 1.0f;
	xPos = radius * cos(delta3 * speed);
	float zPos = radius * sin(delta3 * speed);
	model = glm::mat4(1.0f);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.0f, zPos));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	grass.Draw(shader);

	radius = 0.03f;
	amplitude = 0.03f;
	speed = 1.4f;
	xPos = radius * cos(delta3 * speed);
	zPos = radius * sin(delta3 * speed);
	model = glm::mat4(1.0f);
	model = glm::translate(glm::mat4(1.0f), glm::vec3(xPos, 0.0f, zPos));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	bushes.Draw(shader);

	transparencyShader.useShaderProgram();
	glUniform1f(darkTranspLoc, dark);
	glUniformMatrix4fv(glGetUniformLocation(transparencyShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.5f));
	glUniformMatrix4fv(glGetUniformLocation(transparencyShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(glGetUniformLocation(transparencyShader.shaderProgram, "fogDensity"), fog);
	water.Draw(transparencyShader);
	waterfall.Draw(transparencyShader);	

	shader.useShaderProgram();
	glm::vec3 velocity(0.0f, -10.0f, 0.0f);
	float deltaTime = currentTimeStamp - lastTimeStamp;
	lastTimeStamp = currentTimeStamp;
	if (rainEnable) {
		for (int i = 0; i < 1500; i++) {
			glm::vec3& position = raindrops.at(i);
			position += velocity * deltaTime;
			glm::mat4 model(1.0f);
			model = glm::scale(model, glm::vec3(0.5f));
			model = glm::rotate(model, glm::radians(15.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			model = glm::translate(model, position);
			glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			rain.Draw(shader);
			if (raindrops.at(i).y < -34.f) {
				raindrops.at(i).y = rand() % 40 - 10;
			}
		}
	}
	
}

void renderScene() {

	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	//render scene -> drawObjects
	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render depth map on screen - toggled with the M key
	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		drawObjects(myCustomShader, false);

		skyboxShader.useShaderProgram();
		glUniform1f(glGetUniformLocation(skyboxShader.shaderProgram, "blendFactor"), dark);
		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1,&depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();

	glCheckError();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}

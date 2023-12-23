// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <random>
#include <thread>
#include <omp.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/common.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include "assimp_model.h"

float minX = -30.0f, maxX = 30.0f;
float minY = -30.0f, maxY = 30.0f;
float minZ = 1.4f, maxZ = 10.0f;

struct ObjectState {
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 velocity;
	glm::vec3 angularVelocity;
};

struct LightSource {
	glm::vec3 position;
	float intensity;
};



// 生成介于minIntensity和maxIntensity之间的随机光强度
float RandomIntensity(float minIntensity = 0.0f, float maxIntensity = 50.0f) {
	static std::random_device rd;  // 用于获得随机数种子
	static std::mt19937 gen(rd()); // 标准的mersenne_twister_engine
	static std::uniform_real_distribution<float> dis(minIntensity, maxIntensity);

	return dis(gen);
}

// 生成随机速度和角速度
glm::vec3 RandomVelocity(float minSpeed, float maxSpeed) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(minSpeed, maxSpeed);

	return glm::vec3(dis(gen), dis(gen), dis(gen));
}

glm::vec3 RandomAngularVelocity(float minAngularSpeed, float maxAngularSpeed) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dis(minAngularSpeed, maxAngularSpeed);

	return glm::vec3(dis(gen), dis(gen), dis(gen));
}

// 碰撞检测和响应函数
void handleCollisions(std::vector<ObjectState>& states, float radius) 
{
	#pragma omp parallel for
	for (size_t i = 0; i < states.size(); i++) {
		for (size_t j = i + 1; j < states.size(); j++) {
			glm::vec3 delta = states[j].position - states[i].position;
			float distance = glm::length(delta);
			float minDistance = radius * 2;

			if (distance < minDistance) {
				glm::vec3 midPoint = (states[i].position + states[j].position) / 2.0f;
				states[i].position = midPoint - (delta / distance) * (minDistance / 2.0f);
				states[j].position = midPoint + (delta / distance) * (minDistance / 2.0f);

				// 简单的反弹响应，交换速度
				std::swap(states[i].velocity, states[j].velocity);
			}
		}
	}
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);


	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Final Project -Fan Zilong", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);

	/*

		Model Load Section

	*/

	// Define Model File Name
	std::vector<std::string> modelFiles = 
	{
	"rec.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj",
	"Basketball_ball.obj"
	};

	// Create Four CAssimpModel 
	//CAssimpModel model;
	std::vector<CAssimpModel> models(modelFiles.size());

	//Check the loading Situation
	for (size_t i = 0; i < modelFiles.size(); ++i) 
	{
		if (i >= 1)
		{
			if (!models[i].LoadModelFromFile("Basketball_ball.obj"))
			{
				fprintf(stderr, "Failed to load model file: %s\n", "Basketball_ball.obj");
				getchar();
				glfwTerminate();
				return -1;
			}
		}

		if (i == 0)
		{
			if (!models[i].LoadModelFromFile("rec.obj"))
			{
				fprintf(stderr, "Failed to load model file: %s\n", "rec.obj");
				getchar();
				glfwTerminate();
				return -1;
			}
		}

	}

	/*

		Model Position Section

	*/
	std::vector<glm::vec3> modelPositions = 
	{
		glm::vec3(0.0f, 0.0f, -0.0f),  // Model 0 
		glm::vec3(-4.0f, 0.0f, 3.0f),  // Dynamic Model 1 
		glm::vec3(4.0f, 0.0f, 3.0f),   // Dynamic Model 2 
		glm::vec3(0.0f, 4.0f, 3.0f),   // Dynamic Model 3 
		glm::vec3(0.0f, -4.0f, 3.0f),   // Dynamic Model 4 
		glm::vec3(-8.0f, 0.0f, 1.4f),  // Static Model 1 
		glm::vec3(14.0f, 0.0f, 1.4f),   // Static Model 2 
		glm::vec3(0.0f, 24.0f, 1.4f),   // Static Model 3 
	};

	// Declare the objectState
	std::vector<ObjectState> objectStates(models.size());
	//Initialize all the parameter
	for (size_t i = 0; i < objectStates.size(); i++) {
		objectStates[i].position = modelPositions[i]; // 初始位置
		objectStates[i].rotation = glm::quat();      // 初始旋转
		if (i > 0 && i < (objectStates.size()-3))
		{
			objectStates[i].velocity = RandomVelocity(-0.1f, 0.1f); // 随机速度
			objectStates[i].angularVelocity = RandomAngularVelocity(-1.0f, 1.0f); // 随机角速度
		}
		else
		{
			objectStates[i].velocity = glm::vec3(0, 0, 0); // 初始速度
			objectStates[i].angularVelocity = glm::vec3(0, 0, 0); // 初始角速度
		}

	}

	bool is_g_activatemotion = false;

	CAssimpModel::FinalizeVBO();

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "StandardShading.vertexshader", "StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");
	
	// Get a handle for our buffers
	GLuint vertexPosition_modelspaceID = glGetAttribLocation(programID, "vertexPosition_modelspace");
	GLuint vertexUVID = glGetAttribLocation(programID, "vertexUV");
	GLuint vertexNormal_modelspaceID = glGetAttribLocation(programID, "vertexNormal_modelspace");

	//// Load the texture
	GLuint Texture2 = loadBMP_custom("pumpkin.bmp");
	GLuint Texture = loadBMP_custom("court.bmp");

	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	do{

		/*

			Timing Section

		*/
		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0/double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}
		float deltaTime = currentTime - lastTime;

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);


		if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
			is_g_activatemotion = !is_g_activatemotion;
		}


		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		for (size_t i = 0; i < models.size(); ++i) 
		{
			/*

				Bounce Dectection Section:

			*/
			for (size_t i = 1; i < objectStates.size(); i++) 
			{
				// 检查X轴边界
				if (objectStates[i].position.x < minX) 
				{
					objectStates[i].position.x = minX;
					objectStates[i].velocity.x *= -1; // 反转X轴速度
				}
				else if (objectStates[i].position.x > maxX)
				{
					objectStates[i].position.x = maxX;
					objectStates[i].velocity.x *= -1; // 反转X轴速度
				}

				// 检查Y轴边界
				if (objectStates[i].position.y < minY)
				{
					objectStates[i].position.y = minY;
					objectStates[i].velocity.y *= -1; // 反转Y轴速度
				}
				else if (objectStates[i].position.y > maxY) 
				{
					objectStates[i].position.y = maxY;
					objectStates[i].velocity.y *= -1; // 反转Y轴速度
				}

				// 检查Z轴边界
				if (objectStates[i].position.z < minZ) 
				{
					objectStates[i].position.z = minZ;
					objectStates[i].velocity.z *= -1; // 反转Z轴速度
				}
				else if (objectStates[i].position.z > maxZ) 
				{
					objectStates[i].position.z = maxZ;
					objectStates[i].velocity.z *= -1; // 反转Z轴速度
				}

				// 处理碰撞
				handleCollisions(objectStates, 1.4f); // 假设每个对象的半径为1.4f
			}

			/*

				Moving Control Section:
					Moving

			*/
			if (is_g_activatemotion) {
				for (size_t i = 1; i < models.size()-3; i++) {
					// 基于速度更新位置
					objectStates[i].position += objectStates[i].velocity * deltaTime;

					// 基于角速度更新旋转
					objectStates[i].rotation = glm::normalize(glm::mix(objectStates[i].rotation, glm::quat(objectStates[i].angularVelocity * deltaTime), 0.5f));

					// 更新 modelPositions 以用于后续渲染
					modelPositions[i] = objectStates[i].position;
				}
			}

			/*

				Position Section

			*/
			// 创建模型矩阵并应用位置变换
			glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), objectStates[i].position);
			// 将四元数旋转转换为矩阵
			glm::mat4 RotationMatrix = glm::toMat4(objectStates[i].rotation);
			// 组合平移和旋转矩阵
			glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix;
			
			// 计算最终的MVP矩阵
			glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
			glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);
			
			/* 
				
				Light Section

			*/
			glm::vec3 lightPos = glm::vec3(4, 4, 20);
			glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

			//lightIntensities[i] = RandomIntensity();
			//glUniform1f(glGetUniformLocation(programID, "LightIntensity"), lightIntensities[i]);

			// Bind our texture in Texture Unit 0
			glActiveTexture(GL_TEXTURE0);
			
			if (i == 0) {
				glBindTexture(GL_TEXTURE_2D, Texture); // 使用Texture2纹理
			}
			else {
				glBindTexture(GL_TEXTURE_2D, Texture2); // 使用Texture纹理
			}
			
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(TextureID, 0);
			//glUniform1f(glGetUniformLocation(programID, "LightIntensity"), lightSources[i].intensity);

			// Rendering
			CAssimpModel::BindModelsVAO();
			models[i].RenderModel();
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	//// Cleanup VBO and shader
	//glDeleteProgram(programID);
	//glDeleteTextures(1, &Texture);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}


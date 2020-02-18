#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <shader.h>
#include <camera.h>

#include <iostream>
#include <vector>
#include <fstream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//键盘操作的view
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

//std::vector<float> vertices; 
GLfloat vertices[25000 * 3];
//std::vector<int> indices;
GLint indices[50000 * 3];
//平面法向量取均值就是顶点法向量，然而没有先归一化
float normal[25000 * 3];
//添加了新属性的顶点数组
GLfloat newvertices[25000 * 6];

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// lighting y轴是反转的？！！
glm::vec3 lightPos(1.2f, -3.0f, 2.0f);

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//启用深度测试(前后遮挡)
	//glEnable(GL_DEPTH_TEST);
	//glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// 用一个类封装了顶点/片段着色器的使用
	// ------------------------------------
	Shader ourShader("camera.vs", "camera.fs");

	//顶点新增了颜色和纹理属性(此处没用到颜色)，纹理属性是纹理图片坐标值(左下为0，0)
	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------

	char tp;
	int i = 0, j = 0, tmp;
	float tmpfloat;
	std::ifstream fin("./dragon.obj");
	while (fin >> tp) {
		if (tp == 'v') {
			fin >> tmpfloat;
			if (fin.fail()) { fin.clear(); fin.sync(); continue; }
			else vertices[i++] = tmpfloat;
			fin >> vertices[i++];
			fin >> vertices[i++];
		}
		else if (tp == 'f') {
			fin >> tmp;
			indices[j++] = (tmp - 1);
			fin >> tmp;
			indices[j++] = (tmp - 1);
			fin >> tmp;
			indices[j++] = (tmp - 1);
		}
	}

	int cnt[25000] = { 0 };
	int pt1idx, pt2idx, pt3idx;
	glm::vec3 a, b, normaltp;
	for (i = 0; i < 150000; i += 3) {
		pt1idx = indices[i];
		pt2idx = indices[(i + 1)];
		pt3idx = indices[(i + 2)];
		a = glm::vec3(vertices[(pt1idx * 3)] - vertices[(pt2idx * 3)],
			vertices[(pt1idx * 3 + 1)] - vertices[(pt2idx * 3 + 1)],
			vertices[(pt1idx * 3 + 2)] - vertices[(pt2idx * 3 + 2)]);
		b = glm::vec3(vertices[(pt3idx * 3)] - vertices[(pt2idx * 3)],
			vertices[(pt3idx * 3 + 1)] - vertices[(pt2idx * 3 + 1)],
			vertices[(pt3idx * 3 + 2)] - vertices[(pt2idx * 3 + 2)]);

		normaltp = glm::vec3(a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x);

		tmpfloat = sqrt(normaltp.x * normaltp.x + normaltp.y * normaltp.y + normaltp.z * normaltp.z);
		normaltp = glm::vec3(normaltp.x / tmpfloat, normaltp.y / tmpfloat, normaltp.z / tmpfloat);

		normal[(pt1idx * 3)] += normaltp.x;
		normal[(pt1idx * 3 + 1)] += normaltp.y;
		normal[(pt1idx * 3 + 2)] += normaltp.z;

		normal[(pt2idx * 3)] += normaltp.x;
		normal[(pt2idx * 3 + 1)] += normaltp.y;
		normal[(pt2idx * 3 + 2)] += normaltp.z;

		normal[(pt3idx * 3)] += normaltp.x;
		normal[(pt3idx * 3 + 1)] += normaltp.y;
		normal[(pt3idx * 3 + 2)] += normaltp.z;

		cnt[pt1idx]++;
		cnt[pt2idx]++;
		cnt[pt3idx]++;
	}


	for (i = 0; i < 25000; i++) {
		newvertices[i * 6] = vertices[i * 3];
		newvertices[i * 6 + 1] = vertices[i * 3 + 1];
		newvertices[i * 6 + 2] = vertices[i * 3 + 2];

		newvertices[i * 6 + 3] = normal[i * 3] / cnt[i];
		newvertices[i * 6 + 4] = normal[i * 3 + 1] / cnt[i];
		newvertices[i * 6 + 5] = normal[i * 3 + 2] / cnt[i];

	}
	std::ofstream outfile("./newvertices.txt");
	for (i = 0; i < 25000 * 6; i++) {
		if (i % 6 == 0) outfile << std::endl;
		outfile << newvertices[i] << ' ';
	}
	outfile.close();

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(newvertices), newvertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 0 对应顶点着色器texture.vs中的layout=0，位置属性提取,0号输入，长度3float，间隔3，初始位置0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);//添加了法向量属性
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);//单属性
	glEnableVertexAttribArray(0);

	//法向量属性
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	/*
	// 1 对应顶点着色器texture.vs中的layout=1，颜色属性提取
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// 2 对应顶点着色器texture.vs中的layout=0，纹理属性提取
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	//生成纹理对象
	// load and create a texture
	// -------------------------
	unsigned int texture1, texture2;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	//绑定2D纹理类
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters，两个坐标上包装选项均为重复(坐标值超出时自动重复纹理)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters，放大缩小均使用线性过滤(平滑/模糊)，临近(颗粒像素)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char* data = stbi_load("./container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		//用图像生成纹理
		//参数解释：生成2D纹理，0级渐变，rgb格式纹理，源图以rgb加载，储存为字节，图像数据
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	//得到纹理后释放图像内存
	stbi_image_free(data);
	// texture 2
	// ---------
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load("./awesomeface.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	// -------------------------------------------------------------------------------------------
	ourShader.use(); // don't forget to activate/use the shader before setting uniforms!

	//获取片段着色器中的uniform地址，把对应号码的纹理发送给着色器
	// either set it manually like so:
	glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
	// or set it via the texture class
	ourShader.setInt("texture2", 1);

	*/

	//设置为线框模式
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //图像变化后，深度信息也要清除(遮挡关系改变)
		/*
		//启用纹理单元再绑定
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		*/

		// render container
		ourShader.use();//启用着色器程序

		ourShader.setVec3("objectColor", 0.92f, 0.89f, 0.41f);
		ourShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		ourShader.setVec3("lightPos", lightPos);
		ourShader.setVec3("viewPos", camera.Position);

		// create transformations变换最好按照-缩旋移-的顺序方便控制，矩阵生成次序正好相反(依次右乘)-移旋缩，从左往右，最右最先
		//glm::mat4 transform = glm::mat4(1.0f); // make sure to initialize matrix to单位矩阵Identity
		//注意旋转角按原点视角！45度x轴是从原点沿x轴看顺时针45，而view面垂直于z轴

		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), float(SCR_WIDTH / SCR_HEIGHT), 0.1f, 100.0f);//proj
		ourShader.setMat4("projection", projection);

		// camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);
		ourShader.setMat4("model", model);

		glBindVertexArray(VAO);//启用顶点数据
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);//绘制
		glDrawElements(GL_TRIANGLES, 150000, GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	/*
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	//键盘操作的view
	float cameraSpeed = 0.05f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)//叉乘得到camera右向量，单位化实现平移
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	*/
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
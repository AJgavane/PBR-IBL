#include <iostream>
#include<vector>
#include "display.h"
#include "shader.h"
#include "mesh.h"
#include "model.h"
#include "camera.h"
#include "Constants.h"
#include "HandelKeys.h"
#include "CubeMapFBO.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/GL.h>
#include <GL/glext.h>

void genQueries(GLuint qid[][1]);
void swapQueryBuffers();

void DrawQuadGL();

void initRendering();
void initCube();
void initSkyBox();
GLuint loadCubeMap(std::vector<std::string> faces);
void initCMfbo();
GLuint initCubeMap();
void BindCubeMapForReading();
void VerifyFBStatus(const char* str);
void printVec(const std::string& str, const glm::vec3& v);
void initShaders();
void renderCube();
void initPBR();
void initIrradianceMap();
void initPrefilterCubeMap();
void initBrdfLUTTextureMap();

int main(int args, char** argv)
{
	float dTheta = 0.1;

	glm::mat4 modelScene;
	Model scene("./res/models/sphere/sphere.obj");
	//modelScene = glm::translate(modelScene, glm::vec3(0.50f, -.800f, 2.0f));
	//modelScene = glm::translate(modelScene, glm::vec3(0.0f, 0.00f, 2.0f));
	modelScene = glm::scale(modelScene, glm::vec3(0.3f));
	runtime = true; avgNumFrames = int(glm::radians(360.0f) * 20); csv = true;  //dTheta = 0;

	glm::mat4 modelSphere;
	Model sphere("./res/models/sphere/sphere.obj");
	glm::vec3 renderPosition = sphere.GetCenter(); std::cout << renderPosition.x << ", " << renderPosition.y << ", " << renderPosition.z << std::endl;
	//modelSphere = glm::translate(modelSphere, glm::vec3(0.50f, -.800f, 2.0f));
	modelSphere = glm::translate(modelSphere, glm::vec3(-1.0, 0.0, 0.0));
	modelSphere = glm::scale(modelSphere, glm::vec3(0.50));
	renderPosition = glm::vec3(modelSphere * glm::vec4(renderPosition, 1.0));
	//printVec("renderPosition: ", renderPosition);

	initShaders();
	initPBR();	
	initIrradianceMap();
	initPrefilterCubeMap();
	initBrdfLUTTextureMap();
	
	genQueries(queryID_VIR);
	genQueries(queryID_lightPass);
	lastTime = SDL_GetTicks();
	int scrWidth, scrHeight;
	glViewport(0, 0, 512, 512);
	cameraPosition = glm::vec3(2.34368, 1.69332, -1.39803);
	lookAt = glm::vec3(-10.5786, -4.30668, 5.4588);
	lightPosition = glm::vec3(2, 1, 2);
	// Render:
	float theta = dTheta;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

	while (!display.isClosed())
	{
		
		if (printCameraCoord) {
			printVec("CameraPos: ", cameraPosition);
			printVec("Center ", lookAt);
			printCameraCoord = !printCameraCoord;
		}
		numFrames++;
		glFinish();
		handleKeys();

		Camera camera(cameraPosition, fov, (float)WIDTH, (float)HEIGHT, zNear, zFar, lookAt, bbox);
		glm::mat4 projection = camera.GetPerspProj();
		glm::mat4 view = camera.GetView();
		//glm::mat4 viewProj = camera.GetPersViewProj();

		display.Clear(0.0f, 0.0f, 0.0f, 1.0f);
		theta += dTheta;
		if (dTheta > 0) {
			//modelScene = glm::rotate(modelScene, glm::sin(dTheta), glm::vec3(0.0f, 1.0f, .0f)); // If dTheta is non-zero.
			modelScene = glm::translate(modelScene, glm::vec3(glm::sin(theta) / 5, 0.0f, 0.0f)); // If dTheta is non-zero.
		}

		/* Environment Mapping */
		glDepthFunc(GL_LEQUAL);
		pbrShader.use();
			pbrShader.setMat4("u_view", view);
			pbrShader.setMat4("u_projection", projection);
			pbrShader.setMat4("u_model", modelSphere);
			pbrShader.setVec3("u_camPos", cameraPosition);
			pbrShader.setVec3("u_lightPosition", lightPosition);
			pbrShader.setBool("isDielectric", true);
			// bind pre-computed IBL data
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
			pbrShader.setFloat("metallic", 0.9f);
			pbrShader.setFloat("roughness", 0.40f);
			sphere.Draw(pbrShader);
		pbrShader.disable();
		
		// render skybox (render as last to prevent overdraw)
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		backgroundShader.use();
			glViewport(0, 0, WIDTH, HEIGHT);
			backgroundShader.setMat4("u_projection", projection);
			backgroundShader.setMat4("u_view", view);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
			//glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
			renderCube(); //*/
		backgroundShader.disable();
		//glDepthFunc(GL_LESS);

		// render BRDF map to screen
	   /*brdfShader.use();
	   glViewport(0, 0, WIDTH, HEIGHT);
	   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	   DrawQuadGL();		
	   brdfShader.disable();//*/
		
		
		glEndQuery(GL_TIME_ELAPSED);
		/*****************************************
		 * Display time and number of points     *
		/*****************************************/
		glGetQueryObjectui64v(queryID_VIR[queryFrontBuffer][0], GL_QUERY_RESULT, &virTime);
		avgVIR_time += virTime / 1000000.0;
		if (numFrames % avgNumFrames == 0 && runtime) {
			std::cout << "PCSS: " << avgVIR_time / avgNumFrames;
			std::cout << "\tLIGHT SIZE: " << LIGHT_SIZE;
			std::cout << std::endl;
			avgVIR_time = 0;
		}
		swapQueryBuffers();
		display.Update();
	}

	// Free the buffers
	glDeleteFramebuffers(1, &m_shadowMapFBO);
	glDeleteTextures(1, &depthTexture);
	for (int i = 0; i < NumTextureUnits; i++)
	{
		glDeleteTextures(1, &m_textures[i]);
		glDeleteTextures(1, &m_samplers[i]);
	}
	return 0;
}

void initShaders()
{
	/*pbrShader = Shader("./res/pbr.vs", "./res/pbr.fs");
	pbrShader.use();
	pbrShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
	pbrShader.setFloat("ao", 1.0f);
	*/
	equirectangularToCubemapShader = Shader("./res/cubemap.vs", "./res/equirectangular_to_cubemap.fs");

	backgroundShader = Shader("./res/background.vs", "./res/background.fs");
	backgroundShader.use();
	backgroundShader.setInt("environmentMap", 0);

	irradianceShader = Shader("./res/irradiance_convolution.vs", "./res/irradiance_convolution.fs");
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);

	pbrShader = Shader("./res/pbr.vs", "./res/pbr.fs");
	pbrShader.use();
	pbrShader.setInt("irradianceMap", 0);
	pbrShader.setInt("prefilterMap", 1);
	pbrShader.setInt("brdfLUT", 2);
	pbrShader.setVec3("albedo", glm::vec3(0.6f, 0.0f, 0.0f));
	pbrShader.setFloat("ao", 1.0f);
	pbrShader.disable();

	prefilterShader = Shader("./res/prefilter.vs", "./res/prefilter.fs");

	brdfShader = Shader("./res/brdf.vs", "./res/brdf.fs");
}

void initPBR()
{
	// pbr: load the HDR environment map into 2D floating point texture
	// ---------------------------------
	std::cout << "2. pbr: load the HDR environment map" << std::endl;
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float *data = stbi_loadf("./res/hdr/hdrScene.hdr", &width, &height, &nrComponents, 0);
	std::cout << "[" << width << ", " << height << "]" << std::endl;
	if (data)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
	}
	
	
	// pbr: setup cubemap to render to and attach to framebuffer
   // ---------------------------------------------------------
	std::cout << "3. pbr: setup cubemap to render to and attach to framebuffer" << std::endl;

	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
			CUBEMAP_WIDTH, CUBEMAP_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	}
	
	// pbr: setup framebuffer
	// ----------------------
	std::cout << "1. pbr: setup framebuffer" << std::endl;
	glGenFramebuffers(1, &captureFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glGenRenderbuffers(1, &captureRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, CUBEMAP_WIDTH, CUBEMAP_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, captureFBO, 0);

	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "FBO Erro!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	std::cout << "4. pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions" << std::endl;

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	std::cout << "5. pbr: convert HDR equirectangular environment map to cubemap equivalent" << std::endl;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, captureFBO);
		equirectangularToCubemapShader.use();
		equirectangularToCubemapShader.setInt("hdrTexture", 0);
		equirectangularToCubemapShader.setMat4("u_projection", captureProjection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);

		glViewport(0, 0, CUBEMAP_WIDTH, CUBEMAP_HEIGHT); // don't forget to configure the viewport to the capture dimensions.
		
		for (unsigned int i = 0; i < 6; ++i)
		{
			equirectangularToCubemapShader.setMat4("u_view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + (int) i, envCubemap, NULL);
			GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "FBO Erro!" << std::endl;
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	equirectangularToCubemapShader.disable();
	std::cout << "************************** DONE *********************" << std::endl;
}

void initIrradianceMap()
{
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for( int i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0,
			GL_RGB, GL_FLOAT, NULL);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	
	irradianceShader.use();
	irradianceShader.setInt("environmentMap", 0);
	irradianceShader.setMat4("u_projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader.setMat4("u_view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// pbr: create a pre-filter cubemap, and re-scale capture FBO to pre-filter scale.
// --------------------------------------------------------------------------------
void initPrefilterCubeMap()
{
	glGenTextures(1, &prefilterMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minifcation filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// pbr: run a quasi monte-carlo simulation on the environment lighting to create a prefilter (cube)map.
	// ----------------------------------------------------------------------------------------------------
	prefilterShader.use();
	prefilterShader.setInt("environmentMap", 0);
	prefilterShader.setMat4("u_projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		std::cout << "roughness: " << roughness << std::endl;
		prefilterShader.setFloat("u_roughness", roughness);
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.setMat4("u_view", captureViews[i]);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	
}

void initBrdfLUTTextureMap()
{
	glGenTextures(1, &brdfLUTTexture);

	// pre-allocate enough memory for the LUT texture.
	glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
	// be sure to set wrapping mode to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

	glViewport(0, 0, 512, 512);
	brdfShader.use();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawQuadGL();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint initCubeMap()
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6; i++)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return texID;
}

void initSkyBox()
{
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

}

GLuint loadCubeMap(std::vector<std::string> faces)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		std::cout << faces[i] << ": " << width << ", " << height << std::endl;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}

void renderCube()
{
	if (cubeVAO == 0)
	{
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

void DrawQuadGL()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// call this function when initializating the OpenGL settings
void genQueries(GLuint qid[][1]) {

	glGenQueries(1, qid[queryBackBuffer]);
	glGenQueries(1, qid[queryFrontBuffer]);

	// dummy query to prevent OpenGL errors from popping out
	glQueryCounter(qid[queryFrontBuffer][0], GL_TIME_ELAPSED);
}

// aux function to keep the code simpler
void swapQueryBuffers() {

	if (queryBackBuffer) {
		queryBackBuffer = 0;
		queryFrontBuffer = 1;
	}
	else {
		queryBackBuffer = 1;
		queryFrontBuffer = 0;
	}
}

void VerifyFBStatus(const char* str)
{
	GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		std::cout << str << std::endl;
}

void printVec(const std::string& str, const glm::vec3& v)
{
	std::cout << str << " {" << v[0] << ", " << v[1] << ", " << v[2] << "}" << std::endl;
}

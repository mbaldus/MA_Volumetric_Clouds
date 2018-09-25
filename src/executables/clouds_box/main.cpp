#include <cstdio>
#include <cstdlib>
#include <cmath>

//imgui
#include <imgui-glfw/imgui.h>
#include <memory>
#include "imgui-glfw/imgui_impl_glfw_gl3.h"

//rendering
#include "rendering/GLTools.h"
#include "rendering/ShaderProgram.h"
#include "rendering/CloudRenderer.h"
#include "rendering/Texture.h"
#include "rendering/Texture3D.h"

//presets
#include "presets/Camera.h"
#include "presets/CVK_Sphere.h"
#include "presets/CVK_Cube.h"
#include "presets/SkyBox.h"
#include "presets/Quad.h"

#include "util/util.h"

/*
* Rendering Settings
*/
bool showSkybox = false;
bool showWireframe = false;

/*
* Raymarching Settings
*/
glm::vec3 boundingVolumeScale = glm::vec3(150.f, 10.f, 150.0f);
//glm::vec3 boundingVolumeScale = glm::vec3(1.f, 1.f, 1.0f);
//glm::vec3 boundingVolumeTranslate = glm::vec3(0.f,0.f,0.f);
glm::vec3 boundingVolumeTranslate = glm::vec3(0.f, 25.f, -25.f);
glm::vec3 boundingVolumeMax = boundingVolumeScale + boundingVolumeTranslate;
glm::vec3 boundingVolumeMin = -boundingVolumeScale + boundingVolumeTranslate;
int marchingSteps = 80;
int lightMarchingSteps = 6;
glm::vec3 texScaleNoise = glm::vec3(1.f, 0.5, 1.f);

/*
* Cloud Settings
*/
float coverage = 1.0f;
float cloudType = 0.2f;
float extinctionCloud = 1.f;
float windSpeed = 0.05f;
float sunpowerRayleigh = 0.2f;
glm::vec3 windDirection = glm::vec3(0.1f, 0.0f, 0.1f);
bool useWind = false;
bool useCloudHeightFromTex = false;
bool useDetails = true;
bool doLightMarch = false;

/*
 * Lighting
 */
glm::vec3 lightDirection = glm::vec3(0.5f, -0.5f, 0.5f);
glm::vec3 lightColor = glm::vec3(0.99f, 0.99f, 0.99f);
float betaM = 1.f;
float hgCoeff = 0.5f; //0 equals forward scattering -> 1 equals only backward scattering
bool moveLight = false;
bool useAmbient = true;
//glm::vec3 lightDirection = glm::vec3(170.f, 100.f, -150.f);

void updateUniforms(std::shared_ptr<ShaderProgram> shaderProgram)
{
    shaderProgram->use();
    shaderProgram->update("u_marchingSteps", marchingSteps);
    shaderProgram->update("u_lightMarchingSteps", lightMarchingSteps);
    shaderProgram->update("u_coverage", coverage);
    shaderProgram->update("u_absorptionCloud", extinctionCloud);
    shaderProgram->update("u_cloudType", cloudType);
    shaderProgram->update("u_texScale", texScaleNoise);
    shaderProgram->update("u_useWind", useWind);
    shaderProgram->update("u_windDirection", windDirection);
    shaderProgram->update("u_useCTypeFromTex", useCloudHeightFromTex);
    shaderProgram->update("u_windSpeed", windSpeed);
    shaderProgram->update("u_testValue", sunpowerRayleigh);
    shaderProgram->update("u_useDetails", useDetails);
    shaderProgram->update("u_doLightMarch", doLightMarch);
    shaderProgram->update("u_lightDirection", lightDirection);
    shaderProgram->update("u_lightColor", lightColor);
    shaderProgram->update("u_absorptionLight", betaM);
    shaderProgram->update("u_hgCoeff", hgCoeff);
    shaderProgram->update("u_usePowder", moveLight);
    shaderProgram->update("u_useAmbient", useAmbient);
//  shaderProgram->update("u_boundingVolumeScale", boundingVolumeScale);
//  shaderProgram->update("u_boundingVolumeMax", boundingVolumeMax);
//  shaderProgram->update("u_boundingVolumeMin", boundingVolumeMin);
}

int main()
{
    printf("Cloudrendering\n");

    //create window
    GLFWwindow* window = GLTools::generateWindow(1280, 720, 100, 100, "Program");

    //disable vsync
    glfwSwapInterval(0);

    //set background color
    glClearColor(0.0, 0.0, 0.0, 0.0);
    //init Camera
    Camera camera(GLTools::getWidth(window), GLTools::getHeight(window), CamMode::trackball);

    // Setup ImGui window
    ImGui_ImplGlfwGL3_Init(window, true);
    ImVec4 clear_color = ImColor(30, 115, 253);

    //####################################################################################//
    //				        Camera and Scene Settings                                     //

    std::unique_ptr<CloudRenderer> cloudRenderer(new CloudRenderer(window));

    // create MVP for rendering
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -0.1f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.f, GLTools::getRatio(window), 0.1f, 300.f);

    //Skybox
    std::unique_ptr<SkyBox> skybox(new SkyBox());
    skybox->updateCamera(view, projection);

    // Geometry
    std::unique_ptr<Cube> volume(new Cube(1));
    glm::mat4 planemodel = glm::translate(boundingVolumeTranslate);
    planemodel = glm::scale(planemodel, boundingVolumeScale);

//    auto* t_perlinWorley = new Texture3D(128, Texture_Mode::perlin_worley, 1.0f);
    std::unique_ptr<Texture3D> t_perlinWorley(new Texture3D(TEXTURES_PATH + std::string("/perlinWorley128.raw"), 128));
    std::unique_ptr<Texture3D> t_erosionNoise(new Texture3D(TEXTURES_PATH + std::string("/erosionNoise32.raw"), 32));
    std::unique_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/type+coverage.jpg")));
//    std::unique_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/vignette.jpg")));
    std::unique_ptr<Texture> t_curlnoise1(new Texture(TEXTURES_PATH + std::string("/curlnoise1.jpg")));
    std::unique_ptr<Texture> t_curlnoise2(new Texture(TEXTURES_PATH + std::string("/curlnoise2.jpg")));
    std::shared_ptr<ShaderProgram> cloudShader(new ShaderProgram("/clouds.vert", "/cloudshader_box.frag"));
    cloudShader->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, t_perlinWorley->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_perlinWorleyBase"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, t_erosionNoise->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_erosionNoise"), 2);


    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, t_weather->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_weatherTexture"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, t_curlnoise1->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_curlnoise1"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, t_curlnoise2->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_curlnoise2"), 5);

    checkGLError("tex3d init in main");
    //####################################################################################//

    cloudShader->use();
    cloudShader->update("model", planemodel);
    cloudShader->update("view", view);
    cloudShader->update("projection", projection);
    cloudShader->update("u_resolution", glm::vec2(GLTools::getWidth(window), GLTools::getHeight(window)));
    cloudShader->update("u_boundingVolumeScale", boundingVolumeScale);
    cloudShader->update("u_boundingVolumeMax", boundingVolumeMax);
    cloudShader->update("u_boundingVolumeMin", boundingVolumeMin);

    //####################################################################################//
    //                              Render Settings                                       //

    //alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // initialize renderloop fields
    int framecount = 0;
    float lastTime = 0.0f;
    float currentTime = 0.0f;
    float deltaTime = 0.0f;
    checkGLError("before render loop");
    /*
     * Renderloop
     */
    while (!glfwWindowShouldClose(window))
    {
        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // bind FBO
//      glBindFramebuffer(GL_FRAMEBUFFER, cloudRenderer->getFBOHandle());
        glClearColor(clear_color.x, clear_color.y, clear_color.z, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // Skybox
        if (showSkybox)
            skybox->render(view);

        //Geometry
        updateUniforms(cloudShader);
        cloudShader->use();
        cloudShader->update("model", planemodel);
        cloudShader->update("view", view);
        cloudShader->update("u_time", currentTime);
        cloudShader->update("u_cameraPosition", camera.getCameraPos());

//        planemodel = glm::mat4(1.f);
//        planemodel = glm::translate(boundingVolumeTranslate); // translate in y
//        planemodel = glm::scale(planemodel, boundingVolumeScale);
//        boundingVolumeMax = boundingVolumeScale + boundingVolumeTranslate;
//        boundingVolumeMin = -boundingVolumeScale + boundingVolumeTranslate;


        if (showWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            volume->render();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        volume->render();

        // prevent that ImGui rotates the camera
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (camera.getCamMode() == CamMode::trackball)
                camera.update(window, view);
            else
                camera.update(window, view, deltaTime);
        }

//        //bind Screen
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//        //render framebuffer on screenfilling quad
//        cloudRenderer->execute();

        // Setup and render ImGui

        //#############################IMGUI##################################################//
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Spacing();
        ImGui::ColorEdit3("clear color", (float*) &clear_color);
//        ImGui::SliderFloat3("Bounding Volume Size", &boundingVolumeScale.x, 1.f, 150.0f);

        ImGui::Checkbox("Wireframe", &showWireframe);
        ImGui::SameLine(100);
        ImGui::Checkbox("Skybox", &showSkybox);
        ImGui::SameLine(200);
//        ImGui::Checkbox("Noise", &useNoise);

        ImGui::Spacing();
        ImGui::Text("Raymarcher Settings");
        ImGui::SliderInt("Marchsteps", &marchingSteps, 0, 255);
        ImGui::SliderInt("Marchsteps Light", &lightMarchingSteps, 0, 32);
        ImGui::SliderFloat("Texture Scale", &texScaleNoise.x, 0.01f, 5.0f);
//        texScaleNoise.y = texScaleNoise.x;
        texScaleNoise.z = texScaleNoise.x;
        ImGui::SliderFloat("Texture Scale y", &texScaleNoise.y, 0.01f, 5.0f);

        ImGui::Spacing();

        ImGui::Text("Cloud Settings");
        ImGui::Checkbox("Wind", &useWind);
        ImGui::SliderFloat("Windspeed", &windSpeed, -1.0f, 1.f);
        ImGui::SliderFloat3("Winddirection", (float*) &windDirection, 0.0f, 1.f);
        ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f);
        ImGui::SliderFloat("Cloudtype", &cloudType, 0.0f, 1.0f);
        ImGui::SliderFloat("Absorption Cloud", &extinctionCloud, 0.01f, 1.5f);
        ImGui::SliderFloat("Testvalue", &sunpowerRayleigh, 0.f, 1.f);
        ImGui::Checkbox("Cloudtype trom texture", &useCloudHeightFromTex);
        ImGui::Checkbox("Details", &useDetails);
        ImGui::Spacing();
        ImGui::Text("Light Settings");
        ImGui::Checkbox("Lightmarch", &doLightMarch);
        ImGui::Checkbox("Powder", &moveLight);
        ImGui::SameLine(100);
        ImGui::Checkbox("Ambientlight", &useAmbient);
        ImGui::SliderFloat3("Lightdirection", (float*) &lightDirection, -1.f, 1.f);
        ImGui::ColorEdit3("Lightcolor", (float*) &lightColor);
        ImGui::SliderFloat("Absorption Light", &betaM, 0.01f, 1.5f);
        ImGui::SliderFloat("Henyey Greenstein Coeff", &hgCoeff, -1.f, 1.f);
        ImGui::Spacing();

        ImGui::Text("Values");
        ImGui::Text("Camera position: %.2f, %.2f, %.2f", camera.getCameraPos().x, camera.getCameraPos().y,
                    camera.getCameraPos().z);
//        lightDirection.x = sin(currentTime*0.5f)*0.75;
        ImGui::Render();
        //####################################################################################//
        checkGLError("renderloop");
        framecount++;
        glfwSwapBuffers(window);
        glfwPollEvents();
    };

    //cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    cloudRenderer->cleanup();
    return 0;
}
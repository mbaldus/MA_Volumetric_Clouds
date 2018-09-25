#include <cstdio>
#include <cstdlib>
#include <cmath>

//imgui
#include <imgui-glfw/imgui.h>
#include <memory>
#include <imgui-glfw/imgui_tabs/imgui_tabs.h>
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
bool applyFog = true;
float fogValue = 780.f;
bool highResolution = true;

/*
* Raymarching Settings
*/
glm::vec3 planetCenter = glm::vec3(0.f);
int planetRadius = 20000;                                                //radius of the planet
int cloudVolumeStartHeight = 20500;                                       //radius from planet center to start of sky volume
int cloudVolumeThickness =100;                                         //thickness of the sky volume where clouds appear
int cloudVolumeEndHeight = cloudVolumeStartHeight + cloudVolumeThickness;   //radius from planet center to end of sky volume
float detailLevel = 3.7f;
int marchingStepsMin = 128;
int marchingStepsMax = 128;
int lightMarchingSteps = 4;
float shapeModifier = 0.5f;
glm::vec3 texScaleNoise = glm::vec3(1.f);
float texScaleErosion = 21.f;
float texScaleCoverage = 1.f;
float texScaleCurl = 1.f;
float testvalue = 1.f;

/*
* Cloud Settings
*/
float coverage = 0.527f;
float cloudType = 1.f;
float extinctionCloud = 1.f;
float windSpeed = -0.25f;
float sunpowerRayleigh = 150.f;
float sunpowerMie = 40.f;
int highAltCloudTex = 1;
//float cloudScale = 8.f / float(cloudVolumeEndHeight);
float cloudScale = 0.001f;
glm::vec3 windDirection = glm::vec3(0.1f, 0.01f, 0.1f);
bool useWind = true;
bool useCloudHeightFromTex = true;
bool useDetails = true;
bool useHighAltClouds = true;
bool doLightMarch = false;

/*
 * Lighting
 */
glm::vec3 lightDirection = glm::vec3(0.5f, -0.5f, 1.f);
glm::vec3 lightPolar = glm::vec3(1.0f,glm::pi<float>() / 2.0f, 0.f);
glm::vec3 lightColor = glm::vec3(1.f, 0.9f, 0.85f);
float hgCoeff = 0.75f; //0 equals forward scattering -> 1 equals only backward scattering
bool moveLight = false;
bool useAmbient = true;
float ambientAmount = 460.f;
//float ambientAmount = cloudVolumeEndHeight / 8.f;
float betaM = 10.f;
glm::vec3 betaR = glm::vec3(5.4f, 9.6f, 12.3f);
float vh = 8.f;

/*
 * lightning effect
 */
bool lightning = false;
glm::vec3 lightningPos = glm::vec3(-175.f,20212.f,-87.f);
float lightningStrength = 30.f;
float lightningTime = 20.f;
float lightningThreshold = 0.5f;
int timeEventTrigger = 500;

void updateUniforms(std::shared_ptr<ShaderProgram> shaderProgram)
{
    shaderProgram->use();
    shaderProgram->update("u_skyVolumeStartHeight", cloudVolumeStartHeight);
    shaderProgram->update("u_skyVolumeEndHeight", cloudVolumeEndHeight);
    shaderProgram->update("u_skyVolumeThickness", cloudVolumeThickness);
    shaderProgram->update("u_detailLevel", detailLevel);
    shaderProgram->update("u_marchingStepsMin", marchingStepsMin);
    shaderProgram->update("u_marchingStepsMax", marchingStepsMax);
    shaderProgram->update("u_lightMarchingSteps", lightMarchingSteps);
    shaderProgram->update("u_coverage", coverage);
    shaderProgram->update("u_absorptionCloud", extinctionCloud);
    shaderProgram->update("u_cloudType", cloudType);
    shaderProgram->update("u_texScaleNoise", texScaleNoise);
    shaderProgram->update("u_texScaleErosion", texScaleErosion);
    shaderProgram->update("u_texScaleCoverage", texScaleCoverage);
    shaderProgram->update("u_texScaleCurl", texScaleCurl);
    shaderProgram->update("u_cloudScale", cloudScale);
    shaderProgram->update("u_useWind", useWind);
    shaderProgram->update("u_windDirection", windDirection);
    shaderProgram->update("u_useCTypeFromTex", useCloudHeightFromTex);
    shaderProgram->update("u_windSpeed", windSpeed);
    shaderProgram->update("u_sunpower", sunpowerRayleigh);
    shaderProgram->update("u_sunpower_mie", sunpowerMie);
    shaderProgram->update("u_useDetails", useDetails);
    shaderProgram->update("u_doLightMarch", doLightMarch);
    shaderProgram->update("u_lightDirection", lightDirection);
    shaderProgram->update("u_lightColor", lightColor);
    shaderProgram->update("u_absorptionLight", betaM);
    shaderProgram->update("u_hgCoeff", hgCoeff);
    shaderProgram->update("u_usePowder", moveLight);
    shaderProgram->update("u_useAmbient", useAmbient);
    shaderProgram->update("u_useBackClouds", useHighAltClouds);
    shaderProgram->update("u_ambientAmount", ambientAmount);
    shaderProgram->update("u_betaR", betaR);
    shaderProgram->update("u_betaM", betaM);
    shaderProgram->update("u_vh", vh);
    shaderProgram->update("u_testvalue", testvalue);
    shaderProgram->update("u_applyFog", applyFog);
    shaderProgram->update("u_fogValue", fogValue);
    shaderProgram->update("u_shapeModifier", shapeModifier);

    shaderProgram->update("u_lightning", lightning);
    shaderProgram->update("u_lightningPos", lightningPos);
    shaderProgram->update("u_lightningThreshold;", lightningThreshold);
}

int main()
{
    printf("Cloudrendering\n");

    //create window
    GLFWwindow* window = GLTools::generateWindow(1280, 720, 100, 100, "Clouds_Far");

    //disable vsync
//    glfwSwapInterval(0);

    //set background color
    glClearColor(0.77f,0.74f,0.71f,1.f);
    //init Camera
    Camera camera(GLTools::getWidth(window), GLTools::getHeight(window), CamMode::pilot);
    camera.setCameraPos(glm::vec3(float(0), float(planetRadius + 2), float(0)));
    // Setup ImGui window
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowRounding = 0.0f;
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.9f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.5f, 0.5f, 0.5f);

    //####################################################################################//
    //				        Camera and Scene Settings                                     //

    std::unique_ptr<CloudRenderer> cloudRenderer(new CloudRenderer(window));

    // create MVP for rendering
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -0.1f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
//    glm::mat4 projection = glm::perspective(glm::radians(45.f), GLTools::getRatio(window), 0.1f, 2000.f);
    glm::mat4 projection = glm::perspective(glm::radians(90.f), GLTools::getRatio(window), 10.f, 10000.f);

    //Skybox
    std::unique_ptr<SkyBox> skybox(new SkyBox());
    skybox->updateCamera(view, projection);

    // Geometry
    std::unique_ptr<Sphere> skydome(new Sphere(cloudVolumeEndHeight)); //skyvolumendheigt?
    std::unique_ptr<Sphere> skybottom(new Sphere(cloudVolumeStartHeight));
    std::unique_ptr<Sphere> planet(new Sphere(planetRadius));
    glm::mat4 skydomeModel = glm::mat4(1.f);

//    std::unique_ptr<Texture3D> t_perlinWorley128(new Texture3D(128, Texture_Mode::perlin_worley, true, 10.f));
//    std::unique_ptr<Texture3D> t_perlinWorley128(new Texture3D(128, Texture_Mode::perlin_worley, true, 8.f));
    std::unique_ptr<Texture3D> t_perlinWorley64(
            new Texture3D(TEXTURES_PATH + std::string("/perlinWorley_packed_64.raw"), 64));
    std::unique_ptr<Texture3D> t_perlinWorley128(
            new Texture3D(TEXTURES_PATH + std::string("/perlinWorley_packed_128.raw"), 128));
    std::unique_ptr<Texture3D> t_erosionNoise32(
            new Texture3D(TEXTURES_PATH + std::string("/erosionNoise_packed_32.raw"), 32));
//    std::unique_ptr<Texture3D> t_erosionNoise(new Texture3D(32,Texture_Mode::erosion_noise, true));
//    std::unique_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/type+coverage.jpg")));
//    std::shared_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/kleine_wolken.jpg")));
    std::shared_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/coverageneu.jpg")));
//    std::unique_ptr<Texture> t_weather(new Texture(TEXTURES_PATH + std::string("/testtest.jpg")));
    std::unique_ptr<Texture> t_curlnoise1(new Texture(TEXTURES_PATH + std::string("/testtest.jpg")));
    std::unique_ptr<Texture> t_backClouds1(new Texture(TEXTURES_PATH + std::string("/backclouds1.jpg")));
    std::unique_ptr<Texture> t_backClouds2(new Texture(TEXTURES_PATH + std::string("/backclouds2.jpg")));
    std::unique_ptr<Texture> t_backClouds3(new Texture(TEXTURES_PATH + std::string("/backclouds3.jpg")));
    std::shared_ptr<ShaderProgram> cloudShader(new ShaderProgram("/clouds.vert", "/cloudshader_skyMarch.frag"));
    std::shared_ptr<ShaderProgram> simpleShader(
            new ShaderProgram("/unused_shaders/depthVS.vert", "/unused_shaders/depthFS.frag"));
    checkGLError("Shader initialization");

    cloudShader->use();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, t_perlinWorley128->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_perlinWorleyBase"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, t_erosionNoise32->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_erosionNoise"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, t_weather->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_weatherTexture"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, t_curlnoise1->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_curlnoise1"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, t_backClouds1->getHandle());
    glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_backCloudTex"), 5);

    checkGLError("Texture initialization");
    //####################################################################################//

    simpleShader->use();
    simpleShader->update("model", model);
    simpleShader->update("view", view);
    simpleShader->update("projection", projection);
    cloudShader->use();
    cloudShader->update("model", skydomeModel);
    cloudShader->update("view", view);
    cloudShader->update("projection", projection);
    cloudShader->update("u_resolution", glm::vec2(GLTools::getWidth(window), GLTools::getHeight(window)));
    cloudShader->update("u_planetCenter", planetCenter);
    cloudShader->update("u_planetRadius", planetRadius);
    cloudShader->update("u_skyVolumeStartHeight", cloudVolumeStartHeight);
    cloudShader->update("u_skyVolumeThickness", cloudVolumeThickness);
    cloudShader->update("u_skyVolumeEndHeight", cloudVolumeEndHeight);

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
    static bool test = false;
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
        glClearColor(0.77f,0.74f,0.71f,1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Geometry
        updateUniforms(cloudShader);
        cloudShader->use();
//        cloudShader->update("model", skydomeModel);
        cloudShader->update("view", view);
        cloudShader->update("u_time", currentTime);
        cloudShader->update("u_cameraPosition", camera.getCameraPos());

        if (showWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            skydome->render();
            simpleShader->use();
            simpleShader->update("view", view);
            skybottom->render();
            planet->render();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        cloudShader->use();
        if (highResolution)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, t_perlinWorley128->getHandle());
            glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_perlinWorleyBase"), 1);
        } else
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_3D, t_perlinWorley64->getHandle());
            glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_perlinWorleyBase"), 1);
        }
        if (highAltCloudTex == 0)
        {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, t_backClouds1->getHandle());
            glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_backCloudTex"), 5);
        }else if(highAltCloudTex==1)
        {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, t_backClouds2->getHandle());
            glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_backCloudTex"), 5);
        }else
        {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, t_backClouds3->getHandle());
            glUniform1i(glGetUniformLocation(cloudShader->getShaderProgramHandle(), "u_backCloudTex"), 5);
        }

        skydome->render();
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
//        ImGui::ShowTestWindow(&highResolution);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("Scene Settings"))
        {
            ImGui::Checkbox("Wireframe", &showWireframe);
            ImGui::SameLine(100);
            ImGui::Checkbox("Fog", &applyFog);
            ImGui::SameLine(200);
            ImGui::Checkbox("HighRes", &highResolution);
            ImGui::SliderFloat("Fogamount", &fogValue, 1.f, 10000.f);
            ImGui::SliderInt("Volume Thickness", &cloudVolumeThickness, 10, 255);
            ImGui::SliderInt("Volume Start", &cloudVolumeStartHeight, 20000, 21000);
            cloudVolumeEndHeight = cloudVolumeStartHeight + cloudVolumeThickness;
            ImGui::Checkbox("Lightning", &lightning);
        }
        if (ImGui::CollapsingHeader("Raymarcher Settings"))
        {
            ImGui::SliderInt("Min. Marchsteps", &marchingStepsMin, 0, 255);
            ImGui::SliderInt("Max. Marchsteps", &marchingStepsMax, 0, 255);
            ImGui::SliderInt("Marchsteps Light", &lightMarchingSteps, 0, 32);
            ImGui::SliderFloat("Noise Texture Scale x z", &texScaleNoise.x, 0.01f, 5.0f);
//        texScaleNoise.y = texScaleNoise.x;
            texScaleNoise.z = texScaleNoise.x;
            ImGui::SliderFloat("Noise Texture Scale y", &texScaleNoise.y, 0.01f, 5.0f);
            ImGui::SliderFloat("Coverage Texture Scale", &texScaleCoverage, 0.01f, 25.0f);
            ImGui::SliderFloat("Curl Noise Texture Scale", &texScaleCurl, 0.01f, 25.0f);
        }
        ImGui::Text("Cloud Settings");
        ImGui::Checkbox("Wind", &useWind);
        ImGui::SliderFloat("Windspeed", &windSpeed, -1.0f, 1.f);
        ImGui::SliderFloat3("Winddirection", (float*) &windDirection, 0.0f, 1.f);
        ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f);
        ImGui::SliderFloat("Cloudtype", &cloudType, 0.0f, 1.0f);
        ImGui::SliderFloat("Absorption Cloud", &extinctionCloud, 0.01f, 1.5f);
        ImGui::DragFloat("Cloudscale", &cloudScale, 0.0001f, 0.0f, 0.0f);

        ImGui::Checkbox("Cloudtype trom texture", &useCloudHeightFromTex);
        ImGui::Checkbox("Details", &useDetails);
        ImGui::SliderFloat("Shape Modifier", &shapeModifier, 0.0f, 1.f);
        ImGui::SliderFloat("Height Erode Modifier", &detailLevel, 0.01f, 5.f);
        ImGui::SliderFloat("Erosion Texture Scale", &texScaleErosion, 0.01f, 100.0f);
        ImGui::SliderFloat("cloudheight", &testvalue, 0.01f, 1.0f);
        ImGui::Spacing();
        ImGui::Text("Light Settings");
//        ImGui::Checkbox("Lightmarch", &doLightMarch);
        ImGui::Checkbox("Movelight", &moveLight);
        ImGui::SameLine(100);
        ImGui::Checkbox("Ambientlight", &useAmbient);
        ImGui::SameLine(230);
        ImGui::Checkbox("Background Clouds", &useHighAltClouds);
        ImGui::SliderFloat("Ambient Amount", &ambientAmount, 0.0f, 3000.f);
        ImGui::SliderFloat("Lightdirection Theta",  &lightPolar.y, 0.f, glm::pi<float>());
        ImGui::SliderFloat("Lightdirection Phi",  &lightPolar.z, 0.f, 2.f * glm::pi<float>());
        lightDirection.x = lightPolar.x * sin(lightPolar.y) * sin(lightPolar.z);
        lightDirection.y = lightPolar.x * cos(lightPolar.y);
        lightDirection.z = lightPolar.x * sin(lightPolar.y) * cos(lightPolar.z);
        ImGui::ColorEdit3("Lightcolor", (float*) &lightColor);
        ImGui::SliderFloat("Henyey Greenstein Coeff", &hgCoeff, 0.f, 1.f);
        ImGui::Spacing();
        if (ImGui::CollapsingHeader("Sky Settings"))
        {
            ImGui::SliderFloat3("RayleighScattering", (float*) &betaR, 0.f, 25.f);
            ImGui::SliderFloat("Mie Scattering", &betaM, 0.01f, 20.f);
            ImGui::SliderFloat("Sunpower Ray", &sunpowerRayleigh, 0.f, 1000.f);
            ImGui::SliderFloat("Sunpower Mie", &sunpowerMie, 0.f, 1000.f);
            ImGui::SliderFloat("vh", &vh, 0.1f, 100.f);

        }
        if (ImGui::CollapsingHeader("Lightning"))
        {
            if (ImGui::Button("Lightning Effect"))
            {
                lightning = true;
//                lightningStrength = 20.f;
                lightningTime = 50.f;
                std::printf("Begin Lightning\n");
            }
            ImGui::SliderFloat("Lightning Threshold;", &lightningThreshold, 0.f, 1.f);
            ImGui::SliderFloat("Lightning Strength;", &lightningStrength, 0.f, 100.f);
        }
        if (ImGui::CollapsingHeader("Values"))
        {
            ImGui::Text("Camera position: %.2f, %.2f, %.2f", camera.getCameraPos().x, camera.getCameraPos().y,
                        camera.getCameraPos().z);
            ImGui::Text("Planetsize: %d", planetRadius);
            ImGui::Text("Atmosphere Height: %d", cloudVolumeStartHeight);
            ImGui::Text("Atmosphere Thickness: %d", cloudVolumeThickness);
        }
        if (ImGui::CollapsingHeader("Textures"))
        {
            ImGui::Text("Coverage Map");
            ImGui::Image((GLuint*) t_weather->getHandle(), ImVec2(128, 128));
            ImGui::Text("Background Clouds");
            if(ImGui::ImageButton((GLuint*) t_backClouds1->getHandle(), ImVec2(128, 128)))
                highAltCloudTex = 0;
            ImGui::SameLine(130);
            if(ImGui::ImageButton((GLuint*) t_backClouds2->getHandle(), ImVec2(128, 128)))
                highAltCloudTex = 1;
            ImGui::SameLine(260);
            if(ImGui::ImageButton((GLuint*) t_backClouds3->getHandle(), ImVec2(128, 128)))
                highAltCloudTex = 2;
        }
        if (moveLight)
            lightDirection.x = sin(currentTime * 0.5f) * 0.75;
        ImGui::Render();


        if(lightning)
        {
//            printf("Begin Lightning (trigger)");
//            lightning = true;
//          lightningStrength -= 20.f*deltaTime;
            lightningTime -= 20.f * deltaTime;
            cloudShader->update("u_lightningStrength", lightningStrength);
//            lightningPos = glm::vec3(rand_float(-200,200),rand_float(20420,20450),rand_float(-100,-300)) ;
            lightningPos = glm::vec3(rand_float(-200,200),rand_float(0,20),rand_float(-100,-300)) ;
            if (lightningTime < 0)
            {
                lightning=false;
                printf("%f,%f,%f\n", lightningPos.x,lightningPos.y,lightningPos.z);
//                timeEventTrigger = rand() % 400;
                lightningTime = 20.f;
                printf("End Lightning");
            }
        }

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
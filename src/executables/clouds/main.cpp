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

#include "util/util.h"

/*
* Rendering Settings
*/
bool applyFog = true;
float fogValue = 1500.f;
bool highResolution = true;
bool doPostProcessing = true;
bool allowLightning = false;

/*
* Raymarching/World Settings
*/
glm::vec3 planetCenter = glm::vec3(0.f);
int planetRadius = 100000;                                                 //radius of the planet
int cloudVolumeStartHeight = 100500;                                       //radius from planet center to start of sky volume
int cloudVolumeThickness = 175;                                            //thickness of the sky volume where clouds appear
int cloudVolumeEndHeight =
        cloudVolumeStartHeight + cloudVolumeThickness;                      //radius from planet center to end of sky volume
int skyDomeSize = cloudVolumeEndHeight + 500;
float detailLevel = 7.7f;
int marchingStepsMin = 128;
int marchingStepsMax = 128;
int lightMarchingSteps = 4;
float texScaleErosion = 18.f;
float texScaleCoverage = 1.f;
float testvalue = 2.f;
glm::vec3 texScaleNoise = glm::vec3(1.f);

/*
* Cloud Settings
*/
float coverage = 0.285f;
int coverageChannel = 0;
float cloudType = 0.67f;
float extinctionCloud = 0.55f;
float windSpeed = 0.18f;
glm::vec3 windDirection = glm::vec3(0.1f, -0.01f, 0.01f);
float fraction = 0.00001f;
float cloudScale = 0.001f;
bool useWind = true;
bool useCloudHeightFromTex = false;
bool useDetails = true;
bool doLightMarch = false;

/*
 * Lighting Settings
 */
glm::vec3 lightDirection = glm::vec3(0.f);
glm::vec3 lightPolar = glm::vec3(1.f, 2.2f, 0.57f);
glm::vec3 lightColor = glm::vec3(1.f, 1.0f, 1.0f);
float hgCoeff = 0.85f; //0 equals forward scattering -> 1 equals only backward scattering
bool moveLight = false;
bool useAmbient = true;
float ambientAmount = 0.15f;

/*
 * Sky Settings
 */
bool useHighAltClouds = true;
int highAltCloudTex = 1;
float highAltCloudsStrength = 0.15f;
float sunpowerRayleigh = 0.4f;
float sunpowerMie = 0.025f;
float betaM = 2.1f;
glm::vec3 betaR = glm::vec3(5.8f, 13.5f, 33.1f);

/*
 * Sheet Lightning Effect
 */
bool lightning = false;
float lightningStrength = 40.f;
float lightningTime = 20.f;
float lightningThreshold = 0.05f;
int timeEventTrigger = 500;
glm::vec3 lightningPos = glm::vec3(0.f, cloudVolumeStartHeight + 50.f, -100.f);

/*
 * Post Process
 */
float ppDecay = 1.f;
float ppDensity = 1.0f;
float ppWeight = 0.01f;
int ppNumSamples = 100;
float ppExposure = 1.0f;

void setConfig(int config)
{
    switch (config)
    {
        case (0):
            applyFog = true;
            fogValue = 1500.f;
            doPostProcessing = true;
            coverage = 0.285f;
            coverageChannel = 0;
            useCloudHeightFromTex = false;
            cloudType = 0.697f;
            extinctionCloud = 0.55f;
            windSpeed = 0.13f;
            windDirection = glm::vec3(0.1f, -0.01f, 0.01f);
            useWind = true;
            lightPolar = glm::vec3(1.f, 2.2f, 0.57f);
            lightColor = glm::vec3(1.f, 1.0f, 1.0f);
            useAmbient = true;
            ambientAmount = 0.15f;
            highAltCloudsStrength = 0.15f;
            highAltCloudTex = 1;
            betaM = 2.1f;
            betaR = glm::vec3(5.8f, 13.5f, 33.1f);
            sunpowerRayleigh = 0.4f;
            sunpowerMie = 0.025f;
            break;
        case (1):
            applyFog = true;
            fogValue = 2000.f;
            doPostProcessing = true;
            coverage = 0.285f;
            coverageChannel = 2;
            useCloudHeightFromTex = true;
            cloudType = 1.f;
            extinctionCloud = 0.55f;
            windSpeed = -0.25f;
            highAltCloudTex = 1;
            windDirection = glm::vec3(0.1f, 0.01f, 0.1f);
            useWind = true;
            lightPolar = glm::vec3(1.f, 2.2f, 0.3f);;
            lightColor = glm::vec3(1.f, 1.0f, 1.0f);
            useAmbient = true;
            ambientAmount = 0.25f;
            highAltCloudsStrength = 0.55f;
            betaM = 2.1f;
            betaR = glm::vec3(5.8f, 13.5f, 33.1f);
            sunpowerRayleigh = 1.f;
            sunpowerMie = 0.025f;
            break;
        case (2):
            applyFog = true;
            fogValue = 1200.f;
            doPostProcessing = true;
            coverage = 0.6f;
            coverageChannel = 2;
            useCloudHeightFromTex = true;
            cloudType = 1.f;
            extinctionCloud = 1.01f;
            windSpeed = -0.05f;
            highAltCloudTex = 1;
            windDirection = glm::vec3(-0.05f, 0.1f, -0.1f);
            useWind = true;
            lightPolar = glm::vec3(1.f, 1.8f, 0.3f);
            lightColor = glm::vec3(0.6, 0.6f, 0.6f);
            useAmbient = true;
            ambientAmount = 0.173f;
            highAltCloudsStrength = 0.12f;
            betaM = 0.2f;
            betaR = glm::vec3(5.8f, 13.5f, 33.1f);
            sunpowerRayleigh = 0.13f;
            sunpowerMie = 0.025f;
            break;

        case (3):
            applyFog = true;
            fogValue = 1800.f;
            doPostProcessing = true;
            coverage = 0.098f;
            coverageChannel = 0;
            useCloudHeightFromTex = true;
            cloudType = 1.f;
            extinctionCloud = 0.5f;
            windSpeed = 0.1f;
            useWind = true;
            highAltCloudTex = 1;
            windDirection = glm::vec3(0.1f, -0.05f, 0.1f);
            lightPolar = glm::vec3(1.f, 1.7f, 0.f);
            lightColor = glm::vec3(1.0f, 0.75f, 0.67f);
            useAmbient = true;
            ambientAmount = 0.2f;
            highAltCloudsStrength = 0.75f;
            betaM = 8.4f;
            betaR = glm::vec3(6.7f, 18.1f, 34.0f);
            sunpowerMie = 0.027f;
            sunpowerRayleigh = 0.136f;
            break;
        case (4):
            applyFog = true;
            fogValue = 1400.f;
            doPostProcessing = true;
            coverage = 0.186f;
            coverageChannel = 1;
            useCloudHeightFromTex = true;
            cloudType = 1.f;
            extinctionCloud = 0.55f;
            windSpeed = 0.0f;
            highAltCloudTex = 2;
            windDirection = glm::vec3(0.1f, 0.01f, 0.1f);
            useWind = false;
            lightPolar = glm::vec3(1.f, 2.5f, 0.4f);;
            lightColor = glm::vec3(0.75f, 0.75f, 0.75f);
            useAmbient = true;
            ambientAmount = 0.25f;
            highAltCloudsStrength = 0.55f;
            betaM = 20.f;
            betaR = glm::vec3(5.8f, 13.5f, 33.1f);
            sunpowerRayleigh = 1.f;
            sunpowerMie = 0.025f;
            break;
    }
}

void updateUniforms(ShaderProgram* shaderProgram)
{
    shaderProgram->use();
    shaderProgram->update("u_cloudVolumeStartHeight", cloudVolumeStartHeight);
    shaderProgram->update("u_cloudVolumeEndHeight", cloudVolumeEndHeight);
    shaderProgram->update("u_cloudVolumeThickness", cloudVolumeThickness);
    shaderProgram->update("u_skyDomeSize", skyDomeSize);
    shaderProgram->update("u_detailLevel", detailLevel);
    shaderProgram->update("u_marchingStepsMin", marchingStepsMin);
    shaderProgram->update("u_marchingStepsMax", marchingStepsMax);
    shaderProgram->update("u_lightMarchingSteps", lightMarchingSteps);
    shaderProgram->update("u_coverage", coverage);
    shaderProgram->update("u_coverageChannel", coverageChannel);
    shaderProgram->update("u_extinctionCloud", extinctionCloud);
    shaderProgram->update("u_cloudType", cloudType);
    shaderProgram->update("u_texScaleNoise", texScaleNoise);
    shaderProgram->update("u_texScaleErosion", texScaleErosion);
    shaderProgram->update("u_texScaleCoverage", texScaleCoverage);
    shaderProgram->update("u_cloudScale", cloudScale);
    shaderProgram->update("u_useWind", useWind);
    shaderProgram->update("u_windDirection", windDirection);
    shaderProgram->update("u_useCloudHeightFromTex", useCloudHeightFromTex);
    shaderProgram->update("u_windSpeed", windSpeed);
    shaderProgram->update("u_sunpower", sunpowerRayleigh);
    shaderProgram->update("u_sunpower_mie", sunpowerMie);
    shaderProgram->update("u_useDetails", useDetails);
    shaderProgram->update("u_doLightMarch", doLightMarch);
    shaderProgram->update("u_lightDirection", lightDirection);
    shaderProgram->update("u_lightColor", lightColor);
    shaderProgram->update("u_absorptionLight", betaM);
    shaderProgram->update("u_hgCoeff", hgCoeff);
    shaderProgram->update("u_useAmbient", useAmbient);
    shaderProgram->update("u_useHighAltClouds", useHighAltClouds);
    shaderProgram->update("u_highAltCloudStrength", highAltCloudsStrength);
    shaderProgram->update("u_ambientAmount", ambientAmount);
    shaderProgram->update("u_betaR", betaR);
    shaderProgram->update("u_betaM", betaM);
    shaderProgram->update("u_testvalue", testvalue);
    shaderProgram->update("u_applyFog", applyFog);
    shaderProgram->update("u_fogValue", fogValue);
    shaderProgram->update("u_lightning", lightning);
    shaderProgram->update("u_lightningPos", lightningPos);
    shaderProgram->update("u_lightningThreshold", lightningThreshold);
    shaderProgram->update("u_lightningStrength", lightningStrength);
    shaderProgram->update("u_fraction", fraction);
}

void updatePostProcessUniforms(ShaderProgram* shaderProgram)
{
    shaderProgram->use();
    shaderProgram->update("u_sunPositionWorld", -lightDirection);
    shaderProgram->update("u_density", ppDensity);
    shaderProgram->update("u_weight", ppWeight);
    shaderProgram->update("u_decay", ppDecay);
    shaderProgram->update("u_numSamples", ppNumSamples);
    shaderProgram->update("u_exposure", ppExposure);
}

int main()
{
    printf("Cloudrendering\n");

    //create window
    GLFWwindow* window = GLTools::generateWindow(1280, 720, 100, 100, "Clouds");
//    GLFWwindow* window = GLTools::generateWindow(1920, 1080, 0, 15, "Clouds");

    //disable vsync
    glfwSwapInterval(0);

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
    style->Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);

    //####################################################################################//
    //				        Camera and Scene Settings                                     //
    //init cloudRenderer
    std::unique_ptr<CloudRenderer> cloudRenderer(new CloudRenderer(window,skyDomeSize));
    //init Camera
    Camera camera(GLTools::getWidth(window), GLTools::getHeight(window), CamMode::pilot);
    camera.setCameraPos(glm::vec3(float(0), float(planetRadius + 2), float(0)));

    // create MVP for rendering
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, -0.1f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(90.f), GLTools::getRatio(window), 10.f, 180000.f);
    glm::mat4 skydomeModel = glm::mat4(1.f);

    //####################################################################################//
    // update initial uniforms
    cloudRenderer->getCloudShaderProgram()->update("model", skydomeModel);
    cloudRenderer->getCloudShaderProgram()->update("view", view);
    cloudRenderer->getCloudShaderProgram()->update("projection", projection);
    cloudRenderer->getCloudShaderProgram()->update("u_resolution", glm::vec2(GLTools::getWidth(window), GLTools::getHeight(window)));
    cloudRenderer->getCloudShaderProgram()->update("u_planetCenter", planetCenter);
    cloudRenderer->getCloudShaderProgram()->update("u_planetRadius", planetRadius);
    cloudRenderer->getCloudShaderProgram()->update("u_skyVolumeStartHeight", cloudVolumeStartHeight);
    cloudRenderer->getCloudShaderProgram()->update("u_skyVolumeThickness", cloudVolumeThickness);
    cloudRenderer->getCloudShaderProgram()->update("u_skyVolumeEndHeight", cloudVolumeEndHeight);
    cloudRenderer->getPostProcessShaderProgram()->update("projection", projection);

    //####################################################################################//
    //                              Render Settings                                       //

    //alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.f, 1.f, 1.f, 1.f);
    setConfig(1);
    // initialize renderloop fields
    int framecount = 0;
    float lastTime = 0.0f;
    float currentTime = 0.0f;
    float deltaTime = 0.0f;
    int imguiTab = 0;

    /*
     * Renderloop
     */
    while (!glfwWindowShouldClose(window))
    {
        currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // calculate lightDirection from polar coordinates
        lightDirection.x = lightPolar.x * sin(lightPolar.y) * sin(lightPolar.z);
        lightDirection.y = lightPolar.x * cos(lightPolar.y);
        lightDirection.z = lightPolar.x * sin(lightPolar.y) * cos(lightPolar.z);

        // update uniforms for first renderpass
        updateUniforms(cloudRenderer->getCloudShaderProgram());
        cloudRenderer->getCloudShaderProgram()->update("model", skydomeModel);
        cloudRenderer->getCloudShaderProgram()->update("view", view);
        cloudRenderer->getCloudShaderProgram()->update("u_time", currentTime);
        cloudRenderer->getCloudShaderProgram()->update("u_cameraPosition", camera.getCameraPos());
        cloudRenderer->getCloudShaderProgram()->use();
        cloudRenderer->setHighAltitudeCloud(highAltCloudTex);

        //bind fbo and render to texture
        cloudRenderer->executeMainRenderPass();

        if (doPostProcessing)
        {
            // update post process uniforms
            updatePostProcessUniforms(cloudRenderer->getPostProcessShaderProgram());
            cloudRenderer->getPostProcessShaderProgram()->update("u_cameraPosition", camera.getCameraPos());
            cloudRenderer->getPostProcessShaderProgram()->update("view", view);
            // bind default framebuffer and render to screenfilling quad
            cloudRenderer->executePostProcess();
        } else
            // bind default framebuffer and render to screenfilling quad
            cloudRenderer->execute();

        // Setup and render ImGui
        //#############################IMGUI##################################################//
        // prevent that ImGui rotates the camera
        if (!ImGui::GetIO().WantCaptureMouse)
        {
            if (camera.getCamMode() == CamMode::trackball)
                camera.update(window, view);
            else
                camera.update(window, view, deltaTime);
        }

        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);

        if (ImGui::Button("Scene"))
            imguiTab = 1;
        ImGui::SameLine(50);
        if (ImGui::Button("Raymarcher"))
            imguiTab = 2;
        ImGui::SameLine(127);
        if (ImGui::Button("Clouds"))
            imguiTab = 3;
        ImGui::SameLine(176);
        if (ImGui::Button("Light"))
            imguiTab = 4;
        ImGui::SameLine(218);
        if (ImGui::Button("Lightning"))
            imguiTab = 5;
        ImGui::SameLine(288);
        if (ImGui::Button("Sky"))
            imguiTab = 6;
        ImGui::SameLine(316);
        if (ImGui::Button("Textures"))
            imguiTab = 7;
        ImGui::SameLine(379);
        if (ImGui::Button("Postprocess"))
            imguiTab = 8;

        switch (imguiTab)
        {
            case (1):
                ImGui::Text("Scene Settings");
                ImGui::Checkbox("Fog", &applyFog);
                ImGui::SameLine(100);
                ImGui::Checkbox("HighRes", &highResolution);
                ImGui::SameLine(200);
                ImGui::Checkbox("Postprocessing", &doPostProcessing);
                ImGui::SliderFloat("Fogamount", &fogValue, 1.f, 10000.f);
                ImGui::SliderInt("Volume Thickness", &cloudVolumeThickness, 10, 1000);
                ImGui::SliderInt("Volume Start", &cloudVolumeStartHeight, planetRadius, 105000);
                cloudVolumeEndHeight = cloudVolumeStartHeight + cloudVolumeThickness;
                ImGui::Checkbox("Allow Lightning", &allowLightning);
                break;
            case (2):
                ImGui::Text("Raymarch Settings");
                ImGui::SliderInt("Min. Marchsteps", &marchingStepsMin, 0, 255);
                ImGui::SliderInt("Max. Marchsteps", &marchingStepsMax, 0, 255);
                ImGui::SliderInt("Marchsteps Light", &lightMarchingSteps, 0, 16);
                ImGui::SliderFloat("Noise Texture Scale", &texScaleNoise.x, 0.5f, 2.0f);
                texScaleNoise.y = texScaleNoise.x;
                texScaleNoise.z = texScaleNoise.x;
                ImGui::SliderFloat("Coverage Texture Scale", &texScaleCoverage, 0.5f, 2.0f);
                break;
            case (3):
                ImGui::Text("Cloud Settings");
                ImGui::Checkbox("Wind", &useWind);
                ImGui::SliderFloat("Windspeed", &windSpeed, -1.0f, 1.f);
                ImGui::SliderFloat3("Winddirection", (float*) &windDirection, 0.0f, 1.f);
                ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f);
                ImGui::SliderFloat("Cloudtype", &cloudType, 0.0f, 1.0f);
                ImGui::SliderFloat("Extinction Cloud", &extinctionCloud, 0.01f, 1.5f);
                ImGui::Checkbox("Cloudtype trom texture", &useCloudHeightFromTex);
                ImGui::Checkbox("Details", &useDetails);
                ImGui::SliderFloat("Height Erode Modifier", &detailLevel, 0.01f, 20.f);
                ImGui::SliderFloat("Erosion Texture Scale", &texScaleErosion, 0.01f, 100.0f);
                ImGui::SliderFloat("testValue", &testvalue, 0.f, 10.f);
                break;
            case (4):
                ImGui::Text("Light Settings");
                ImGui::Checkbox("Movelight", &moveLight);
                ImGui::SameLine(0);
                ImGui::Checkbox("Ambientlight", &useAmbient);
                ImGui::SliderFloat("Ambient Amount", &ambientAmount, 0.0f, 1.5f);
                ImGui::SliderFloat("Lightdirection Theta", &lightPolar.y, 0.f, glm::pi<float>());
                ImGui::SliderFloat("Lightdirection Phi", &lightPolar.z, -glm::pi<float>(), glm::pi<float>());
                ImGui::ColorEdit3("Lightcolor", (float*) &lightColor);
                ImGui::SliderFloat("Henyey Greenstein Coeff", &hgCoeff, 0.f, 1.f);
                break;
            case (5):
                ImGui::Text("Lightning Settings");
                ImGui::SliderFloat("Lightning Threshold;", &lightningThreshold, 0.f, 1.f);
                ImGui::SliderFloat("Lightning Strength;", &lightningStrength, 0.f, 100.f);
                if (ImGui::Button("Lightning Effect"))
                {
                    lightning = true;
                    std::printf("Triggered Lightning (Button)\n");
                }
                break;
            case (6):
                ImGui::Text("Sky Settings");
                ImGui::SliderFloat3("RayleighScattering", (float*) &betaR, 0.f, 34.f);
                ImGui::SliderFloat("Mie Scattering", &betaM, 0.01f, 20.f);
                ImGui::SliderFloat("Sunpower Ray", &sunpowerRayleigh, 0.f, 2.f);
                ImGui::SliderFloat("Sunpower Mie", &sunpowerMie, 0.f, 2.f);
                ImGui::Checkbox("High Altitude Clouds", &useHighAltClouds);
                ImGui::SliderFloat("Clouds Strength", &highAltCloudsStrength, 0.f, 1.f);
                break;
            case (7):
                ImGui::Text("Texture Settings");
                ImGui::Spacing();
                ImGui::Text("Coverage Map - Select Channel:");
                ImGui::Image((GLuint*) cloudRenderer->getWeatherTex()->getHandle(), ImVec2(128, 128));
                ImGui::SameLine(140);
                if (ImGui::Button("Red"))
                    coverageChannel = 0;
                ImGui::SameLine(0);
                if (ImGui::Button("Green"))
                    coverageChannel = 1;
                ImGui::SameLine(0);
                if (ImGui::Button("Blue"))
                    coverageChannel = 2;
                ImGui::Text("Background Clouds (Click to change)");
                if (ImGui::ImageButton((GLuint*) cloudRenderer->getCirrusTex()->getHandle(), ImVec2(128, 128)))
                    highAltCloudTex = 0;
                ImGui::SameLine(0);
                if (ImGui::ImageButton((GLuint*) cloudRenderer->getCirrostratusTex()->getHandle(), ImVec2(128, 128)))
                    highAltCloudTex = 1;
                ImGui::SameLine(0);
                if (ImGui::ImageButton((GLuint*) cloudRenderer->getCirrocumulusTex()->getHandle(), ImVec2(128, 128)))
                    highAltCloudTex = 2;
                break;
            case (8):
                ImGui::Text("Post Processing Settings");
                ImGui::SliderFloat("Density", &ppDensity, 0.f, 1.f);
                ImGui::SliderFloat("Weight", &ppWeight, 0.f, 0.1f);
                ImGui::SliderFloat("Decay", &ppDecay, 0.8f, 1.f);
                ImGui::SliderInt("Samples", &ppNumSamples, 0, 100);
                ImGui::SliderFloat("Exposure", &ppExposure, 0.0f, 2.f);
                break;

        }
        if (ImGui::CollapsingHeader("Fast Debug", ImGuiTreeNodeFlags_CollapsingHeader))
        {
            ImGui::Checkbox("Fog", &applyFog);
            ImGui::SameLine(0);
            ImGui::Checkbox("Wind", &useWind);
            ImGui::SameLine(0);
            ImGui::Checkbox("Cloudtype trom texture", &useCloudHeightFromTex);
            ImGui::SliderFloat("Fogamount", &fogValue, 1.f, 10000.f);
            ImGui::SliderFloat("Ambient Amount", &ambientAmount, 0.0f, 1.5f);
            ImGui::SliderFloat("Coverage", &coverage, 0.0f, 1.0f);
            ImGui::SliderFloat("Extinction Cloud", &extinctionCloud, 0.01f, 1.5f);
            ImGui::SliderFloat("Lightdirection Theta", &lightPolar.y, 0.f, glm::pi<float>());
            ImGui::SliderFloat("Lightdirection Phi", &lightPolar.z, -glm::pi<float>(), glm::pi<float>());
        }
        if (ImGui::CollapsingHeader("Values"))
        {
            ImGui::Text("Camera position: %.2f, %.2f, %.2f", camera.getCameraPos().x, camera.getCameraPos().y,
                        camera.getCameraPos().z);
            ImGui::Text("Light Direction: %.2f, %.2f, %.2f", lightDirection.x, lightDirection.y,
                        lightDirection.z);
            ImGui::Text("Light Polar: %.2f, %.2f, %.2f", lightPolar.x, lightPolar.y,
                        lightPolar.z);
            ImGui::Text("Planetsize: %d", planetRadius);
            ImGui::Text("Atmosphere Height: %d", cloudVolumeStartHeight);
            ImGui::Text("Atmosphere Thickness: %d", cloudVolumeThickness);
        }
        ImGui::Spacing();
        ImGui::Text("Presets:");
        if (ImGui::Button("Cloudy Day "))
            setConfig(1);
        ImGui::SameLine(0);
        if (ImGui::Button("Hazy Day"))
            setConfig(4);
        ImGui::SameLine(0);
        if (ImGui::Button("Sundown"))
            setConfig(3);
        ImGui::SameLine(0);
        if (ImGui::Button("Sheet Lightning"))
            setConfig(2);
        ImGui::SameLine(0);
        if (ImGui::Button("Default"))
            setConfig(0);
        ImGui::Render();

        //lightning calculations
        if (allowLightning)
        {
            if ((framecount % timeEventTrigger) == 0 || lightning)
            {
                lightning = true;
                lightningTime -= (rand() % 20 + 20) * deltaTime;
                lightningPos += glm::vec3(rand_float(-10, 10), rand_float(-10, 10), rand_float(-10, 10));
                if (lightningTime < 0)
                {
                    lightning = false;
                    timeEventTrigger = rand() % 350 + 30;
                    lightningTime = rand_float(15.f, 20.f);
                    lightningPos = glm::vec3(rand_float(-650.f, 650.f),
                                             rand_float(cloudVolumeStartHeight + 50.f,
                                                        cloudVolumeEndHeight - 100.f),
                                             rand_float(-10.f, -1000.f));
                }
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
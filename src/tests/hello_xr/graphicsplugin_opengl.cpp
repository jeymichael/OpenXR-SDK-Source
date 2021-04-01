// Copyright (c) 2017-2021, The Khronos Group Inc.
//
// SPDX-License-Identifier: Apache-2.0

#include "pch.h"
#include "common.h"
#include "geometry.h"
#include "graphicsplugin.h"

#ifdef XR_USE_GRAPHICS_API_OPENGL

#include <common/gfxwrapper_opengl.h>
#include <common/xr_linear.h>

namespace {
constexpr float DarkSlateGray[] = {0.184313729f, 0.309803933f, 0.309803933f, 1.0f};

static const char* VertexShaderGlsl = R"_(
    #version 410

    in vec3 VertexPos;
    in vec3 VertexColor;

    out vec3 PSVertexColor;

    uniform mat4 ModelViewProjection;

    void main() {
       gl_Position = ModelViewProjection * vec4(VertexPos, 1.0);
       PSVertexColor = VertexColor;
    }
    )_";

static const char* FragmentShaderGlsl = R"_(
    #version 410

    in vec3 PSVertexColor;
    out vec4 FragColor;

    void main() {
       FragColor = vec4(PSVertexColor, 1);
    }
    )_";

struct OpenGLGraphicsPlugin : public IGraphicsPlugin {
    OpenGLGraphicsPlugin(const std::shared_ptr<Options>& /*unused*/, const std::shared_ptr<IPlatformPlugin> /*unused*/&){};

    OpenGLGraphicsPlugin(const OpenGLGraphicsPlugin&) = delete;
    OpenGLGraphicsPlugin& operator=(const OpenGLGraphicsPlugin&) = delete;
    OpenGLGraphicsPlugin(OpenGLGraphicsPlugin&&) = delete;
    OpenGLGraphicsPlugin& operator=(OpenGLGraphicsPlugin&&) = delete;

    ~OpenGLGraphicsPlugin() override {
        if (m_swapchainFramebuffer != 0) {
            glDeleteFramebuffers(1, &m_swapchainFramebuffer);
        }
        if (m_program != 0) {
            glDeleteProgram(m_program);
        }
        if (m_vao != 0) {
            glDeleteVertexArrays(1, &m_vao);
        }
        if (m_cubeVertexBuffer != 0) {
            glDeleteBuffers(1, &m_cubeVertexBuffer);
        }
        if (m_cubeIndexBuffer != 0) {
            glDeleteBuffers(1, &m_cubeIndexBuffer);
        }

        for (auto& colorToDepth : m_colorToDepthMap) {
            if (colorToDepth.second != 0) {
                glDeleteTextures(1, &colorToDepth.second);
            }
        }
    }

    std::vector<std::string> GetInstanceExtensions() const override { return {XR_KHR_OPENGL_ENABLE_EXTENSION_NAME}; }

    ksGpuWindow window{};

    void DebugMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message) {
        (void)source;
        (void)type;
        (void)id;
        (void)severity;
        Log::Write(Log::Level::Info, "GL Debug: " + std::string(message, 0, length));
    }

    void InitializeDevice(XrInstance instance, XrSystemId systemId) override {
        // Extension function must be loaded by name
        PFN_xrGetOpenGLGraphicsRequirementsKHR pfnGetOpenGLGraphicsRequirementsKHR = nullptr;
        CHECK_XRCMD(xrGetInstanceProcAddr(instance, "xrGetOpenGLGraphicsRequirementsKHR",
                                          reinterpret_cast<PFN_xrVoidFunction*>(&pfnGetOpenGLGraphicsRequirementsKHR)));

        XrGraphicsRequirementsOpenGLKHR graphicsRequirements{XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_KHR};
        CHECK_XRCMD(pfnGetOpenGLGraphicsRequirementsKHR(instance, systemId, &graphicsRequirements));

        // Initialize the gl extensions. Note we have to open a window.
        ksDriverInstance driverInstance{};
        ksGpuQueueInfo queueInfo{};
        ksGpuSurfaceColorFormat colorFormat{KS_GPU_SURFACE_COLOR_FORMAT_B8G8R8A8};
        ksGpuSurfaceDepthFormat depthFormat{KS_GPU_SURFACE_DEPTH_FORMAT_D24};
        ksGpuSampleCount sampleCount{KS_GPU_SAMPLE_COUNT_1};
        if (!ksGpuWindow_Create(&window, &driverInstance, &queueInfo, 0, colorFormat, depthFormat, sampleCount, 640, 480, false)) {
            THROW("Unable to create GL context");
        }

        GLint major = 0;
        GLint minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);

        const XrVersion desiredApiVersion = XR_MAKE_VERSION(major, minor, 0);
        if (graphicsRequirements.minApiVersionSupported > desiredApiVersion) {
            THROW("Runtime does not support desired Graphics API and/or version");
        }

#ifdef XR_USE_PLATFORM_WIN32
        m_graphicsBinding.hDC = window.context.hDC;
        m_graphicsBinding.hGLRC = window.context.hGLRC;
#elif defined(XR_USE_PLATFORM_XLIB)
        m_graphicsBinding.xDisplay = window.context.xDisplay;
        m_graphicsBinding.visualid = window.context.visualid;
        m_graphicsBinding.glxFBConfig = window.context.glxFBConfig;
        m_graphicsBinding.glxDrawable = window.context.glxDrawable;
        m_graphicsBinding.glxContext = window.context.glxContext;
#elif defined(XR_USE_PLATFORM_XCB)
        // TODO: Still missing the platform adapter, and some items to make this usable.
        m_graphicsBinding.connection = window.connection;
        // m_graphicsBinding.screenNumber = window.context.screenNumber;
        // m_graphicsBinding.fbconfigid = window.context.fbconfigid;
        m_graphicsBinding.visualid = window.context.visualid;
        m_graphicsBinding.glxDrawable = window.context.glxDrawable;
        // m_graphicsBinding.glxContext = window.context.glxContext;
#elif defined(XR_USE_PLATFORM_WAYLAND)
        // TODO: Just need something other than NULL here for now (for validation).  Eventually need
        //       to correctly put in a valid pointer to an wl_display
        m_graphicsBinding.display = reinterpret_cast<wl_display*>(0xFFFFFFFF);
#endif

        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(
            [](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
               const void* userParam) {
                ((OpenGLGraphicsPlugin*)userParam)->DebugMessageCallback(source, type, id, severity, length, message);
            },
            this);

        InitializeResources();
    }

    void InitializeResources() {
        glGenFramebuffers(1, &m_swapchainFramebuffer);

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &VertexShaderGlsl, nullptr);
        glCompileShader(vertexShader);
        CheckShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &FragmentShaderGlsl, nullptr);
        glCompileShader(fragmentShader);
        CheckShader(fragmentShader);

        m_program = glCreateProgram();
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);
        CheckProgram(m_program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        m_modelViewProjectionUniformLocation = glGetUniformLocation(m_program, "ModelViewProjection");

        m_vertexAttribCoords = glGetAttribLocation(m_program, "VertexPos");
        m_vertexAttribColor = glGetAttribLocation(m_program, "VertexColor");

        for (int i = 0; i < 36 * 26; i++) {
            m_cubeVertices[i] = Geometry::c_cubeVertices[i];
        }
        glGenBuffers(1, &m_cubeVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVertexBuffer);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(Geometry::c_cubeVertices), Geometry::c_cubeVertices, GL_STATIC_DRAW);
        glBufferData(GL_ARRAY_BUFFER, sizeof(m_cubeVertices), m_cubeVertices, GL_DYNAMIC_DRAW);

        glGenBuffers(1, &m_cubeIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Geometry::c_cubeIndices), Geometry::c_cubeIndices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        glEnableVertexAttribArray(m_vertexAttribCoords);
        glEnableVertexAttribArray(m_vertexAttribColor);
        glBindBuffer(GL_ARRAY_BUFFER, m_cubeVertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeIndexBuffer);
        glVertexAttribPointer(m_vertexAttribCoords, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry::Vertex), nullptr);
        glVertexAttribPointer(m_vertexAttribColor, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry::Vertex),
                              reinterpret_cast<const void*>(sizeof(XrVector3f)));
    }

    void CheckShader(GLuint shader) {
        GLint r = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &r);
        if (r == GL_FALSE) {
            GLchar msg[4096] = {};
            GLsizei length;
            glGetShaderInfoLog(shader, sizeof(msg), &length, msg);
            THROW(Fmt("Compile shader failed: %s", msg));
        }
    }

    void CheckProgram(GLuint prog) {
        GLint r = 0;
        glGetProgramiv(prog, GL_LINK_STATUS, &r);
        if (r == GL_FALSE) {
            GLchar msg[4096] = {};
            GLsizei length;
            glGetProgramInfoLog(prog, sizeof(msg), &length, msg);
            THROW(Fmt("Link program failed: %s", msg));
        }
    }

    int64_t SelectColorSwapchainFormat(const std::vector<int64_t>& runtimeFormats) const override {
        // List of supported color swapchain formats.
        constexpr int64_t SupportedColorSwapchainFormats[] = {
            GL_RGB10_A2,
            GL_RGBA16F,
            // The two below should only be used as a fallback, as they are linear color formats without enough bits for color
            // depth, thus leading to banding.
            GL_RGBA8,
            GL_RGBA8_SNORM,
        };

        auto swapchainFormatIt =
            std::find_first_of(runtimeFormats.begin(), runtimeFormats.end(), std::begin(SupportedColorSwapchainFormats),
                               std::end(SupportedColorSwapchainFormats));
        if (swapchainFormatIt == runtimeFormats.end()) {
            THROW("No runtime swapchain format supported for color swapchain");
        }

        return *swapchainFormatIt;
    }

    const XrBaseInStructure* GetGraphicsBinding() const override {
        return reinterpret_cast<const XrBaseInStructure*>(&m_graphicsBinding);
    }

    std::vector<XrSwapchainImageBaseHeader*> AllocateSwapchainImageStructs(
        uint32_t capacity, const XrSwapchainCreateInfo& /*swapchainCreateInfo*/) override {
        // Allocate and initialize the buffer of image structs (must be sequential in memory for xrEnumerateSwapchainImages).
        // Return back an array of pointers to each swapchain image struct so the consumer doesn't need to know the type/size.
        std::vector<XrSwapchainImageOpenGLKHR> swapchainImageBuffer(capacity);
        std::vector<XrSwapchainImageBaseHeader*> swapchainImageBase;
        for (XrSwapchainImageOpenGLKHR& image : swapchainImageBuffer) {
            image.type = XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR;
            swapchainImageBase.push_back(reinterpret_cast<XrSwapchainImageBaseHeader*>(&image));
        }

        // Keep the buffer alive by moving it into the list of buffers.
        m_swapchainImageBuffers.push_back(std::move(swapchainImageBuffer));

        return swapchainImageBase;
    }

    uint32_t GetDepthTexture(uint32_t colorTexture) {
        // If a depth-stencil view has already been created for this back-buffer, use it.
        auto depthBufferIt = m_colorToDepthMap.find(colorTexture);
        if (depthBufferIt != m_colorToDepthMap.end()) {
            return depthBufferIt->second;
        }

        // This back-buffer has no corresponding depth-stencil texture, so create one with matching dimensions.

        GLint width;
        GLint height;
        glBindTexture(GL_TEXTURE_2D, colorTexture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

        uint32_t depthTexture;
        glGenTextures(1, &depthTexture);
        glBindTexture(GL_TEXTURE_2D, depthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

        m_colorToDepthMap.insert(std::make_pair(colorTexture, depthTexture));

        return depthTexture;
    }

    void RenderView(const XrCompositionLayerProjectionView& layerView, const XrSwapchainImageBaseHeader* swapchainImage,
                    int64_t swapchainFormat, const std::vector<Cube>& cubes) override {
        CHECK(layerView.subImage.imageArrayIndex == 0);  // Texture arrays not supported.
        UNUSED_PARM(swapchainFormat);                    // Not used in this function for now.

        glBindFramebuffer(GL_FRAMEBUFFER, m_swapchainFramebuffer);

        const uint32_t colorTexture = reinterpret_cast<const XrSwapchainImageOpenGLKHR*>(swapchainImage)->image;

        glViewport(static_cast<GLint>(layerView.subImage.imageRect.offset.x),
                   static_cast<GLint>(layerView.subImage.imageRect.offset.y),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.width),
                   static_cast<GLsizei>(layerView.subImage.imageRect.extent.height));

        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        const uint32_t depthTexture = GetDepthTexture(colorTexture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

        // Clear swapchain and depth buffer.
        glClearColor(DarkSlateGray[0], DarkSlateGray[1], DarkSlateGray[2], DarkSlateGray[3]);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Set shaders and uniform variables.
        glUseProgram(m_program);

        const auto& pose = layerView.pose;
        XrMatrix4x4f proj;
        XrMatrix4x4f_CreateProjectionFov(&proj, GRAPHICS_OPENGL, layerView.fov, 0.05f, 100.0f);
        XrMatrix4x4f toView;
        XrVector3f scale{1.f, 1.f, 1.f};
        XrMatrix4x4f_CreateTranslationRotationScale(&toView, &pose.position, &pose.orientation, &scale);
        XrMatrix4x4f view;
        XrMatrix4x4f_InvertRigidBody(&view, &toView);
        XrMatrix4x4f vp;
        XrMatrix4x4f_Multiply(&vp, &proj, &view);

        // Set cube primitive data.
        glBindVertexArray(m_vao);

        // Render each cube
        for (const Cube& cube : cubes) {
            // Compute the model-view-projection transform and set it..
            XrMatrix4x4f model;
            XrMatrix4x4f_CreateTranslationRotationScale(&model, &cube.Pose.position, &cube.Pose.orientation, &cube.Scale);
            XrMatrix4x4f mvp;
            XrMatrix4x4f_Multiply(&mvp, &vp, &model);
            glUniformMatrix4fv(m_modelViewProjectionUniformLocation, 1, GL_FALSE, reinterpret_cast<const GLfloat*>(&mvp));

            // Draw the cube.
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ArraySize(Geometry::c_cubeIndices)), GL_UNSIGNED_SHORT, nullptr);
        }

        glBindVertexArray(0);
        glUseProgram(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Swap our window every other eye for RenderDoc
        static int everyOther = 0;
        if ((everyOther++ & 1) != 0) {
            ksGpuWindow_SwapBuffers(&window);
        }
    }

    uint32_t GetSupportedSwapchainSampleCount(const XrViewConfigurationView&) override { return 1; }
    void rotate_subcube(int scube, const float degreesX, const float degreesY, const float degreesZ) {
#if 0
        std::ostringstream out;
        out.fill('0');
        out << "X: " << m_cubeVertices[0].Position.x;
        out << "  New X : " << m_cubeVertices[0].Position.x << std::endl;
        Log::Write(Log::Level::Info, out.str().c_str());
#endif
        XrMatrix4x4f txformRot;
        XrMatrix4x4f_CreateRotation(&txformRot, degreesX, degreesY, degreesZ);
        int i, iMax = 6 * 6;  // 6 vertex per face
        int vInd = scube * 6 * 6;
        for (i = 0; i < iMax; i++) {
            struct XrVector3f vtxIn = m_cubeVertices[vInd + i].Position;
            struct XrVector3f vtxOut = vtxIn;
            // Rotate around Y axis
            XrMatrix4x4f_TransformVector3f(&vtxOut, &txformRot, &vtxIn);
            m_cubeVertices[vInd + i].Position = vtxOut;
        }
    }

    void reset() {
        rotateCmd = 0;
        // memcpy of all verts from source to local data member again
        for (int i = 0; i < 36 * 26; i++) {
            m_cubeVertices[i].Position = Geometry::c_cubeVertices[i].Position;
            m_tmpVertices[i].Color = m_cubeVertices[i].Color;  // Take a copy of current colors
        }
    }
    bool increment_rotate(int dir) {
        static float rotateMin = -90.0f, rotateMax = 90.0f, rotateCurrent = 0.0f;
        if (dir < 0)
            rotateCurrent += (-0.25f);
        else
            rotateCurrent += (0.25f);
        if (((dir < 0) && (rotateCurrent < rotateMin)) || ((dir > 0) && (rotateCurrent > rotateMax))) {
            // stop the rotation and reset current
            reset();
            rotateCurrent = 0.0f;
            return false;
        }
        return true;
    }

    void copy_subCube_face_colors(int dstCube, int dstFace, int srcCube, int srcFace) {
        int srcOff = srcCube * 6 * 6 + srcFace * 6;
        int dstOff = dstCube * 6 * 6 + dstFace * 6;
        for (int j = 0; j < 6; j++) {
            m_cubeVertices[dstOff + j].Color = m_tmpVertices[srcOff + j].Color;
        }
    }

    void reset_subCube_colors(int dstInd) {
        int dstOff = dstInd * 6 * 6;
        for (int j = 0; j < 36; j++) {
            m_cubeVertices[dstOff + j].Color = Geometry::Grey;
        }
    }

    void rotate_R() {
        // Right Clockwise : Rotate -90 about X Axis
        // Blue Right SubCubes  : Axis SubCube[1] Edges SubCube[10:13] Corners SubCube[22:25]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {1, 10, 11, 12, 13, 22, 23, 24, 25};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], -0.25f, 0.f, 0.f);
        }
        if (!increment_rotate(-1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 10,11,12,13 from copy:12,13,11,10
            // xx.F1 transfers to xx.F1, 10.F2 = 12.F5, 11.F3 = 13.F5, 12.F4 = 11.F3, 13.F5 = 10.F2
            reset_subCube_colors(10);
            reset_subCube_colors(11);
            reset_subCube_colors(12);
            reset_subCube_colors(13);
            copy_subCube_face_colors(10, 1, 12, 1);
            copy_subCube_face_colors(11, 1, 13, 1);
            copy_subCube_face_colors(12, 1, 11, 1);
            copy_subCube_face_colors(13, 1, 10, 1);
            copy_subCube_face_colors(10, 2, 12, 4);
            copy_subCube_face_colors(11, 3, 13, 5);
            copy_subCube_face_colors(12, 4, 11, 3);
            copy_subCube_face_colors(13, 5, 10, 2);

            // Copy Corner subcubes colors to 22,23,24,25 from copy:24,22,25,23
            copy_subCube_face_colors(22, 2, 24, 4);
            copy_subCube_face_colors(22, 4, 24, 3);
            copy_subCube_face_colors(23, 2, 22, 4);
            copy_subCube_face_colors(23, 5, 22, 2);
            copy_subCube_face_colors(24, 3, 25, 5);
            copy_subCube_face_colors(24, 4, 25, 3);
            copy_subCube_face_colors(25, 3, 23, 5);
            copy_subCube_face_colors(25, 5, 23, 2);
        }
    }

    void rotate_L() {
        // Left Clockwise : Rotate 90 about X Axis
        // White Left SubCubes  : Axis SubCube[0] Edges SubCube[6:9] Corners SubCube[18:21]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {0, 6, 7, 8, 9, 18, 19, 20, 21};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], 0.25f, 0.f, 0.f);
        }
        if (!increment_rotate(1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 6,7,8,9 from copy:9,8,6,7
            reset_subCube_colors(6);
            reset_subCube_colors(7);
            reset_subCube_colors(8);
            reset_subCube_colors(9);
            copy_subCube_face_colors(6, 0, 9, 0);
            copy_subCube_face_colors(7, 0, 8, 0);
            copy_subCube_face_colors(8, 0, 6, 0);
            copy_subCube_face_colors(9, 0, 7, 0);
            copy_subCube_face_colors(6, 2, 9, 5);
            copy_subCube_face_colors(7, 3, 8, 4);
            copy_subCube_face_colors(8, 4, 6, 2);
            copy_subCube_face_colors(9, 5, 7, 3);

            // Copy Corner subcubes colors to 18,19,20,21 from copy:19,21,18,20
            copy_subCube_face_colors(18, 2, 19, 5);
            copy_subCube_face_colors(18, 4, 19, 2);
            copy_subCube_face_colors(19, 2, 21, 5);
            copy_subCube_face_colors(19, 5, 21, 3);
            copy_subCube_face_colors(20, 3, 18, 4);
            copy_subCube_face_colors(20, 4, 18, 2);
            copy_subCube_face_colors(21, 3, 20, 4);
            copy_subCube_face_colors(21, 5, 20, 3);
        }
    }

    void rotate_U() {
        // Up Clockwise : Rotate -90 about Y Axis
        // Yellow Up SubCubes  : Axis SubCube[3] Edges SubCube[7,11,16,17] Corners SubCube[20, 21, 24, 25]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {3, 7, 11, 16, 17, 20, 21, 24, 25};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], 0.f, -0.25f, 0.f);
        }
        if (!increment_rotate(-1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 7,11,16,17 from copy:17,16,7,11
            reset_subCube_colors(7);
            reset_subCube_colors(11);
            reset_subCube_colors(16);
            reset_subCube_colors(17);
            copy_subCube_face_colors(7, 3, 17, 3);
            copy_subCube_face_colors(11, 3, 16, 3);
            copy_subCube_face_colors(16, 3, 7, 3);
            copy_subCube_face_colors(17, 3, 11, 3);
            copy_subCube_face_colors(7, 0, 17, 5);
            copy_subCube_face_colors(11, 1, 16, 4);
            copy_subCube_face_colors(16, 4, 7, 0);
            copy_subCube_face_colors(17, 5, 11, 1);

            // Copy Corner subcubes colors to 20,21,24,25 from copy:21,25,20,24
            copy_subCube_face_colors(20, 0, 21, 5);
            copy_subCube_face_colors(20, 4, 21, 0);
            copy_subCube_face_colors(21, 0, 25, 5);
            copy_subCube_face_colors(21, 5, 25, 1);
            copy_subCube_face_colors(24, 1, 20, 4);
            copy_subCube_face_colors(24, 4, 20, 0);
            copy_subCube_face_colors(25, 1, 24, 4);
            copy_subCube_face_colors(25, 5, 24, 1);
        }
    }

    void rotate_D() {
        // Down Clockwise : Rotate 90 about Y Axis
        // Green Down SubCubes  : Axis SubCube[2] Edges SubCube[6,10,14,15] Corners SubCube[18, 19, 22, 23]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {2, 6, 10, 14, 15, 18, 19, 22, 23};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], 0.f, 0.25f, 0.f);
        }
        if (!increment_rotate(1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 6,10,14,15 from copy:14,15,10,6
            reset_subCube_colors(6);
            reset_subCube_colors(10);
            reset_subCube_colors(14);
            reset_subCube_colors(15);
            copy_subCube_face_colors(6, 2, 14, 2);
            copy_subCube_face_colors(10, 2, 15, 2);
            copy_subCube_face_colors(14, 2, 10, 2);
            copy_subCube_face_colors(15, 2, 6, 2);
            copy_subCube_face_colors(6, 0, 14, 4);
            copy_subCube_face_colors(10, 1, 15, 5);
            copy_subCube_face_colors(14, 4, 10, 1);
            copy_subCube_face_colors(15, 5, 6, 0);

            // Copy Corner subcubes colors to 18,19,22,23 from copy:22,18,23,19
            copy_subCube_face_colors(18, 0, 22, 4);
            copy_subCube_face_colors(18, 4, 22, 1);
            copy_subCube_face_colors(19, 0, 18, 4);
            copy_subCube_face_colors(19, 5, 18, 0);
            copy_subCube_face_colors(22, 1, 23, 5);
            copy_subCube_face_colors(22, 4, 23, 1);
            copy_subCube_face_colors(23, 1, 19, 5);
            copy_subCube_face_colors(23, 5, 19, 0);
        }
    }

    void rotate_F() {
        // Front Clockwise : Rotate -90 about Z Axis
        // Red Front SubCubes  : Axis SubCube[5] Edges SubCube[9,13,15,17] Corners SubCube[19, 21, 23, 25]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {5, 9, 13, 15, 17, 19, 21, 23, 25};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], 0.f, 0.f, -0.25f);
        }
        if (!increment_rotate(-1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 9,13,15,17 from copy:15,17,13,9
            reset_subCube_colors(9);
            reset_subCube_colors(13);
            reset_subCube_colors(15);
            reset_subCube_colors(17);
            copy_subCube_face_colors(9, 5, 15, 5);
            copy_subCube_face_colors(13, 5, 17, 5);
            copy_subCube_face_colors(15, 5, 13, 5);
            copy_subCube_face_colors(17, 5, 9, 5);
            copy_subCube_face_colors(9, 0, 15, 2);
            copy_subCube_face_colors(13, 1, 17, 3);
            copy_subCube_face_colors(15, 2, 13, 1);
            copy_subCube_face_colors(17, 3, 9, 0);

            // Copy Corner subcubes colors to 19,21,23,25 from copy:23,19,25,21
            copy_subCube_face_colors(19, 0, 23, 2);
            copy_subCube_face_colors(19, 2, 23, 1);
            copy_subCube_face_colors(21, 0, 19, 2);
            copy_subCube_face_colors(21, 3, 19, 0);
            copy_subCube_face_colors(23, 1, 25, 3);
            copy_subCube_face_colors(23, 2, 25, 1);
            copy_subCube_face_colors(25, 1, 21, 3);
            copy_subCube_face_colors(25, 3, 21, 0);
        }
    }

    void rotate_B() {
        // Back Clockwise : Rotate 90 about Z Axis
        // Orange Front SubCubes  : Axis SubCube[4] Edges SubCube[8,12,14,16] Corners SubCube[18, 20, 22, 24]
        // Verts of i-th subcube : [i * SUBCUBE_VMAX, (i+1) * SUBCUBE_VMAX - 1]
        if (0 == rotateCmd) return;
        int i, subcube_indices[9] = {4, 8, 12, 14, 16, 18, 20, 22, 24};
        for (i = 0; i < 9; i++) {
            rotate_subcube(subcube_indices[i], 0.f, 0.f, 0.25f);
        }
        if (!increment_rotate(1)) {
            // Rotation complete.  Reset verts (done by increment_rotate) & update colors
            // Copy Edge subcubes colors to 8,12,14,16 from copy:16,14,8,12
            reset_subCube_colors(8);
            reset_subCube_colors(12);
            reset_subCube_colors(14);
            reset_subCube_colors(16);
            copy_subCube_face_colors(8, 4, 16, 4);
            copy_subCube_face_colors(12, 4, 14, 4);
            copy_subCube_face_colors(14, 4, 8, 4);
            copy_subCube_face_colors(16, 4, 12, 4);
            copy_subCube_face_colors(8, 0, 16, 3);
            copy_subCube_face_colors(12, 1, 14, 2);
            copy_subCube_face_colors(14, 2, 8, 0);
            copy_subCube_face_colors(16, 3, 12, 1);

            // Copy Corner subcubes colors to 18,20,22,24 from copy:20,24,18,22
            copy_subCube_face_colors(18, 0, 20, 3);
            copy_subCube_face_colors(18, 2, 20, 0);
            copy_subCube_face_colors(20, 0, 24, 3);
            copy_subCube_face_colors(20, 3, 24, 1);
            copy_subCube_face_colors(22, 1, 18, 2);
            copy_subCube_face_colors(22, 2, 18, 0);
            copy_subCube_face_colors(24, 1, 22, 2);
            copy_subCube_face_colors(24, 3, 22, 1);
        }
    }

    void postRenderLayer() override {
        if (rotateCmd == 1) {
            rotate_R();
        } else if (rotateCmd == 2) {
            rotate_L();
        } else if (rotateCmd == 3) {
            rotate_U();
        } else if (rotateCmd == 4) {
            rotate_D();
        } else if (rotateCmd == 5) {
            rotate_F();
        } else if (rotateCmd == 6) {
            rotate_B();
        }
        // Call glBufferSubData and update the above vertices
        // Can also glMapBuffer and directly update GPU memory, if the sync can be handled
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_cubeVertices), m_cubeVertices);
    }

    void execCmd() override {
        if (rotateCmd == 0) {
            rotateCmd = (rand() % 6) + 1;
        }
    }

   private:
#ifdef XR_USE_PLATFORM_WIN32
    XrGraphicsBindingOpenGLWin32KHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR};
#elif defined(XR_USE_PLATFORM_XLIB)
    XrGraphicsBindingOpenGLXlibKHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR};
#elif defined(XR_USE_PLATFORM_XCB)
    XrGraphicsBindingOpenGLXcbKHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR};
#elif defined(XR_USE_PLATFORM_WAYLAND)
    XrGraphicsBindingOpenGLWaylandKHR m_graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND_KHR};
#endif

    std::list<std::vector<XrSwapchainImageOpenGLKHR>> m_swapchainImageBuffers;
    GLuint m_swapchainFramebuffer{0};
    GLuint m_program{0};
    GLint m_modelViewProjectionUniformLocation{0};
    GLint m_vertexAttribCoords{0};
    GLint m_vertexAttribColor{0};
    GLuint m_vao{0};
    GLuint m_cubeVertexBuffer{0};
    GLuint m_cubeIndexBuffer{0};

    Geometry::Vertex m_cubeVertices[36 * 26], m_tmpVertices[36 * 26];
    int rotateCmd = 0;

    // Map color buffer to associated depth buffer. This map is populated on demand.
    std::map<uint32_t, uint32_t> m_colorToDepthMap;
};
}  // namespace

std::shared_ptr<IGraphicsPlugin> CreateGraphicsPlugin_OpenGL(const std::shared_ptr<Options>& options,
                                                             std::shared_ptr<IPlatformPlugin> platformPlugin) {
    return std::make_shared<OpenGLGraphicsPlugin>(options, platformPlugin);
}

#endif

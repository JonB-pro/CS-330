#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Module 6 Milestone"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        GLuint vbo;         // Handle for the vertex buffer object
        GLuint nVertices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh1, gMesh2, gMesh3, gMesh4, gLightMesh, gLightMesh2;
    // Textures
    GLuint gTextureId1, gTextureId2, gTextureId3;
    glm::vec2 gUVScale(5.0f, 5.0f);
    // Shader program
    GLuint gObjectProgramId, gLightProgramId;

    // variable to handle ortho change
    bool perspective = false;

    // camera
    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gObjectPosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gObjectScale(1.0f);
    glm::vec3 gObjectRotation(1.0, 1.0f, 1.0f);

    // Subject and light color
    glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gKeyLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gFillLightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 gPyramidLightColor(1.0f, 1.0f, 1.0f);

    // Key light position and scale
    glm::vec3 gKeyLightPosition(10.0f, 0.0f, -10.0f);
    glm::vec3 gKeyLightScale(1.0f);
    glm::vec3 gKeyLightRotation(0.0, 1.0f, 0.0f);

    // Fill light position and scale
    glm::vec3 gFillLightPosition(-10.0f, 0.0f, 10.0f);
    glm::vec3 gFillLightScale(1.0f);
    glm::vec3 gFillLightRotation(0.0, 1.0f, 0.0f);

    // Pyramid light position and scale
    glm::vec3 gPyramidLightPosition(1.0f, 2.15f, -0.4f);
    glm::vec3 gPyramidLightScale(0.75f);
    glm::vec3 gPyramidLightRotation(0.0, 1.0f, 0.0f);
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UCreatePlane(GLMesh& mesh);
void UCreateDrawer(GLMesh& mesh);
void UCreateLegs(GLMesh& mesh);
void UCreatePyramidLight(GLMesh& mesh);
void UCreateLight(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Object Vertex Shader Source Code*/
const GLchar* objectVertexShaderSource = GLSL(440,
    layout(location = 0) in vec3 position; // Vertex data from Vertex Attrib Pointer 0
    layout(location = 1) in vec3 normal;  // Data from Vertex Attrib Pointer 1
    layout(location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // variable to transfer data to the fragment shader
    out vec3 vertexFragmentPos; // For outgoing pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // transforms vertices to clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment or pixel position in world space only (excludes view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // Gets normal vectors in world space only and excludes normal translation properties

        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Object Fragment Shader Source Code*/
const GLchar* objectFragmentShaderSource = GLSL(440,
    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor;

    uniform vec3 objectColor;
    uniform vec3 keyLightColor;
    uniform vec3 keyLightPos;
    uniform vec3 fillLightColor;
    uniform vec3 fillLightPos;
    uniform vec3 pyramidLightColor;
    uniform vec3 pyramidLightPos;
    uniform vec3 viewPosition;

    uniform sampler2D uTexture;
    uniform vec2 uvScale;

    void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
        float keyAmbientStrength = 0.1f; // Set ambient or global lighting strength.
        vec3 key = keyAmbientStrength * keyLightColor; // Generate ambient light color.
        float fillAmbientStrength = 0.1f; // Set ambient or global lighting strength.
        vec3 fill = fillAmbientStrength * fillLightColor; // Generate ambient light color.
        float pyramidAmbientStrength = 0.1f; // Set ambient or global lighting strength.
        vec3 pyramid = pyramidAmbientStrength * pyramidLightColor; // Generate ambient light color.

        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit.

        vec3 keyLightDirection = normalize(keyLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels.
        float keyImpact = max(dot(norm, keyLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light.
        vec3 keyDiffuse = keyImpact * keyLightColor; // Generate diffuse light color.

        vec3 fillLightDirection = normalize(fillLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels.
        float fillImpact = max(dot(norm, fillLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light.
        vec3 fillDiffuse = fillImpact * fillLightColor; // Generate diffuse light color.

        vec3 pyramidLightDirection = normalize(pyramidLightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels.
        float pyramidImpact = max(dot(norm, pyramidLightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light.
        vec3 pyramidDiffuse = pyramidImpact * pyramidLightColor; // Generate diffuse light color.

        //Calculate Specular lighting*/
        float specularIntensity = 0.8f; // Set specular light strength.
        float highlightSize = 16.0f; // Set specular highlight size.
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction.
        vec3 keyReflectDir = reflect(-keyLightDirection, norm);// Calculate reflection vector.
        vec3 fillReflectDir = reflect(-fillLightDirection, norm);// Calculate reflection vector.
        vec3 pyramidReflectDir = reflect(-pyramidLightDirection, norm);// Calculate reflection vector.

        //Calculate specular component.
        float keySpecularComponent = pow(max(dot(viewDir, keyReflectDir), 0.0), highlightSize);
        float fillSpecularComponent = pow(max(dot(viewDir, fillReflectDir), 0.0), highlightSize);
        float pyramidSpecularComponent = pow(max(dot(viewDir, pyramidReflectDir), 0.0), highlightSize);
        vec3 keySpecular = specularIntensity * keySpecularComponent * keyLightColor;
        vec3 fillSpecular = specularIntensity * fillSpecularComponent * fillLightColor;
        vec3 pyramidSpecular = specularIntensity * pyramidSpecularComponent * pyramidLightColor;

        // Texture holds the color to be used for all three components.
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate Phong result
        vec3 phong = (key + fill + pyramid + keyDiffuse + keySpecular + fillDiffuse + fillSpecular + pyramidDiffuse + pyramidSpecular) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU.
    }
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;
    
    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


/* Lamp Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color to the GPU

    void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh1); // Calls the function to create the Vertex Buffer Object
    UCreateDrawer(gMesh2);
    UCreatePlane(gMesh3);
    UCreateLegs(gMesh4);
    UCreateLight(gLightMesh);
    UCreatePyramidLight(gLightMesh2);

    // Create the shader programs
    if (!UCreateShaderProgram(objectVertexShaderSource, objectFragmentShaderSource, gObjectProgramId))
        return EXIT_FAILURE;
    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLightProgramId))
        return EXIT_FAILURE;

    // Load textures
    const char* texFilename1 = "Wood1.jpeg";
    if (!UCreateTexture(texFilename1, gTextureId1))
    {
        cout << "Failed to load texture " << texFilename1 << endl;
        return EXIT_FAILURE;
    }

    const char* texFilename2 = "Wood2.jpeg";
    if (!UCreateTexture(texFilename2, gTextureId2))
    {
        cout << "Failed to load texture " << texFilename2 << endl;
        return EXIT_FAILURE;
    }

    const char* texFilename3 = "Bricks.jpeg";
    if (!UCreateTexture(texFilename3, gTextureId3))
    {
        cout << "Failed to load texture " << texFilename3 << endl;
        return EXIT_FAILURE;
    }

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gObjectProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gObjectProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh1);
    UDestroyMesh(gMesh2);
    UDestroyMesh(gMesh3);
    UDestroyMesh(gMesh4);
    UDestroyMesh(gLightMesh);
    UDestroyMesh(gLightMesh2);

    // Release texture
    UDestroyTexture(gTextureId1);
    UDestroyTexture(gTextureId2);
    UDestroyTexture(gTextureId3);

    // Release shader program
    UDestroyShaderProgram(gObjectProgramId);
    UDestroyShaderProgram(gLightProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraOffset = gCamera.MovementSpeed * gDeltaTime;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.Position += gCameraUp * cameraOffset;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.Position -= gCameraUp * cameraOffset;

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        perspective = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        perspective = true;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{

    if (yoffset != 0)
    {
        gCamera.MovementSpeed += yoffset;
    }
    else
        gCamera.MovementSpeed = gCamera.MovementSpeed;

}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Function called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection;
    if (!perspective)
    {
        // p for perspective (default)
        projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
    }
    else
        // o for ortho
        projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    // OBJECTS
    //----------------
    // Activate object shader
    glUseProgram(gObjectProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gObjectPosition) * glm::scale(gObjectScale) * glm::rotate(0.0f, gObjectRotation);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gObjectProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gObjectProgramId, "view");
    GLint projLoc = glGetUniformLocation(gObjectProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the object color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gObjectProgramId, "objectColor");
    GLint keyLightColorLoc = glGetUniformLocation(gObjectProgramId, "keyLightColor");
    GLint keyLightPositionLoc = glGetUniformLocation(gObjectProgramId, "keyLightPos");
    GLint fillLightColorLoc = glGetUniformLocation(gObjectProgramId, "fillLightColor");
    GLint fillLightPositionLoc = glGetUniformLocation(gObjectProgramId, "fillLightPos");
    GLint pyramidLightColorLoc = glGetUniformLocation(gObjectProgramId, "pyramidLightColor");
    GLint pyramidLightPositionLoc = glGetUniformLocation(gObjectProgramId, "pyramidLightPos");
    GLint viewPositionLoc = glGetUniformLocation(gObjectProgramId, "viewPosition");
    
    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(keyLightColorLoc, gKeyLightColor.r, gKeyLightColor.g, gKeyLightColor.b);
    glUniform3f(keyLightPositionLoc, gKeyLightPosition.x, gKeyLightPosition.y, gKeyLightPosition.z);
    glUniform3f(fillLightColorLoc, gFillLightColor.r, gFillLightColor.g, gFillLightColor.b);
    glUniform3f(fillLightPositionLoc, gFillLightPosition.x, gFillLightPosition.y, gFillLightPosition.z);
    glUniform3f(pyramidLightColorLoc, gPyramidLightColor.r, gPyramidLightColor.g, gPyramidLightColor.b);
    glUniform3f(pyramidLightPositionLoc, gPyramidLightPosition.x, gPyramidLightPosition.y, gPyramidLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gObjectProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // Table
    glBindVertexArray(gMesh1.vao);  // Activate the VBOs contained within the mesh's VAO
    glActiveTexture(GL_TEXTURE0);    // bind textures on corresponding texture units
    glBindTexture(GL_TEXTURE_2D, gTextureId1);
    glDrawArrays(GL_TRIANGLES, 0, gMesh1.nVertices);    // Draws the triangles
    glBindVertexArray(0);   // Deactivate the Vertex Array Object
    
    // Drawer
    glBindVertexArray(gMesh2.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId2);
    glDrawArrays(GL_TRIANGLES, 0, gMesh2.nVertices);
    glBindVertexArray(0);

    // Plane(floor)
    glBindVertexArray(gMesh3.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId3);
    glDrawArrays(GL_TRIANGLES, 0, gMesh3.nVertices);
    glBindVertexArray(0);

    // Table Legs
    glBindVertexArray(gMesh4.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId1);
    glDrawArrays(GL_TRIANGLES, 0, gMesh4.nVertices);
    glBindVertexArray(0);

    // LAMPs: draw lamps
    //----------------
    // Key Light
    glBindVertexArray(gLightMesh.vao);

    glUseProgram(gLightProgramId);

    //Transform visual que for the key light source
    model = glm::translate(gKeyLightPosition) * glm::scale(gKeyLightScale) * glm::rotate(40.0f, gKeyLightRotation);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLightProgramId, "model");
    viewLoc = glGetUniformLocation(gLightProgramId, "view");
    projLoc = glGetUniformLocation(gLightProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gLightMesh.nVertices);

    // Fill Light
    //Transform visual que for the fill light source
    model = glm::translate(gFillLightPosition) * glm::scale(gFillLightScale) * glm::rotate(40.0f, gFillLightRotation);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLightProgramId, "model");
    viewLoc = glGetUniformLocation(gLightProgramId, "view");
    projLoc = glGetUniformLocation(gLightProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gLightMesh.nVertices);

    // Pyramid Light
    //Transform visual que for the fill light source
    glBindVertexArray(gLightMesh2.vao);
    model = glm::translate(gPyramidLightPosition) * glm::scale(gPyramidLightScale) * glm::rotate(10.0f, gPyramidLightRotation);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLightProgramId, "model");
    viewLoc = glGetUniformLocation(gLightProgramId, "view");
    projLoc = glGetUniformLocation(gLightProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gLightMesh2.nVertices);

    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);


    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Vertex data
    GLfloat tverts[] = {
       // bottom horizontal****************************************************
         // Right Face---------------------------------------------------------
          1.525f,   -0.65f,   1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1
          1.525f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2
          1.525f,   -0.525f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          1.525f,   -0.525f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          1.525f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2
          1.525f,   -0.525f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face---------------------------------------------------------
         -1.525f,   -0.65f,   1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower  1
         -1.525f,   -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower  2
         -1.525f,   -0.525f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower  3

         -1.525f,   -0.525f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper  1
         -1.525f,   -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper  2
         -1.525f,   -0.525f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper  3

         // Front Face---------------------------------------------------------
          1.525f,   -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1
          1.525f,   -0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
         -1.525f,   -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3

         -1.525f,   -0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
         -1.525f,   -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          1.525f,   -0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face---------------------------------------------------------
          1.525f,   -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1
          1.525f,   -0.525f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
         -1.525f,   -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3

         -1.525f,   -0.525f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
         -1.525f,   -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          1.525f,   -0.525f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face---------------------------------------------------------
          1.525f,   -0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          1.525f,   -0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
         -1.525f,   -0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

         -1.525f,   -0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          1.525f,   -0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
         -1.525f,   -0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
          1.525f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          1.525f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
         -1.525f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

         -1.525f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          1.525f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
         -1.525f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3

       // mid horizontal*******************************************************
         // Right Face---------------------------------------------------------
          1.525f,    0.65f,   1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1
          1.525f,    0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2
          1.525f,    0.525f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          1.525f,    0.525f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          1.525f,    0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2
          1.525f,    0.525f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face---------------------------------------------------------
         -1.525f,    0.65f,   1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower  1
         -1.525f,    0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower  2
         -1.525f,    0.525f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower  3

         -1.525f,    0.525f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper  1
         -1.525f,    0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper  2
         -1.525f,    0.525f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper  3

         // Front Face---------------------------------------------------------
          1.525f,    0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1
          1.525f,    0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
         -1.525f,    0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3

         -1.525f,    0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
         -1.525f,    0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          1.525f,    0.525f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face---------------------------------------------------------
          1.525f,    0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1
          1.525f,    0.525f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
         -1.525f,    0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3

         -1.525f,    0.525f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
         -1.525f,    0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          1.525f,    0.525f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face---------------------------------------------------------
          1.525f,    0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          1.525f,    0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
         -1.525f,    0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

         -1.525f,    0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          1.525f,    0.525f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
         -1.525f,    0.525f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
          1.525f,    0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          1.525f,    0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
         -1.525f,    0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

         -1.525f,    0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          1.525f,    0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
         -1.525f,    0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3

       // top horizontal******************************************************
         // Right Face---------------------------------------------------------
          1.525f,    1.65f,   1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1
          1.525f,    1.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2
          1.525f,    1.775f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          1.525f,    1.775f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          1.525f,    1.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2
          1.525f,    1.775f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face---------------------------------------------------------
         -1.525f,    1.65f,   1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower  1
         -1.525f,    1.65f,  -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower  2
         -1.525f,    1.775f,  1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower  3

         -1.525f,    1.775f, -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper  1
         -1.525f,    1.65f,  -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper  2
         -1.525f,    1.775f,  1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper  3

         // Front Face---------------------------------------------------------
          1.525f,    1.65f,   1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1
          1.525f,    1.775f,  1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
         -1.525f,    1.65f,   1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3

         -1.525f,    1.775f,  1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
         -1.525f,    1.65f,   1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          1.525f,    1.775f,  1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face---------------------------------------------------------
          1.525f,   1.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1
          1.525f,   1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
         -1.525f,   1.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3

         -1.525f,   1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
         -1.525f,   1.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          1.525f,   1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face---------------------------------------------------------
          1.525f,   1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          1.525f,   1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
         -1.525f,   1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

         -1.525f,   1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          1.525f,   1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
         -1.525f,   1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
          1.525f,   1.65f,    1.0f,  0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          1.525f,   1.65f,   -1.0f,  0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
         -1.525f,   1.65f,    1.0f,  0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

         -1.525f,   1.65f,   -1.0f,  0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          1.525f,   1.65f,   -1.0f,  0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
         -1.525f,   1.65f,    1.0f,  0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3


       // left vertical********************************************************
         // Right Face---------------------------------------------------------
         -1.525f,  -0.65f,    1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1 
         -1.525f,  -0.65f,   -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2 
         -1.525f,   1.775f,   1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

         -1.525f,   1.775f,  -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
         -1.525f,  -0.65f,   -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2 
         -1.525f,   1.775f,   1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face----------------------------------------------------------
         -1.65f,   -0.65f,    1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower   1 
         -1.65f,   -0.65f,   -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower   2 
         -1.65f,    1.775f,   1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower   3

         -1.65f,    1.775f,  -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper   1
         -1.65f,   -0.65f,   -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper   2 
         -1.65f,    1.775f,   1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper   3

         // Front Face---------------------------------------------------------
         -1.525f,  -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1 
         -1.525f,   1.775f,   1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
         -1.65f,   -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3 

         -1.65f,    1.775f,   1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
         -1.65f,   -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
         -1.525f,   1.775f,   1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face---------------------------------------------------------
         -1.525f,  -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1 
         -1.525f,   1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
         -1.65f,   -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3 

         -1.65f,    1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
         -1.65f,   -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
         -1.525f,   1.775f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face---------------------------------------------------------
         -1.525f,   1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
         -1.525f,   1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
         -1.65f,    1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

         -1.65f,    1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
         -1.525f,   1.775f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
         -1.65f,    1.775f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
         -1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
         -1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
         -1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

         -1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
         -1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
         -1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3

       // right vertical******************************************************
         // Right Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1 
          1.65f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2 
          1.65f,    1.775f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          1.65f,    1.775f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          1.65f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2 
          1.65f,    1.775f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face---------------------------------------------------------
          1.525f,  -0.65f,   1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower   1 
          1.525f,  -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower   2 
          1.525f,   1.775f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower   3

          1.525f,   1.775f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper   1
          1.525f,  -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper   2 
          1.525f,   1.775f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper   3

         // Front Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1 
          1.65f,    1.775f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
          1.525f,  -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3 

          1.525f,   1.775f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
          1.525f,  -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          1.65f,    1.775f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face--------------------------------------------------------
          1.65f,   -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1 
          1.65f,    1.775f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
          1.525f,  -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3 

          1.525f,   1.775f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
          1.525f,  -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          1.65f,    1.775f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face--------------------------------------------------------
          1.65f,    1.775f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          1.65f,    1.775f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
          1.525f,   1.775f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

          1.525f,   1.775f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          1.65f,    1.775f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
          1.525f,   1.775f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
          1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

          1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
          1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3

    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(tverts) / (sizeof(tverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(tverts), tverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateLegs(GLMesh& mesh)
{
    // Vertex data
    GLfloat dverts[] = {
        // left vertical********************************************************
          // Right Face---------------------------------------------------------
          -1.525f,  -0.65f,    1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1 
          -1.525f,  -0.65f,   -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2 
          -1.525f,   -3.0f,   1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          -1.525f,   -3.0f,  -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          -1.525f,  -0.65f,   -1.0f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2 
          -1.525f,   -3.0f,   1.0f,  1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

          // Left Face----------------------------------------------------------
          -1.65f,   -0.65f,    1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower   1 
          -1.65f,   -0.65f,   -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower   2 
          -1.65f,    -3.0f,   1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower   3

          -1.65f,    -3.0f,  -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper   1
          -1.65f,   -0.65f,   -1.0f,  -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper   2 
          -1.65f,    -3.0f,   1.0f,  -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper   3

          // Front Face---------------------------------------------------------
          -1.525f,  -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1 
          -1.525f,   -3.0f,   1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
          -1.65f,   -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3 

          -1.65f,    -3.0f,   1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
          -1.65f,   -0.65f,    1.0f,  0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          -1.525f,   -3.0f,   1.0f,  0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

          // Back Face---------------------------------------------------------
          -1.525f,  -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1 
          -1.525f,   -3.0f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
          -1.65f,   -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3 

          -1.65f,    -3.0f,  -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
          -1.65f,   -0.65f,   -1.0f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          -1.525f,   -3.0f,  -1.0f,  0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

          // Top Face---------------------------------------------------------
          -1.525f,   -3.0f,   1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          -1.525f,   -3.0f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
          -1.65f,    -3.0f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

          -1.65f,    -3.0f,  -1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          -1.525f,   -3.0f,  -1.0f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
          -1.65f,    -3.0f,   1.0f,  0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

          // Bottom Face--------------------------------------------------------
          -1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          -1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
          -1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

          -1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          -1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
          -1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3

         // right vertical******************************************************
         // Right Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Right lower  1 
          1.65f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right lower  2 
          1.65f,    -3.0f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right lower  3

          1.65f,    -3.0f, -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Right upper  1
          1.65f,   -0.65f,  -1.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Right upper  2 
          1.65f,    -3.0f,  1.0f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Right upper  3

         // Left Face---------------------------------------------------------
          1.525f,  -0.65f,   1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Left lower   1 
          1.525f,  -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left lower   2 
          1.525f,   -3.0f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left lower   3

          1.525f,   -3.0f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Left upper   1
          1.525f,  -0.65f,  -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Left upper   2 
          1.525f,   -3.0f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Left upper   3

         // Front Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Front lower  1 
          1.65f,    -3.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front lower  2
          1.525f,  -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front lower  3 

          1.525f,   -3.0f,  1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Front upper  1
          1.525f,  -0.65f,   1.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Front upper  2
          1.65f,    -3.0f,  1.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Front upper  3

         // Back Face--------------------------------------------------------
          1.65f,   -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Back lower  1 
          1.65f,    -3.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back lower  2
          1.525f,  -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back lower  3 

          1.525f,   -3.0f, -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Back upper  1
          1.525f,  -0.65f,  -1.0f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Back upper  2
          1.65f,    -3.0f, -1.0f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Back upper  3

         // Top Face--------------------------------------------------------
          1.65f,    -3.0f,  1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Top front  1
          1.65f,    -3.0f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top front  2
          1.525f,   -3.0f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top front  3

          1.525f,   -3.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Top back  1
          1.65f,    -3.0f, -1.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Top back  2
          1.525f,   -3.0f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Top back  3

         // Bottom Face--------------------------------------------------------
          1.65f,   -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Bottom front  1
          1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom front  2
          1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom front  3

          1.525f,  -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Bottom back  1
          1.65f,   -0.65f,  -1.0f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Bottom back  2
          1.525f,  -0.65f,   1.0f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Bottom back  3
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(dverts) / (sizeof(dverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(dverts), dverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateDrawer(GLMesh& mesh)
{
    // Vertex data
    GLfloat dverts[] = {
        // Vertex Positions     // Textures
        // Right Face----------------------------------------------------------
         1.5f,  -0.5f,  1.0f,    1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Drawer right lower 1
         1.5f,  -0.5f, -1.0f,    1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Drawer right lower 2
         1.5f,   0.5f,  1.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Drawer right lower 3

         1.5f,   0.5f, -1.0f,    1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Drawer right upper 1
         1.5f,  -0.5f, -1.0f,    1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Drawer right upper 2
         1.5f,   0.5f,  1.0f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Drawer right upper 3

        // Left Face----------------------------------------------------------
        -1.5f,  -0.5f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // Drawer left lower 1
        -1.5f,  -0.5f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Drawer left lower 2
        -1.5f,   0.5f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Drawer left lower 3

        -1.5f,   0.5f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f, // Drawer left upper 1
        -1.5f,  -0.5f, -1.0f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f, // Drawer left upper 2
        -1.5f,   0.5f,  1.0f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // Drawer left upper 3

        // Front Face----------------------------------------------------------
         1.5f,  -0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f, // Drawer front lower 1
         1.5f,   0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Drawer front lower 2
        -1.5f,  -0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Drawer front lower 3

        -1.5f,   0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // Drawer front upper 1
        -1.5f,  -0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // Drawer front upper 2
         1.5f,   0.5f,  1.0f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f, // Drawer front upper 3

        // Back Face----------------------------------------------------------
         1.5f,  -0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Drawer back lower 1
         1.5f,   0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Drawer back lower 2
        -1.5f,  -0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Drawer back lower 3

        -1.5f,   0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   0.0f, 1.0f, // Drawer back upper 1
        -1.5f,  -0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   0.0f, 0.0f, // Drawer back upper 2
         1.5f,   0.5f, -1.0f,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Drawer back upper 3

        // Top Face----------------------------------------------------------
         1.5f,   0.5f,  1.0f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Drawer top front 1
         1.5f,   0.5f, -1.0f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Drawer top front 2
        -1.5f,   0.5f,  1.0f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Drawer top front 3

        -1.5f,   0.5f, -1.0f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Drawer top back 1
         1.5f,   0.5f, -1.0f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Drawer top back 2
        -1.5f,   0.5f,  1.0f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Drawer top back 3

        // Bottom Face----------------------------------------------------------
         1.5f,  -0.5f,  1.0f,    0.0f, -1.0f, 0.0f,   1.0f, 0.0f, // Drawer bottom front 1
         1.5f,  -0.5f, -1.0f,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Drawer bottom front 2
        -1.5f,  -0.5f,  1.0f,    0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Drawer bottom front 3

        -1.5f,  -0.5f, -1.0f,    0.0f, -1.0f, 0.0f,   0.0f, 1.0f, // Drawer bottom back 1
         1.5f,  -0.5f, -1.0f,    0.0f, -1.0f, 0.0f,   1.0f, 1.0f, // Drawer bottom back 2
        -1.5f,  -0.5f,  1.0f,    0.0f, -1.0f, 0.0f,   0.0f, 0.0f, // Drawer bottom back 3
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(dverts) / (sizeof(dverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(dverts), dverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreatePlane(GLMesh& mesh)
{
    // Vertex data
    GLfloat planeverts[] = {
        10.0f,  -3.0f,   10.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // Plane front 1
        10.0f,  -3.0f,  -10.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Plane front 2
       -10.0f,  -3.0f,   10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Plane front 3

       -10.0f,  -3.0f,  -10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f, // Plane back 1
       -10.0f,  -3.0f,   10.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f, // Plane back 2
        10.0f,  -3.0f,  -10.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // Plane back 3
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(planeverts) / (sizeof(planeverts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeverts), planeverts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreatePyramidLight(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        //Positions           //Normals            //Texture Coords
        -0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.0f,  0.75f, 0.0f,   0.0f,  0.0f, -1.0f,  0.5f, 1.0f,

        -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.0f,  0.75f, 0.0f,   0.0f,  0.0f,  1.0f,  0.5f, 1.0f,

        -0.5f, -0.5f, -0.5f,  -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.0f,  0.75f, 0.0f,  -1.0f,  0.0f,  0.0f,  0.5f, 1.0f,

         0.5f, -0.5f,  0.5f,   1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.0f,  0.75f, 0.0f,   1.0f,  0.0f,  0.0f,  0.5f, 1.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UCreateLight(GLMesh& mesh)
{
    // Vertex data
    GLfloat verts[] = {
        //Positions           //Normals            //Texture Coords
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
       -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
       -0.5f, -0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
       -0.5f,  0.5f,  0.5f,   0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);
}

void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}
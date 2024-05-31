#define _USE_MATH_DEFINES
#define _CRT_SECURE_NO_WARNINGS

#include <GLFW/glfw3.h>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct {
    float x;
    float y;
    float z;
} Vertice;

typedef struct {
    char id;
    Vertice coord;
} Transformation;

typedef struct {
    int posVertice1;
    int posVertice2;
    int posVertice3;
} Face;


typedef struct {
    float r;
    float g;
    float b;
} Color;

int num_points = 0;

//Variável para controlar a exibição do polígono de controle
bool show_control_polygon = false;

//pontos de controle
std::vector<Vertice> controlPoints;

//vetor transformacoes (id, valor)
std::vector<Transformation> transformations;

//posicao da transformacao atual
int posTransf = 0;

//vetor dos pontos atuais
std::vector<Vertice> actualDraw;

//vetor de faces atuais
std::vector<Face> actualFaces;

//vetor dos pontos antes da transformacao
std::vector<Vertice> drawBeforeTransformation;

//vetor de faces antes da transformacao
std::vector<Face> facesBeforeTransformation;

#define TRANSLATION 't'
#define ROTATION 'r'
#define SCALING 's'
#define SHEARING 'c' //cisalhamento
#define REFLECTION 'm' //mirror
#define FACE 'f' //mirror

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void applyTranslationOnDraw() {
    for (auto& point : actualDraw) {
        point.x += transformations[posTransf].coord.x;
        //point.first += transformations[posTransf].second.first;
        point.y += transformations[posTransf].coord.y;
        //point.second += transformations[posTransf].second.second;
        point.z += transformations[posTransf].coord.z;
    }
}

void applyRotationOnDraw() {
    float pi = static_cast<float>(M_PI);
    float angle = transformations[posTransf].coord.x * (pi / 180.0); // Converter graus para radianos
    float x0, y0, z0; //ponto a ser rotacionado em torno

    std::cout << "Digite x0: ";
    std::cin >> x0;

    std::cout << "Digite y0: ";
    std::cin >> y0;

    std::cout << "Digite z0: ";
    std::cin >> z0;

    for (auto& point : actualDraw) {
        // Translação para o ponto de origem
        float x = point.x - x0;
        float y = point.y - y0;
        float z = point.z - z0;

        // Rotação em torno do eixo Z
        float new_x = x * cos(angle) - y * sin(angle);
        float new_y = x * sin(angle) + y * cos(angle);
        x = new_x;
        y = new_y;

        // Rotação em torno do eixo Y
        new_x = x * cos(angle) + z * sin(angle);
        float new_z = -x * sin(angle) + z * cos(angle);
        x = new_x;
        z = new_z;

        // Rotação em torno do eixo X
        new_y = y * cos(angle) - z * sin(angle);
        new_z = y * sin(angle) + z * cos(angle);
        y = new_y;
        z = new_z;

        // Translação de volta para a posição original
        point.x = x + x0;
        point.y = y + y0;
        point.z = z + z0;
    }
}

Vertice getDrawCentralPoint() {
    Vertice centralPoint;
    float centralCoord_x = 0.0f;
    float centralCoord_y = 0.0f;
    float centralCoord_z = 0.0f;
    for (const auto& point : drawBeforeTransformation) {
        centralCoord_x += point.x;
        centralCoord_y += point.y;
        centralCoord_z += point.z;
    }
    centralCoord_x /= drawBeforeTransformation.size();
    centralCoord_y /= drawBeforeTransformation.size();
    centralCoord_z /= drawBeforeTransformation.size();

    centralPoint.x = centralCoord_x;
    centralPoint.y = centralCoord_y;
    centralPoint.z = centralCoord_z;
    return centralPoint;
}

void applyScalingOnDraw() {
    //pegar ponto central do desenho
    Vertice centralPointDraw = getDrawCentralPoint();
    // Aplicar a transformação em relação ao centro do desenho
    for (auto& point : actualDraw) {
        point.x = centralPointDraw.x + (point.x - centralPointDraw.x) * transformations[posTransf].coord.x;
        point.y = centralPointDraw.y + (point.y - centralPointDraw.y) * transformations[posTransf].coord.y;
        point.z = centralPointDraw.z + (point.z - centralPointDraw.z) * transformations[posTransf].coord.z;
    }
}

void applyShearingOnDraw() { // modificar
    //pegar ponto central do desenho
    Vertice centralPointDraw = getDrawCentralPoint();

    // Aplicar a transformação em relação ao centro de massa
    for (auto& point : actualDraw) {
        float new_x = point.x + transformations[posTransf].coord.x * (point.y - centralPointDraw.y) + transformations[posTransf].coord.z * (point.z - centralPointDraw.z);
        float new_y = point.y + transformations[posTransf].coord.y * (point.x - centralPointDraw.x) + transformations[posTransf].coord.z * (point.z - centralPointDraw.z);
        float new_z = point.z + transformations[posTransf].coord.z * (point.x - centralPointDraw.x) + transformations[posTransf].coord.z * (point.y - centralPointDraw.y);
        point.x = new_x;
        point.y = new_y;
        point.z = new_z;
    }
}

void applyReflectionOnDraw() {
    for (auto& point : actualDraw) {
        point.x = 2 * transformations[posTransf].coord.x - point.x;
        point.y = 2 * transformations[posTransf].coord.y - point.y;
        point.z = 2 * transformations[posTransf].coord.z - point.z;
    }
}


void applyTransformation() {
    drawBeforeTransformation.assign(actualDraw.begin(), actualDraw.end());
    if (posTransf >= transformations.size()) {
        posTransf = 0;
        actualDraw.assign(controlPoints.begin(), controlPoints.end());
        return;
    }
    else if (transformations[posTransf].id == TRANSLATION) {
        applyTranslationOnDraw();

    }
    else if (transformations[posTransf].id == ROTATION) {
        applyRotationOnDraw();

    }
    else if (transformations[posTransf].id == SCALING) {
        applyScalingOnDraw();

    }
    else if (transformations[posTransf].id == SHEARING) {
        applyShearingOnDraw();
    }
    else if (transformations[posTransf].id == REFLECTION) {
        applyReflectionOnDraw();
    }
    posTransf++;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        show_control_polygon = !show_control_polygon; // vizualizar o poligono
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) { //aplicar transformacoes
        applyTransformation();
    }
}

int loadControlPoints(const char* filename) {
    float x, y, z;
    int degrees;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return -1;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') {
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                Vertice point = { x, y, z };
                controlPoints.emplace_back(point);
                num_points++;
            }
            else {
                printf("Error parsing line: %s\n", line);
            }

        }
        else if (line[0] == 't') {
            if (sscanf(line, "t %f %f %f", &x, &y, &z) == 3) {
                Vertice point = { x, y, z };
                Transformation translation = { TRANSLATION , point };
                transformations.emplace_back(translation);
            }
            else {
                printf("Error parsing line: %s\n", line);
            }

        }
        else if (line[0] == 'r') {
            if (sscanf(line, "r %d", &degrees) == 1) {
                Vertice graus = { degrees , 0, 0 };
                Transformation rotation = { ROTATION , graus };
                transformations.emplace_back(rotation);
            }
            else {
                printf("Error parsing line: %s\n", line);
            }
        }
        else if (line[0] == 's') {
            if (sscanf(line, "s %f %f %f", &x, &y, &z) == 3) {
                Vertice point = { x, y, z };
                Transformation scaling = { SCALING , point };
                transformations.emplace_back(scaling);
            }
            else {
                printf("Error parsing line: %s\n", line);
            }

        }
        else if (line[0] == 'c') {
            if (sscanf(line, "c %f %f %f", &x, &y, &z) == 3) {
                Vertice point = { x, y, z };
                Transformation shearing = { SHEARING , point };
                transformations.emplace_back(shearing);
            }
            else {
                printf("Error parsing line: %s\n", line);
            }

        }
        else if (line[0] == 'm') {
            if (sscanf(line, "m %f %f %f", &x, &y, &z) == 3) {
                Vertice point = { x, y, z };
                Transformation reflection = { REFLECTION , point };
                transformations.emplace_back(reflection);
            }
            else {
                printf("Error parsing line: %s\n", line);
            }
        }
        //implementar identificacao das face no arquivo obj
        /*
        else if (line[0] == 'f') {
            if (sscanf(line, "f ", ) == ) {
                
            }
            else {
                printf("Error parsing line: %s\n", line);
            }
        }
        */
    }
    fclose(file);

    return 0;
}


//VAI TER QUE MUDA
void draw_axes() {
    glColor3f(0.21569f, 0.62745f, 0.37255f); // verde
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(-320.0f, 0.0f);
    glVertex2f(320.0f, 0.0f);
    glVertex2f(300.0f, 20.0f);
    glVertex2f(320.0f, 0.0f);
    glVertex2f(300.0f, -20.0f);
    glVertex2f(320.0f, 0.0f);
    glEnd();


    glColor3f(0.23922f, 0.12941f, 0.76863f); // azul
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(0.0f, -240.0f);
    glVertex2f(0.0f, 240.0f);
    glVertex2f(-20.0f, 220.0f);
    glVertex2f(0.0f, 240.0f);
    glVertex2f(20.0f, 220.0f);
    glVertex2f(0.0f, 240.0f);
    glEnd();
}

int binomialCoefficient(int n, int k) {
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;

    int result = 1;
    for (int i = 1; i <= k; ++i) {
        result *= (n - i + 1);
        result /= i;
    }
    return result;
}


void draw_control_points(std::vector<Vertice> pointsForDraw) {
    glColor3f(1.0f, 0.5f, 0.8f); //rosa
    glPointSize(5.0f);

    glBegin(GL_POINTS);
    for (int i = 0; i < num_points; ++i) {
        glVertex3f(pointsForDraw[i].x, pointsForDraw[i].y, pointsForDraw[i].z);
    }
    glEnd();
}


int main(int argc, char* argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_path>" << std::endl;
        return 1;
    }
    GLFWwindow* window;

    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Modelo 3D", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //background preto
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glOrtho(-320.0, 320.0, -240.0, 240.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    //AJEITA DEPOIS TO COM SONO
    if (loadControlPoints(argv[1]) == -1) {
        return 1;
    }
    actualDraw.assign(controlPoints.begin(), controlPoints.end());
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        draw_axes();
        if (posTransf != 0) {
            draw_bezier_curve(drawBeforeTransformation, colors[1]);
        }
        draw_bezier_curve(actualDraw, colors[0]);

        if (show_control_polygon) {
            draw_control_points(actualDraw);
            draw_control_polygon(actualDraw);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
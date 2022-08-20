#include "Graphics.hpp"

void Graphics::Init() {
    float vertices[] = {
         0.0f,  0.5f, 1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vbo); // Generate 1 buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if (!(
        ReadShaderFile(vertexShader, 1, "drawing.vert") 
        && CompileShader(vertexShader)
    )) {
        return;
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (!(
        ReadShaderFile(fragmentShader, 1, "drawing.frag") 
        && CompileShader(fragmentShader)
    )) {
        return;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

    GLint colAttrib = glGetAttribLocation(shaderProgram, "color");
    glEnableVertexAttribArray(colAttrib);
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));

    startTime = std::chrono::high_resolution_clock::now();
}

void Graphics::Draw() {
    // Background Fill Color
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    auto t_now = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - startTime).count();

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

Graphics::~Graphics() {
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

bool Graphics::ReadShaderFile(GLuint shader, GLsizei count, const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file) {
        return false;
    }
    std::stringstream sstr;
    sstr << file.rdbuf();
    auto file_contents = sstr.str();
    const char* shaderSource = file_contents.c_str();

    glShaderSource(shader, count, &shaderSource, NULL);
    return true;
}

bool Graphics::CompileShader(GLuint shader) {
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        printf(buffer);

        return false;
    }
    return true;
}

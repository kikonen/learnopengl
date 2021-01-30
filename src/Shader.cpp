#include "Shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "UBO.h"

// name + geom
std::map<std::string, Shader*> shaders;


Shader* Shader::getShader(
    const Assets& assets, 
    const std::string& name, 
    const std::string& geometryType)
{
    std::string key = name + geometryType;
    Shader* shader = shaders[key];

    if (!shader) {
        shader = new Shader(assets, key, name, geometryType);
        shaders[key] = shader;
    }

    return shader;
}

Shader::Shader(
    const Assets& assets,
    const std::string& key,
    const std::string& name,
    const std::string& geometryType)
    : assets(assets),
    key(key),
    shaderName(name),
    geometryType(geometryType),
    geometryOptional(geometryType.empty())
{
    std::string basePath = assets.shadersDir + "/" + name;
    vertexShaderPath = basePath + ".vs";
    fragmentShaderPath = basePath + ".fs";
    geometryShaderPath = basePath + geometryType + ".gs";

    bindTexture = true;
}

Shader::~Shader()
{
    glDeleteProgram(id);
    id = 0;
}

const void Shader::bind()
{
    glUseProgram(id); 
}

const void Shader::unbind()
{
    glUseProgram(0);
}

int Shader::setup()
{
    if (setupDone) {
        return res;
    }
    setupDone = true;
    res = -1;

    vertexShaderSource = loadSource(vertexShaderPath, false);
    fragmentShaderSource = loadSource(fragmentShaderPath, false);
    geometryShaderSource = loadSource(geometryShaderPath, geometryOptional);

    if (vertexShaderSource.empty() || fragmentShaderSource.empty()) {
        return -1;
    }

    if (createProgram()) {
        return -1;
    }
    res = 0;
    return res;
}

GLint Shader::getUniformLoc(const std::string& name)
{
    if (uniformLocations.count(name)) {
        return uniformLocations[name];
    }

    GLint vi = glGetUniformLocation(id, name.c_str());
    uniformLocations[name] = vi;
    if (vi < 0) {
        std::cout << "SHADER::MISSING_UNIFORM: " << shaderName << " uniform=" << name << std::endl;
    }
    return vi;
}

int Shader::createProgram() {
    int success;
    char infoLog[512];

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    {
        const char* src = vertexShaderSource.c_str();
        glShaderSource(vertexShader, 1, &src, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED " << shaderName << " vert=" << vertexShaderPath << "\n" << infoLog << std::endl;
        }
    }

    // fragment shader
    int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    {
        const char* src = fragmentShaderSource.c_str();
        glShaderSource(fragmentShader, 1, &src, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED " << shaderName << " frag=" << fragmentShaderPath << "\n" << infoLog << std::endl;
        }
    }

    // geoemtry shader
    int geometryShader = -1;
    if (!geometryShaderSource.empty()) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

        const char* src = geometryShaderSource.c_str();
        glShaderSource(geometryShader, 1, &src, NULL);
        glCompileShader(geometryShader);
        // check for shader compile errors
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED " << shaderName << " frag=" << geometryShaderPath << "\n" << infoLog << std::endl;
        }
    }

    // link shaders
    id = glCreateProgram();
    glAttachShader(id, vertexShader);
    glAttachShader(id, fragmentShader);
    if (geometryShader != -1) {
        glAttachShader(id, geometryShader);
    }
    glLinkProgram(id);
    // check for linking errors
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED " << shaderName << "\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    if (geometryShader != -1) {
        glDeleteShader(geometryShader);
    }

    // NOTE KI set UBOs only once for shader
    setUBO("Matrices", UBO_MATRICES);
    setUBO("Data", UBO_DATA);
    setUBO("Lights", UBO_LIGHTS);
    setUBO("Materials", UBO_MATERIALS);

    projectionMatrix.init();
    viewMatrix.init();
    modelMatrix.init(); 
    normalMatrix.init();

    normalMap.init(); 
    shadowMap.init();

    drawInstanced.init();

    nearPlane.init(); 
    farPlane.init();

    skybox.init();

    return 0;
}

//void Shader::setFloat3(const std::string& name, float v1, float v2, float v3)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform3f(vi, v1, v2, v3);
//    }
//}

//void Shader::setVec3(const std::string& name, const glm::vec3& v)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform3f(vi, v.x, v.y, v.z);
//    }
//}
//
//void Shader::setVec4(const std::string& name, const glm::vec4& v)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform4f(vi, v.x, v.y, v.z, v.w);
//    }
//}

//void Shader::setFloat(const std::string& name, float value)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform1f(vi, value);
//    }
//}
//
void Shader::setInt(const std::string& name, int value)
{
    GLint vi = getUniformLoc(name);
    if (vi != -1) {
        glUniform1i(vi, value);
    }
}

//void Shader::setIntArray(const std::string& name, int count, const GLint* values)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform1iv(vi, count, values);
//    }
//}

//void Shader::setBool(const std::string& name, bool value)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniform1i(vi, (int)value);
//    }
//}
//
//void Shader::setMat4(const std::string& name, const glm::mat4& mat)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniformMatrix4fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
//    }
//}
//
//void Shader::setMat3(const std::string& name, const glm::mat3& mat)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniformMatrix3fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
//    }
//}
//
//void Shader::setMat2(const std::string& name, const glm::mat2& mat)
//{
//    GLint vi = getUniformLoc(name);
//    if (vi != -1) {
//        glUniformMatrix2fv(vi, 1, GL_FALSE, glm::value_ptr(mat));
//    }
//}

void Shader::setUBO(const std::string& name, unsigned int UBO)
{
    unsigned int blockIndex = glGetUniformBlockIndex(id, name.c_str());
    if (blockIndex == GL_INVALID_INDEX) {
        std::cout << "ERROR::SHADER::MISSING_UBO " << shaderName << " UBO=" << name << std::endl;
        return;
    } 
    glUniformBlockBinding(id, blockIndex, UBO);
}

/**
* Load shader file
*/
std::string Shader::loadSource(const std::string& path, bool optional) {
    std::vector<std::string> lines = loadSourceLines(path, optional);

    std::stringstream sb;
    
    for (auto line : lines) {
        sb << line << std::endl;
    }

    std::string src = sb.str();
    std::cout << "\n== " << path << " ===\n" << src << "\n--------\n";

    return src;
}

/**
* Load shader file
*/
std::vector<std::string> Shader::loadSourceLines(const std::string& path, bool optional) {
    std::ifstream file;

    std::vector<std::string> lines;

    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        std::ifstream file;
        //	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        file.exceptions(std::ifstream::badbit);
        file.open(path);

        std::string line;
        int lineNumber = 1;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string k;
            std::string v1;
            std::string v2;
            std::string v3;
            ss >> k;
            ss >> v1 >> v2 >> v3;

            if (k == "#include") {
                for (auto l : processInclude(v1, lineNumber)) {
                    lines.push_back(l);
                }
                lines.push_back("#line " + std::to_string(lineNumber + 1) + " " + std::to_string(lineNumber + 1));
            }
            else {
                lines.push_back(line);
            }
            lineNumber++;
        }

        file.close();
    }
    catch (std::ifstream::failure e) {
        if (!optional) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path << std::endl;
        }
        else {
            std::cout << "INFO::SHADER::FILE_NOT_SUCCESFULLY_READ " << shaderName << " path=" << path << std::endl;
        }
    }
    std::cout << "== " << path << std::endl;

    return lines;
}

std::vector<std::string> Shader::processInclude(const std::string& includePath, int lineNumber) 
{
    std::string path = assets.shadersDir + "/" + includePath;
    std::vector<std::string> lines = loadSourceLines(path, false);

    std::vector<std::string> result;
    result.push_back("#line 1 " + std::to_string(lineNumber));
    for (auto line : lines) {
        result.push_back(line);
    }

    return result;
}


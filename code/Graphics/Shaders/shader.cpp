#include "shader.h"

#include <iterator>
#include <ios>
#include <fstream>
#include <stdexcept>
#include <Registry/registries.h>
#include <Registry/registry.h>
#include <Graphics/Materials/materials.h>
#include <Graphics/Lighting/Lights/DirectionalLight/directionalLight.h>
#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Graphics/Lighting/Lights/SpotLight/spotLight.h>
#include <Graphics/Lighting/Lights/LightInstance/lightInstance.h>
#include <Graphics/UBOs/UBOs.h>
#include <Utilities/Logger/logger.h>
#include <Utilities/Var/var.h>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>

#endif

namespace VoxelEng {

    unsigned int shader::compileShader(const std::string& shaderSource, unsigned int type) {

        unsigned int shaderID = glCreateShader(type);
        const char* source = shaderSource.c_str();

        glShaderSource(shaderID, 1, &source, nullptr);
        glCompileShader(shaderID);

        int compileResult;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileResult);
        if (!compileResult) {

            // Errors were detected while compiling the shader.
            int length;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &length);
            char* compileLog = new char[length];
            glGetShaderInfoLog(shaderID, length, &length, compileLog);

            if (type == GL_VERTEX_SHADER)
                logger::errorLog("ERROR WHILE COMPILING VERTEX SHADER:\n" + std::string(compileLog));
            else
                logger::errorLog("ERROR WHILE COMPILING FRAGMENT SHADER:\n" + std::string(compileLog));

            delete[] compileLog;

        }

        return shaderID;

    }

    unsigned int shader::createShader(const std::string& vertexShader, const std::string& fragmentShader) {

        unsigned int shaderID = glCreateProgram();

        // Compile the two shaders and get their corresponding IDs.
        unsigned int vertexShaderID = compileShader(vertexShader, GL_VERTEX_SHADER);
        unsigned int fragmentShaderID = compileShader(fragmentShader, GL_FRAGMENT_SHADER);

        // Attach the shaders.
        glAttachShader(shaderID, vertexShaderID);
        glAttachShader(shaderID, fragmentShaderID);

        // Linking stage.
        glLinkProgram(shaderID);

        // Validation stage.
        glValidateProgram(shaderID);

        // Delete object files since we don't need them outside this method.
        glDeleteShader(vertexShaderID); 
        glDeleteShader(fragmentShaderID);

        return shaderID;

    }

    shader::shader(const std::string& name, const std::string& vertexShaderPath, const std::string& fragmentShaderPath,
        std::initializer_list<std::string> requiredUBOsNames, std::initializer_list<std::string> requiredSSBOsNames)
	    : name_(name), rendererID_(0) {

        // Load the two shaders from the files and compile them later in the Shader::createShader() private method.
        std::ifstream vertex_shader_file(vertexShaderPath);
        std::noskipws(vertex_shader_file); // Don't skip whitespaces.
        std::istream_iterator<char> vertex_It(vertex_shader_file);

        std::ifstream fragment_shader_file(fragmentShaderPath);
        std::noskipws(fragment_shader_file);
        std::istream_iterator<char> fragment_It(fragment_shader_file);

        std::istream_iterator<char> end;
        std::string vertex_shader(vertex_It, end);
        std::string fragment_shader(fragment_It, end);

        vertex_shader_file.close();
        fragment_shader_file.close();

        rendererID_ = createShader(vertex_shader, fragment_shader);

        registry<std::string, var>* UBOs = registries::get("UBOs")->pointer<registry<std::string, var>>();
        registry<std::string, var>* SSBOs = registries::get("SSBOs")->pointer<registry<std::string, var>>();
        bind();
        // Bind UBOs.
        for (auto it = requiredUBOsNames.begin(); it != requiredUBOsNames.end(); it++) {
        
            var* ubo = UBOs->get(*it);
            switch (ubo->getVarType()) {
           
            case var::varType::UBO_OF_MATERIALS:
                bindUBO(*ubo->pointer<UBO<material>>());
                break;

            case var::varType::UBO_OF_DIRECTIONALLIGHTS:
                bindUBO(*ubo->pointer<UBO<directionalLight>>());
                break;

            case var::varType::UBO_OF_POINTLIGHTS:
                bindUBO(*ubo->pointer<UBO<pointLight>>());
                break;

            case var::varType::UBO_OF_SPOTLIGHTS:
                bindUBO(*ubo->pointer<UBO<spotLight>>());
                break;

            default:
            case var::varType::UNKNOWN:
                logger::errorLog("Unsupported UBO var type specified");
                break;
            
            }
        
        }

        // Bind SSBOs.
        for (auto it = requiredSSBOsNames.begin(); it != requiredSSBOsNames.end(); it++) {

            var* ssbo = SSBOs->get(*it);
            switch (ssbo->getVarType()) {

            case var::varType::SSBO_OF_LIGHTINSTANCES:
                bindSSBO(*ssbo->pointer<SSBO<lightInstance>>());
                break;

            default:
            case var::varType::UNKNOWN:
                logger::errorLog("Unsupported SSBO var type specified");
                break;

            }

        }
        unbind();
    }

    void shader::bind() const {

        glUseProgram(rendererID_);

    }

    void shader::unbind() const {

        glUseProgram(0);

    }

    GLint shader::getUniformLocation(const std::string& name) const {

        // We use a cache to prevent getting the location N times because
        // it is a costly operation.
        if (uniformLocationCache_.find(name) != uniformLocationCache_.end()) 
            return uniformLocationCache_[name];

        int location = glGetUniformLocation(rendererID_, name.c_str());

        if (location == -1)
            logger::errorLog("Uniform " + name + " doesn't exist\n");

        uniformLocationCache_[name] = location;

        return location;

    }

    void shader::setUniform1i(const std::string& name, int i1) {

        glUniform1i(getUniformLocation(name), i1);

    }

    void shader::setUniform1iv(const std::string& name, const int* v, int vSize) {

        glUniform1iv(getUniformLocation(name), vSize, v);

    }

    void shader::setUniformVec4f(const std::string& name, float f1, float f2, float f3, float f4) {

        glUniform4f(getUniformLocation(name), f1, f2, f3, f4);

    }

    void shader::setUniformVec3f(const std::string& name, const vec3& vec) {

        glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z);

    }

    void shader::setUniformMatrix4f(const std::string& name, const glm::mat4& matrix) {

        /* 
        The third parameter means that it isn't necessary to transpose the matrix since glm
        creates matrices that OpenGL doesn't need to transpose in order to understand them.
        */
        glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &matrix[0][0]); 

    }

    shader::~shader() {

        glDeleteProgram(rendererID_);

    }

}
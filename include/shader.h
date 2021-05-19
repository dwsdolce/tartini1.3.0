#ifndef SHADER_H
#define SHADER_H

#include <QOpenGLShaderProgram>

class Shader
{
public:
    QOpenGLShaderProgram m_shaderProgram;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(QOpenGLShaderProgram* m_shaderProgram, QString vertexPath, QString fragmentPath)
    {
        // build and compile our shader program
        // ------------------------------------
        // vertex shader
        if (!m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, vertexPath)) {
            // check for shader compile errors
            qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << m_shaderProgram->log();
            throw 1;
        }

        // fragment shader
        if (!m_shaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentPath)) {
            // check for shader compile errors
            qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << m_shaderProgram->log();
            throw 1;
        }

        // link shaders
        if (!m_shaderProgram->link()) {
            // check for linking errors
            qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << m_shaderProgram->log();
            throw 1;
        }

        // glUseProram
        if (!m_shaderProgram->bind()) {
            qDebug() << "ERROR::SHADER::PROGRAM::USE_FAILED\n" << m_shaderProgram->log();
            throw 1;
        }
    }
};
#endif
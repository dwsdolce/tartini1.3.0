#ifndef MYGL_H
#define MYGL_H

#include <QCoreApplication>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class MyGL
{
public:
    static void DrawLine(QOpenGLShaderProgram& program, QOpenGLVertexArrayObject& vao, QOpenGLBuffer& vbo, QOpenGLBuffer& vbo_color, int count, int lineType, float width)
    {
        program.bind();
        program.setUniformValue("width", width);

        vao.bind();
        vbo.bind();
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        vbo_color.bind();
        program.enableAttributeArray(1);
        program.setAttributeBuffer(1, GL_FLOAT, 0, 4);

        glDrawArrays(lineType, 0, count);

        vbo.release();
        vbo_color.release();
        vao.release();
        program.release();
    }
    static void DrawLine(QOpenGLShaderProgram& program, QOpenGLVertexArrayObject& vao, QOpenGLBuffer& vbo, int count, int lineType, float width, const QColor& color)
    {
        QVector4D colorV = QVector4D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0, color.alpha() / 255.0);

        program.bind();
        program.setUniformValue("frag_color", colorV);
        program.setUniformValue("width", width);

        vao.bind();
        vbo.bind();
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

        glDrawArrays(lineType, 0, count);

        vbo.release();
        vao.release();
        program.release();
    }

    static void DrawShape(QOpenGLShaderProgram& program, QOpenGLVertexArrayObject& vao, QOpenGLBuffer& vbo, QOpenGLBuffer& vbo_color, int count, int shapeType)
    {
        program.bind();

        vao.bind();
        vbo.bind();
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 3);
        vbo_color.bind();
        program.enableAttributeArray(1);
        program.setAttributeBuffer(1, GL_FLOAT, 0, 4);

        glDrawArrays(shapeType, 0, count);

        vbo.release();
        vbo_color.release();
        vao.release();
        program.release();
    }

    static void DrawShape(QOpenGLShaderProgram& program, QOpenGLVertexArrayObject& vao, QOpenGLBuffer& vbo, int count, int shapeType, const QColor& color)
    {
        QVector4D colorV = QVector4D(color.red() / 255.0, color.green() / 255.0, color.blue() / 255.0, color.alpha() / 255.0);
        // qDebug() << "alpha = " << color.alpha();
        program.bind();
        program.setUniformValue("frag_color", colorV);

        vao.bind();
        vbo.bind();
        program.enableAttributeArray(0);
        program.setAttributeBuffer(0, GL_FLOAT, 0, 3);

        glDrawArrays(shapeType, 0, count);

        vbo.release();
        vao.release();
        program.release();
    }
};
#endif

#ifndef CAMERA_H
#define CAMERA_H

#include <QVector3D>
#include <QMatrix4x4>
#include <QtMath>
#include <QDebug>

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
    enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    // camera Attributes
    QVector3D Position;
    QVector3D Front;
    QVector3D Up;
    QVector3D Right;
    QVector3D WorldUp;

    // euler Angles
    float Yaw;
    float Pitch;

    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Default camera values
    #define YAW         -90.0f
    #define PITCH         0.0f
    #define SPEED         2.5f
    #define SENSITIVITY   0.1f
    #define ZOOM         45.0f

    // constructor with vectors
    Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
    {
        initPosition = position;
        initWorldUp = up;
        initYaw = yaw;
        initPitch = pitch;
        ResetCamera();
        updateCameraVectors();
    }

    void ResetCamera ()
    {
        Position        = initPosition;
        Front           = {0.0f, 0.0f, -1.0f};
        WorldUp         = initWorldUp;
        Yaw              = initYaw;
        Pitch            = initPitch;
        Zoom             = ZOOM;
        MovementSpeed    = SPEED;
        MouseSensitivity = SENSITIVITY;
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    QMatrix4x4 GetViewMatrix()
    {
        QMatrix4x4 view;
        view.lookAt(Position, Position + Front, Up);
        return view;
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        switch (direction) {
            case FORWARD:
                Position += Front * velocity;
                break;
            case BACKWARD:
                Position -= Front * velocity;
                break;
            case LEFT:
                Position -= Right * velocity;
                break;
            case RIGHT:
                Position += Right * velocity;
                break;
        }
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw   += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch) {
            if (Pitch > 89.0f) {
                Pitch = 89.0f;
            }
            if (Pitch < -89.0f) {
                Pitch = -89.0f;
            }
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 80.0f)
            Zoom = 80.0f;
    }

private:
    QVector3D initPosition;
    QVector3D initWorldUp;
    float initYaw;
    float initPitch;

    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        QVector3D front(cos(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)),
                        sin(qDegreesToRadians(Pitch)),
                        sin(qDegreesToRadians(Yaw)) * cos(qDegreesToRadians(Pitch)));
        Front = front.normalized();

        // also re-calculate the Right and Up vector
        Right = QVector3D::crossProduct(Front, WorldUp).normalized(); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = QVector3D::crossProduct(Right, Front).normalized();
    }
};
#endif

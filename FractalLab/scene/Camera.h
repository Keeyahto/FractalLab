#pragma once

namespace FractalLab
{
struct Vec3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct CameraState
{
    Vec3 position = { 0.0f, 0.0f, -4.0f };
    Vec3 forward = { 0.0f, 0.0f, 1.0f };
    float yawRadians = 0.0f;
    float pitchRadians = 0.0f;
    float movementSpeed = 2.5f;
};
}

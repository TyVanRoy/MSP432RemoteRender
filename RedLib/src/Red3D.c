/*
 * RedGeometry.c
 *
 *  Created on: Mar 8, 2020
 *      Author: tyvanroy
 */

#include <math.h>
#include <Red3D.h>
#include <RedLCD.h>

#define FOV_DEGREES     120.0
#define FOCAL_LENGTH    300


static Camera* GlobalCamera;
static bool isometricView;

static float FOV;

const Matrix ID_MAT = {
  1, 0, 0, 0,
  0, 1, 0, 0,
  0, 0, 1, 0,
  0, 0, 0, 1
};

void InitRed3D(void){
    float rad = (FOV_DEGREES / 360.0) * (2 * PI);
    FOV = (1.0 / tanf(rad/2.0));
}

void SetGlobalCam(Camera* cam, bool isometric){
    GlobalCamera = cam;
    isometricView = isometric;
}

inline Point WorldToScreenPoint(Point3 cameraPosition, Point3 point){
//    float f = point.z - cameraPosition.z;
//    return (Point) {(int16_t) (((point.x - cameraPosition.x) * (f / point.z)) + cameraPosition.x) + (MAX_SCREEN_X >> 1)
//        , (int16_t) (((point.y - cameraPosition.y) * (f / point.z)) + cameraPosition.y) + (MAX_SCREEN_Y >> 1)};
    float perspective = FOCAL_LENGTH / (FOCAL_LENGTH + point.z);

    float transX = (point.x * perspective) + (MAX_SCREEN_X >> 1);
    float transY = (point.y * perspective) + (MAX_SCREEN_Y >> 1);

    return (Point) { (int16_t) ((perspective * point.x) + transX), (int16_t)((perspective * point.y) + transY)};
}

inline void Print3DLine(Point3 cameraPosition, Point3 p1, Point3 p2, Line* line){
    Point a = WorldToScreenPoint(cameraPosition, p1);
    Point b = WorldToScreenPoint(cameraPosition, p2);

    line->a.x = a.x;
    line->a.y = a.y;
    line->b.x = b.x;
    line->b.y = b.y;
}

void PrintPoly(Point3 cameraPosition, Point3* points, Point3 origin, Pair* lines, uint8_t lineCount, uint16_t color, Frame_t* frame, uint8_t frameOffset, bool renderMode){
    int linesDrawn = 0;
    Point3 center;
    Point3 temp = Normalized(cameraPosition);
    for(int i = 0; i < lineCount; i++){
        if((frame->lineCount + i) >= MAX_FRAME_LINES){
            return;
        }

        center = VectorAdd(points[lines[i].a], points[lines[i].b]);
        center.x /= 2;
        center.y /= 2;
        center.z /= 2;

        if(renderMode || DotProduct(temp, Normalized(VectorSub(center, origin))) > 0){
            Print3DLine(cameraPosition, points[lines[i].a], points[lines[i].b], &(frame->lines[linesDrawn + frameOffset]));
            frame->lines[linesDrawn + frameOffset].color = color;
            linesDrawn++;
        }
    }
    frame->lineCount += linesDrawn;
}

void ClonePoly(Poly* target, Poly* source, float scale){
    ClonePoints(target->points, source->points, source->pointCount, scale);
    target->pointCount = source->pointCount;
    for(int i = 0; i < source->lineCount; i++){
        target->lines[i].a = source->lines[i].a;
        target->lines[i].b = source->lines[i].b;
    }
    target->lineCount = source->lineCount;
}

void ClonePoints(Point3* target, Point3* source, uint16_t nPoints, float scale){
    for(int i = 0; i < nPoints; i++){
        target[i].x = (source[i].x * scale);
        target[i].y = (source[i].y * scale);
        target[i].z = (source[i].z * scale);
    }
}

void CloneToPosition(Point3* target, Point3* source, Point3 position, uint16_t nPoints){
    for(int i = 0; i < nPoints; i++){
        target[i].x = source[i].x + position.x;
        target[i].y = source[i].y + position.y;
        target[i].z = source[i].z + position.z;
    }
}

void DrawPrism(Point3 points[8], uint16_t thickness, uint16_t color){
    for(int i = 0; i < 4; i++){
        Draw3DLine(points[i], i == 3 ? points[0] : points[i + 1], thickness, color);
        Draw3DLine(points[i + 4], i == 3 ? points[4] : points[i + 5], thickness, color);
        Draw3DLine(points[i], points[i + 4], thickness, color);
    }
}

void DrawCube(Point3 center, uint16_t size, uint16_t thickness, uint16_t color){
    int length = size >> 1;
    Point3 cube[8] =
        {{center.x - length, center.y - length, center.z - length},
         {center.x + length, center.y - length, center.z - length},
         {center.x + length, center.y + length, center.z - length},
         {center.x - length, center.y + length, center.z - length},
         {center.x - length, center.y - length, center.z + length},
         {center.x + length, center.y - length, center.z + length},
         {center.x + length, center.y + length, center.z + length},
         {center.x - length, center.y + length, center.z + length}};
    DrawPrism(cube, thickness, color);
}

void Draw3DLine(Point3 p1, Point3 p2, uint16_t thickness, uint16_t color){

    // First point

    float perspective = FOCAL_LENGTH / (FOCAL_LENGTH + p1.z);

    float transX;
    float transY;

    float x1;
    float y1;
    float x2;
    float y2;

    if(!isometricView){
        transX = (p1.x * perspective) + GlobalCamera->position.x;
        transY = (p1.y * perspective) + GlobalCamera->position.y;

        x1  = (perspective * p1.x) + transX;
        y1  = (perspective * p1.y) + transY;

        // Second point;

        perspective = FOCAL_LENGTH / (FOCAL_LENGTH + p2.z);

        transX = (p2.x * perspective) + GlobalCamera->position.x;
        transY = (p2.y * perspective) + GlobalCamera->position.y;

        x2 = (perspective * p2.x) + transX;
        y2 = (perspective * p2.y) + transY;

    }else{
        transX = (p1.x * perspective);
        transY = (p1.y * perspective);

        x1  = (perspective * transX) + transX;
        y1  = (perspective * transY) + transY;

        // Second point;

        perspective = FOCAL_LENGTH / (FOCAL_LENGTH + p2.z);

        transX = (p2.x * perspective);
        transY = (p2.y * perspective);

        x2 = (perspective * transX) + transX;
        y2 = (perspective * transY) + transY;
    }

    DrawLine(x1, y1, x2, y2, thickness, color);
}

inline float Magnitude(Point3 vector){
    return sqrt((vector.x * vector.x) + (vector.y * vector.y) + (vector.z * vector.z));
}

inline float DistanceBetween(Point3 p1, Point3 p2){
    Point3 dif = VectorSub(p1, p2);
    return Magnitude(dif);
}

inline float DotProduct(Point3 v1, Point3 v2){
    return (v1.x * v2.x) + (v2.x * v2.y) + (v1.z * v2.z);
}

inline Point3 VectorAdd(Point3 v1, Point3 v2){
    return (Point3) {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

inline Point3 VectorSub(Point3 v1, Point3 v2){
    return (Point3) {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

inline Point3 VectorMul(Point3 v1, Point3 v2){
    return (Point3) {v1.x * v2.x, v1.y * v2.y, v1.z * v2.z};
}

inline Point3 VectorScale(Point3 vector, float scalar){
    return (Point3) {vector.x * scalar, vector.y * scalar, vector.z * scalar};
}

inline Point3 Normalized(Point3 vector){
    float mag = Magnitude(vector);
    return (Point3) {vector.x / mag, vector.y / mag, vector.z / mag};
}

// Credit to rosettacode.org!
inline Point3 Intersection(Point3 lineDir, Point3 linePoint, Point3 planeDir, Point3 planePoint){
    Point3 dif = VectorSub(linePoint, planePoint);
    return VectorAdd(VectorAdd(dif, planePoint), VectorScale(lineDir, -DotProduct(dif, planeDir) / DotProduct(lineDir, planeDir)));
}

inline void TranslatePoint(Point3* point, Point3 displacement){
    point->x += displacement.x;
    point->y += displacement.y;
    point->z += displacement.z;
}

void TranslatePoints(Point3* points, uint8_t nPoints, Point3 displacement){
    for(int i = 0; i < nPoints; i++){
        TranslatePoint(&points[i], displacement);
    }
}

//void RotatePoint(Point3* point, Point3 origin, Point3 rotation){
//
//    float cosTheta = cosf(rotation.x);
//    float sinTheta = sinf(rotation.x);
//    float cosPhi = cosf(rotation.y);
//    float sinPhi = sinf(rotation.y);
//    float cosPsi = cosf(rotation.z);
//    float sinPsi = sinf(rotation.z);
//
//    float sinThetaCosPsi = sinTheta * cosPsi;
//    float sinThetaSinPsi = sinTheta * sinPsi;
//
//    // x column
//    Point3 xCol = {cosTheta * cosPsi, -cosTheta * sinPsi, sinTheta};
//
//    // y column
//    Point3 yCol = {(cosPhi * sinPsi) + (sinPhi * sinThetaCosPsi), (cosPhi * cosPsi) - (sinPhi * sinThetaSinPsi), -(sinPhi * cosTheta)};
//
//    // z column
//    Point3 zCol = {(sinPhi * sinPsi) - (cosPhi * sinThetaCosPsi), (sinPhi * cosPsi) + (cosPhi * sinThetaSinPsi), (cosPhi * cosTheta)};
//
//    Point3 xyz = VectorSub(*point, origin);
//
//    point->x = DotProduct(xyz, xCol) + origin.x;
//    point->y = DotProduct(xyz, yCol) + origin.y;
//    point->z = DotProduct(xyz, zCol) + origin.z;
//}

//void RotatePoints(Point3* points, uint8_t nPoints, Point3 origin, Point3 rotation){
//    float cosTheta = cosf(rotation.x);
//    float sinTheta = sinf(rotation.x);
//    float cosPhi = cosf(rotation.y);
//    float sinPhi = sinf(rotation.y);
//    float cosPsi = cosf(rotation.z);
//    float sinPsi = sinf(rotation.z);
//
//    float sinThetaCosPsi = sinTheta * cosPsi;
//    float sinThetaSinPsi = sinTheta * sinPsi;
//
//    // x column
//    Point3 xCol = {cosTheta * cosPsi, -cosTheta * sinPsi, sinTheta};
//
//    // y column
//    Point3 yCol = {(cosPhi * sinPsi) + (sinPhi * sinThetaCosPsi), (cosPhi * cosPsi) - (sinPhi * sinThetaSinPsi), -(sinPhi * cosTheta)};
//
//    // z column
//    Point3 zCol = {(sinPhi * sinPsi) - (cosPhi * sinThetaCosPsi), (sinPhi * cosPsi) + (cosPhi * sinThetaSinPsi), (cosPhi * cosTheta)};
//
//    Point3 xyz;
//    for(int i = 0; i < nPoints; i++){
//        xyz = VectorSub(points[i], origin);
//
//        points[i].x = DotProduct(xyz, xCol) + origin.x;
//        points[i].y = DotProduct(xyz, yCol) + origin.y;
//        points[i].z = DotProduct(xyz, zCol) + origin.z;
//    }
//}

// This could work instead
Point3 SphereToCart(float distance, float theta, float phi){
    return (Point3) {distance * cosf(phi) * cosf(theta), distance * cosf(phi) * sinf(theta), distance * sinf(phi)};
}

void RotatePoint(Point3* point, Point3 origin, Point3 rotation){
    float cosine;
    float sine;


    if(rotation.y != 0){
        cosine = cosf(rotation.y);
        sine = sinf(rotation.y);

        float fz = (point->z - origin.z);
        float fx = (point->x - origin.x);

        point->z = origin.z + (fz * cosine + fx * sine);
        point->x = origin.x + (fx * cosine - fz * sine);
    }
    if(rotation.x != 0){
        cosine = cosf(rotation.x);
        sine = sinf(rotation.x);

        float fz = (point->z - origin.z);
        float fy = (point->y - origin.y);

        point->z = origin.z + (fz * cosine + fy * sine);
        point->y = origin.y + (fy * cosine - fz * sine);

    }
    if(rotation.z != 0){
        cosine = cosf(rotation.z);
        sine = sinf(rotation.z);

        float fx = (point->x - origin.x);
        float fy = (point->y - origin.y);

        point->x = origin.x + (fx * cosine - fy * sine);
        point->y = origin.y + (fx * sine + fy * cosine);
    }

}

void RotatePoints(Point3* points, uint8_t nPoints, Point3 origin, Point3 rotation){

    float cosine;
    float sine;

    if(rotation.x != 0){
        cosine = cosf(rotation.x);
        sine = sinf(rotation.x);
        for(int i = 0; i < nPoints; i++){

            float fz = (points[i].z - origin.z);
            float fy = (points[i].y - origin.y);

            points[i].z = origin.z + (fz * cosine + fy * sine);
            points[i].y = origin.y + (fy * cosine - fz * sine);
        }

    }
    if(rotation.y != 0){
        cosine = cosf(rotation.y);
        sine = sinf(rotation.y);
        for(int i = 0; i < nPoints; i++){

            float fz = (points[i].z - origin.z);
            float fx = (points[i].x - origin.x);

            points[i].z = origin.z + (fz * cosine + fx * sine);
            points[i].x = origin.x + (fx * cosine - fz * sine);
        }
    }
    if(rotation.z != 0){
        cosine = cosf(rotation.z);
        sine = sinf(rotation.z);
        for(int i = 0; i < nPoints; i++){

            float fx = (points[i].x - origin.x);
            float fy = (points[i].y - origin.y);

            points[i].x = origin.x + (fx * cosine - fy * sine);
            points[i].y = origin.y + (fx * sine + fy * cosine);
        }
    }

}


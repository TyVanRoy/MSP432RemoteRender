/*
 * RedGeometry.h
 *
 *  Created on: Mar 8, 2020
 *      Author: tyvanroy
 */

#ifndef REDLIB_INC_RED3D_H_
#define REDLIB_INC_RED3D_H_

#include <RedTypes.h>

/** Call before using this module **/
void InitRed3D(void);
void SetGlobalCam(Camera* cam, bool isometric);

inline float Magnitude(Point3 vector);
inline float DistanceBetween(Point3 p1, Point3 p2);
inline float DotProduct(Point3 v1, Point3 v2);
inline Point3 VectorAdd(Point3 v1, Point3 v2);
inline Point3 VectorSub(Point3 v1, Point3 v2);
inline Point3 VectorMul(Point3 v1, Point3 v2);
inline Point3 VectorScale(Point3 vector, float scalar);
inline Point3 Normalized(Point3 vector);
inline Point3 Intersection(Point3 lineDir, Point3 linePoint, Point3 planeDir, Point3 planePoint);

inline void TranslatePoint(Point3* point, Point3 displacement);
void TranslatePoints(Point3* points, uint8_t nPoints, Point3 displacement);
void RotatePoint(Point3* point, Point3 origin, Point3 rotation);
void RotatePoints(Point3* points, uint8_t nPoints, Point3 origin, Point3 rotation);

/* Print 3D -> 2D */
inline Point WorldToScreenPoint(Point3 cameraPosition, Point3 point);
inline void Print3DLine(Point3 cameraPosition, Point3 p1, Point3 p2, Line* line);
void PrintPoly(Point3 cameraPosition, Point3* points, Point3 origin, Pair* lines, uint8_t lineCount, uint16_t color, Frame_t* frame, uint8_t frameOffset, bool renderMode);

/** Clone Point3 array **/
void ClonePoly(Poly* target, Poly* source, float scale);
void ClonePoints(Point3* target, Point3* source, uint16_t nPoints, float scale);
void CloneToPosition(Point3* target, Point3* source, Point3 position, uint16_t nPoints);

/** These use camera passed with InitRed3D **/
void DrawPrism(Point3 points[8], uint16_t thickness, uint16_t color);
void DrawCube(Point3 center, uint16_t size, uint16_t thickness, uint16_t color);
void Draw3DLine(Point3 p1, Point3 p2, uint16_t thickness, uint16_t color);

#endif /* REDLIB_INC_RED3D_H_ */

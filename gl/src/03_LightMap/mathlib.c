/*  light mapping demo
 *
 *      written by Alexander Zaprjagaev
 *      frustum@public.tsu.ru
 *      2:5005/93.15@FidoNet
 */

#include <math.h>
#include "mathlib.h"

float   VectorLength(const float *v) {
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float   VectorNormalize(const float *v,float *out) {
    float length,ilength;
    length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if(length == 0) {
        VectorClear(out);
        return 0;
    }
    ilength = 1.0 / length;
    VectorScale(v,ilength,out);
    return length;
}

void    VectorCrossProduct(const float *v1,const float *v2,float *out) {
    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void    VectorTransform(const float *v,const float *m,float *out) {
    float v1[3];
    VectorCopy(v,v1);
    out[0] = v1[0] * m[0] + v1[1] * m[4] + v1[2] * m[8] + m[12];
    out[1] = v1[0] * m[1] + v1[1] * m[5] + v1[2] * m[9] + m[13];
    out[2] = v1[0] * m[2] + v1[1] * m[6] + v1[2] * m[10] + m[14];
}

void    VectorTransformNormal(const float *v,const float *m,float *out) {
    float v1[3];
    VectorCopy(v,v1);
    out[0] = v1[0] * m[0] + v1[1] * m[4] + v1[2] * m[8];
    out[1] = v1[0] * m[1] + v1[1] * m[5] + v1[2] * m[9];
    out[2] = v1[0] * m[2] + v1[1] * m[6] + v1[2] * m[10];
}

void    MatrixMultiply(const float *m1,const float *m2,float *out) {
    out[0] = m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3];
    out[1] = m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3];
    out[2] = m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3];
    out[3] = m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3];
    out[4] = m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7];
    out[5] = m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7];
    out[6] = m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7];
    out[7] = m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7];
    out[8] = m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11];
    out[9] = m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11];
    out[10] = m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11];
    out[11] = m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11];
    out[12] = m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15];
    out[13] = m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15];
    out[14] = m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15];
    out[15] = m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15];    
}

int MatrixInverse(const float *m,float *out) {
    float   det;
    det = m[0] * m[5] * m[10];
    det += m[4] * m[9] * m[2];
    det += m[8] * m[1] * m[6];
    det -= m[8] * m[5] * m[2];
    det -= m[4] * m[1] * m[10];
    det -= m[0] * m[9] * m[6];
    if(det * det < 1e-25) return 0;
    det = 1.0 / det;    
    out[0] =    (m[5] * m[10] - m[9] * m[6]) * det;
    out[1] =  - (m[1] * m[10] - m[9] * m[2]) * det;
    out[2] =    (m[1] * m[6] -  m[5] * m[2]) * det;
    out[3] = 0.0;
    out[4] =  - (m[4] * m[10] - m[8] * m[6]) * det;
    out[5] =    (m[0] * m[10] - m[8] * m[2]) * det;
    out[6] =  - (m[0] * m[6] -  m[4] * m[2]) * det;
    out[7] = 0.0;
    out[8] =    (m[4] * m[9] -  m[8] * m[5]) * det;
    out[9] =  - (m[0] * m[9] -  m[8] * m[1]) * det;
    out[10] =   (m[0] * m[5] -  m[4] * m[1]) * det;
    out[11] = 0.0;
    out[12] = - (m[12] * out[0] + m[13] * out[4] + m[14] * out[8]);
    out[13] = - (m[12] * out[1] + m[13] * out[5] + m[14] * out[9]);
    out[14] = - (m[12] * out[2] + m[13] * out[6] + m[14] * out[10]);
    out[15] = 1.0;
    return 1;
}

void    MatrixInverseTranspose(const float *m,float *out) {
   out[0] = m[0]; out[4] = m[1]; out[8] = m[2]; out[12] = m[3];
   out[1] = m[4]; out[5] = m[5]; out[9] = m[6]; out[13] = m[7];
   out[2] = m[8]; out[6] = m[9]; out[10] = m[10]; out[14] = m[11];
   out[3] = m[12]; out[7] = m[13]; out[11] = m[14]; out[15] = m[15];
}

void    MatrixRotationX(float angle,float *out) {
    float rad = angle * DEG2RAD;
    float Cos = cos(rad);
    float Sin = sin(rad);
    out[0] = 1.0; out[4] = 0.0; out[8] = 0.0; out[12] = 0.0;
    out[1] = 0.0; out[5] = Cos; out[9] = -Sin; out[13] = 0.0;
    out[2] = 0.0; out[6] = Sin; out[10] = Cos; out[14] = 0.0;
    out[3] = 0.0; out[7] = 0.0; out[11] = 0.0; out[15] = 1.0;
}

void    MatrixRotationY(float angle,float *out) {
    float rad = angle * DEG2RAD;
    float Cos = cos(rad);
    float Sin = sin(rad);
    out[0] = Cos; out[4] = 0.0; out[8] = Sin; out[12] = 0.0;
    out[1] = 0.0; out[5] = 1.0; out[9] = 0.0; out[13] = 0.0;
    out[2] = -Sin; out[6] = 0.0; out[10] = Cos; out[14] = 0.0;
    out[3] = 0.0; out[7] = 0.0; out[11] = 0.0; out[15] = 1.0;
}

void    MatrixRotationZ(float angle,float *out) {
    float rad = angle * DEG2RAD;
    float Cos = cos(rad);
    float Sin = sin(rad);
    out[0] = Cos; out[4] = -Sin; out[8] = 0.0; out[12] = 0.0;
    out[1] = Sin; out[5] = Cos; out[9] = 0.0; out[13] = 0.0;
    out[2] = 0.0; out[6] = 0.0; out[10] = 1.0; out[14] = 0.0;
    out[3] = 0.0; out[7] = 0.0; out[11] = 0.0; out[15] = 1.0;
}

void    MatrixTranslate(float x,float y,float z,float *out) {
    out[0] = 1.0; out[4] = 0.0; out[8] = 0.0; out[12] = x;
    out[1] = 0.0; out[5] = 1.0; out[9] = 0.0; out[13] = y;
    out[2] = 0.0; out[6] = 0.0; out[10] = 1.0; out[14] = z;
    out[3] = 0.0; out[7] = 0.0; out[11] = 0.0; out[15] = 1.0;
}

void    MatrixLookAt(const float *eye,const float *dir,const float *up,float *out) {
    float x[3],y[3],z[3],m1[16],m2[16];
    VectorSub(eye,dir,z);
    VectorNormalize(z,z);
    VectorCrossProduct(up,z,x);
    VectorCrossProduct(z,x,y);
    VectorNormalize(x,x);
    VectorNormalize(y,y);
    m1[0] = x[0]; m1[4] = x[1]; m1[8] = x[2]; m1[12] = 0.0;
    m1[1] = y[0]; m1[5] = y[1]; m1[9] = y[2]; m1[13] = 0.0;
    m1[2] = z[0]; m1[6] = z[1]; m1[10] = z[2]; m1[14] = 0.0;
    m1[3] = 0.0; m1[7] = 0.0; m1[11] = 0.0; m1[15] = 1.0;
    MatrixTranslate(-eye[0],-eye[1],-eye[2],m2);
    MatrixMultiply(m1,m2,out);
}

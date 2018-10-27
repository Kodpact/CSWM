#ifndef INCLUDE_MINIMATH_H
#define INCLUDE_MINIMATH_H

#define M_PI			3.14159265358979323846

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef clamp
#define clamp(a,b,c) (((a) > (c)) ? (c) : (((a) < (b)) ? (b) : (a)))
#endif

#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define VectorClear(a) {(a)[0]=0.0;(a)[1]=0.0;(a)[2]=0.0;}
#define VectorScale(a,b,c){c[0]=a[0]*b;c[1]=a[1]*b;c[2]=a[2]*b;}

typedef float vec_t;
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef int fixed16_t;

typedef union DLONG_u
{
	int i[2];
	double d;
	float f;
} DLONG;

#endif
#ifndef APP_MATH_QUATERNION_H
#define APP_MATH_QUATERNION_H
/* Hamilton convention (BNO055), NOT JPL. q = (w, x, y, z). */
typedef struct { float w, x, y, z; } quat_t;

quat_t quat_conj(quat_t q);          /* unit-quat inverse */
quat_t quat_mul (quat_t a, quat_t b);/* Hamilton product a*b */
float  quat_norm(quat_t q);
#endif
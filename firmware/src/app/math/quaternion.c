#include "app/math/quaternion.h"
#include <math.h>

quat_t quat_conj(quat_t q) { return (quat_t){ q.w, -q.x, -q.y, -q.z }; }

quat_t quat_mul(quat_t a, quat_t b) {
    return (quat_t){
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w
    };
}

float quat_norm(quat_t q) { return sqrtf(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z); }
/*
 * This file is part of the MOD project.
 *
 * Copyright (C) 2025 Zoltán Mánya <zltnmanya@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef UTIL_MATH_HH
#define UTIL_MATH_HH

#include <math.h>
#include <stdio.h>
#include <quaternion.h>

#define PI      (3.14159265358979323)
#define NATURAL (2.71828182845904523)
#define G       (9.8067)

static inline double sqrt_fp(double v) { return sqrt(v); }
static inline float sqrt_fp(float v) { return sqrtf(v); }

static inline double logistic(double x) {
  return 1.0 / (1.0 + pow(NATURAL, -x));
}

static inline double clamp_perc(double x) {
  if (x <= 0.0) return 0.0;
  else if (x >= 1.0) return 1.0;
  else return x;
}

template <typename T>
class RangeCheck {
	T lower, upper;
public:
	RangeCheck(T mid, T diff):lower(mid - diff), upper(mid+diff) {}
	int inside(T value) const {
		return (value >= lower) && (value <= upper);
	}
};

template <typename T>
class RangeSqareCheck {
	T lower, upper;
public:
	RangeSqareCheck(T center, T dev):lower((center - dev) * (center - dev)), upper((center + dev) * (center + dev)) { }
	int inside(T value) const {
		return (value >= lower) && (value <= upper);
	}
};

template <typename T>
class Vec3 {
  public:
  T x,y,z;
  Vec3():x(0),y(0),z(0) {}
  Vec3(T x,T y,T z):x(x),y(y),z(z) {}
  template<typename S>
  Vec3(const Vec3<S>& s): x((T)s.x), y((T)s.y), z((T)s.z) { }

  void setZero() {
    x=y=z=0;
  }

  T square() const {
	  return x*x + y*y + z*z;
  }
  T abs() const {
    return sqrt_fp(x*x + y*y + z*z);
  }

  void normalize() {
    T len = abs();
    if (len > 0.001) {
      x /= len;
      y /= len;
      z /= len;
    }
  }

  Vec3<T>& operator /=(T b) {
    this->x /= b;
    this->y /= b;
    this->z /= b;
    return *this;
  }
  Vec3<T>& operator +=(const Vec3<T> &b) {
    this->x += b.x;
    this->y += b.y;
    this->z += b.z;
    return *this;
  }
  Vec3<T>& operator -=(const Vec3<T> &b) {
    this->x -= b.x;
    this->y -= b.y;
    this->z -= b.z;
    return *this;
  }
  Vec3<T> operator -() {
    return Vec3<T>(-x, -y, -z);
  }
};

template<typename T>
T abs(const Vec3<T>& v) {
  return sqrt_fp(v * v);
}

template <typename T, typename S>
Vec3<T> operator *(const Vec3<T> &a, S b) {
  return Vec3<T>(a.x * b,a.y * b,a.z * b);
}

template <typename T, typename S>
Vec3<T> operator /(const Vec3<T> &a, S b) {
  return Vec3<T>(a.x / b,a.y / b,a.z / b);
}

template <typename T>
Vec3<T> operator +(const Vec3<T> &a, const Vec3<T> &b) {
  return Vec3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}
template <typename T>
Vec3<T> operator -(const Vec3<T> &a, const Vec3<T> &b) {
  return Vec3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}
template <typename T>
T operator *(const Vec3<T> &a, const Vec3<T> &b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
Vec3<T> operator ^(const Vec3<T> &a, const Vec3<T> &b) {
  //  i  j  k
  // ax ay az
  // bx by bz
  //
  //  i * (ay * bz - az * by)
  // -j * (ax * bz - az * bx)
  //  k * (ax * by - ay * bx)
  //
  //  i * (ay * bz - az * by)
  //  j * (az * bx - ax * bz)
  //  k * (ax * by - ay * bx)
  return Vec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
template <typename T, typename S>
Vec3<T>& operator *= (Vec3<T>& a, const S b) {
  a.x = (T)(a.x * b);
  a.y = (T)(a.y * b);
  a.z = (T)(a.z * b);
  return a;
}

template <typename T>
T angle_dot(const Vec3<T> &a, const Vec3<T> &b) {
  return acos_fp(a*b / (a.abs() * b.abs()));
}

typedef Vec3<double> Vec3d;
typedef Vec3<float> Vec3f;

static const double sqrt05 = sqrt(0.5);

void quat2ypr(float *yaw, float *pitch, float *roll, float x, float y, float z, float w);

template<typename T>
Vec3<T> scale(const Vec3<T>&a,const Vec3<T>&b) {
  return Vec3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}


template <typename T>
class Matrix3x3 {
	int minor_index(int i, int skipped) {
  	if (i < skipped) return i;
  	else return i+1;
	}
    
  public:
	T values[3][3]; /* [row][column] */

	Matrix3x3() {
  }
	Matrix3x3(const Vec3d& a, const Vec3d& b, const Vec3d& c) {
  	values[0][0] = a.x;
  	values[0][1] = a.y;
  	values[0][2] = a.z;
 	 
  	values[1][0] = b.x;
  	values[1][1] = b.y;
  	values[1][2] = b.z;
 	 
  	values[2][0] = c.x;
  	values[2][1] = c.y;
  	values[2][2] = c.z;
	}
    
	T minor_matrix(int i, int j) {
  	int i1 = minor_index(0, i);
  	int i2 = minor_index(1, i);
  	int j1 = minor_index(0, j);
  	int j2 = minor_index(1, j);
 	 
  	return values[i1][j1] * values[i2][j2] - values[i1][j2] * values[i2][j1];
	}
    
	void invert() {
  	T result[3][3];
 	 
  	T determinant = 0;
 	 
  	for (int i=0;i<3;i++) for (int j=0;j<3;j++) {
    	T val = minor_matrix(i,j);
   	 
    	// check table rule
    	if (((i + j) & 1) != 0) val = -val;
 	 
    	result[i][j] = val;
   	 
    	if (i == 0) determinant += values[0][j] * val;
  	}
 	 
  	for (int i=0;i<3;i++) for (int j=0;j<3;j++) {
    	// transpose and divide
    	values[i][j] = result[j][i] / determinant;
  	}
	}
    
  void set_union() {
  	for (int i=0;i<3;i++)
    	for (int j=0;j<3;j++)
        values[i][j] = (i==j) ? 1 : 0;
  }
};

template<typename T>
Matrix3x3<T> operator*(const Matrix3x3<T>& a,const Matrix3x3<T>& b) {
  Matrix3x3<T> result;
  for (int i=0;i<3;i++)
    for (int j=0;j<3;j++) {
        result.values[i][j] = 0;
        for (int k=0;k<3;k++)
          result.values[i][j] += a.values[i][k] * b.values[k][j];
      }
  return result;
}

template<typename T>
Vec3<T> rotate(const Vec3<T>& v, const quaternion::Quaternion<T>& q) {
  quaternion::Quaternion<T> qv(0, v.x, v.y, v.z);
  quaternion::Quaternion<T> q3 = q * qv * conj(q);
  return Vec3<T>(q3.b(), q3.c(), q3.d());
}

#endif


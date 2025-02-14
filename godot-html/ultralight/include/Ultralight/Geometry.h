/******************************************************************************
 *  This file is a part of Ultralight, an ultra-portable web-browser engine.  *
 *                                                                            *
 *  See <https://ultralig.ht> for licensing and more.                         *
 *                                                                            *
 *  (C) 2023 Ultralight, Inc.                                                 *
 *****************************************************************************/
#pragma once
#include <Ultralight/Defines.h>
#include <memory.h>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace ultralight {

///
/// 2D Vector Helper
///
struct UExport vec2 {
  float x, y;

  inline vec2() : x(0.0f), y(0.0f) { }

  inline vec2(float x, float y) : x(x), y(y) {}

  inline vec2(float x) : x(x), y(x) {}

  inline vec2 yx() const { return { y, x }; }

  inline vec2 xx() const { return { x, x }; }

  inline vec2 yy() const { return { y, y }; }

  inline friend vec2 operator+(vec2 lhs, const vec2& rhs) { lhs += rhs; return lhs; }

  inline friend vec2 operator-(vec2 lhs, const vec2& rhs) { lhs -= rhs; return lhs; }

  inline friend vec2 operator*(vec2 lhs, const vec2& rhs) { lhs *= rhs; return lhs; }

  inline friend vec2 operator/(vec2 lhs, const vec2& rhs) { lhs /= rhs; return lhs; }

  inline friend vec2 operator+(vec2 lhs, float rhs) { lhs += rhs; return lhs; }

  inline friend vec2 operator-(vec2 lhs, float rhs) { lhs -= rhs; return lhs; }

  inline friend vec2 operator*(vec2 lhs, float rhs) { lhs *= rhs; return lhs; }

  inline friend vec2 operator/(vec2 lhs, float rhs) { lhs /= rhs; return lhs; }

  inline vec2& operator+=(const vec2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
  }

  inline vec2& operator-=(const vec2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
  }

  inline vec2& operator*=(const vec2& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    return *this;
  }

  inline vec2& operator/=(const vec2& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    return *this;
  }

  inline vec2& operator+=(float rhs) {
    x += rhs;
    y += rhs;
    return *this;
  }

  inline vec2& operator-=(float rhs) {
    x -= rhs;
    y -= rhs;
    return *this;
  }

  inline vec2& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    return *this;
  }

  inline vec2& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    return *this;
  }

  inline friend bool operator==(const vec2& a, const vec2& b) {
    return !memcmp(&a, &b, sizeof(a));
  }

  inline friend bool operator!=(const vec2& a, const vec2& b) {
    return !(a == b);
  }

  inline friend vec2 min_(const vec2& a, const vec2& b) {
    return{ (b.x < a.x) ? b.x : a.x, 
            (b.y < a.y) ? b.y : a.y };
  }

  inline friend vec2 max_(const vec2& a, const vec2& b) {
    return{ (a.x < b.x) ? b.x : a.x, 
            (a.y < b.y) ? b.y : a.y };
  }

  inline friend vec2 clamp(const vec2& x, const vec2& minVal, const vec2& maxVal) {
    return min_(max_(x, minVal), maxVal);
  }

  inline friend vec2 mix(const vec2& a, const vec2& b, float t) {
    return a * (1.0f - t) + b * t;
  }

  inline friend float length(const vec2& a) {
    return sqrtf(a.x * a.x + a.y * a.y);
  }

  // squared length
  inline friend float length2(const vec2& a) {
    return dot(a, a);
  }

  inline friend float distance(const vec2& a, const vec2& b) {
    return length(a - b);
  }

  // squared distance
  inline friend float distance2(const vec2& a, const vec2& b) {
    return length2(a - b);
  }

  inline friend vec2 normalize(const vec2& a) {
    return a / length(a);
  }

  inline friend float dot(const vec2& a, const vec2& b) {
    return a.x * b.x + a.y * b.y;
  }
};

///
/// 3D Vector Helper
///
struct UExport vec3 {
  float x, y, z;

  inline vec3() : x(0.0f), y(0.0f), z(0.0f) { }

  inline vec3(float x, float y, float z) : x(x), y(y), z(z) {}

  inline vec3(float x) : x(x), y(x), z(x) {}

  inline friend vec3 operator+(vec3 lhs, const vec3& rhs) { lhs += rhs; return lhs; }

  inline friend vec3 operator-(vec3 lhs, const vec3& rhs) { lhs -= rhs; return lhs; }

  inline friend vec3 operator*(vec3 lhs, const vec3& rhs) { lhs *= rhs; return lhs; }

  inline friend vec3 operator/(vec3 lhs, const vec3& rhs) { lhs /= rhs; return lhs; }

  inline friend vec3 operator+(vec3 lhs, float rhs) { lhs += rhs; return lhs; }

  inline friend vec3 operator-(vec3 lhs, float rhs) { lhs -= rhs; return lhs; }

  inline friend vec3 operator*(vec3 lhs, float rhs) { lhs *= rhs; return lhs; }

  inline friend vec3 operator/(vec3 lhs, float rhs) { lhs /= rhs; return lhs; }

  inline vec3& operator+=(const vec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  inline vec3& operator-=(const vec3& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  inline vec3& operator*=(const vec3& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
  }

  inline vec3& operator/=(const vec3& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
  }

  inline vec3& operator+=(float rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
    return *this;
  }

  inline vec3& operator-=(float rhs) {
    x -= rhs;
    y -= rhs;
    z -= rhs;
    return *this;
  }

  inline vec3& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  inline vec3& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  inline friend bool operator==(const vec3& a, const vec3& b) {
    return !memcmp(&a, &b, sizeof(a));
  }

  inline friend bool operator!=(const vec3& a, const vec3& b) {
    return !(a == b);
  }

  inline friend vec3 min_(const vec3& a, const vec3& b) {
    return{ (b.x < a.x) ? b.x : a.x,
            (b.y < a.y) ? b.y : a.y,
            (b.z < a.z) ? b.z : a.z };
  }

  inline friend vec3 max_(const vec3& a, const vec3& b) {
    return{ (a.x < b.x) ? b.x : a.x,
            (a.y < b.y) ? b.y : a.y,
            (a.z < b.z) ? b.z : a.z };
  }
  inline friend vec3 clamp(const vec3& x, const vec3& minVal, const vec3& maxVal) {
    return min_(max_(x, minVal), maxVal);
  }

  inline friend vec3 mix(const vec3& a, const vec3& b, float t) {
    return a * (1.0f - t) + b * t;
  }

  inline friend float length(const vec3& a) {
    return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
  }

  inline friend float distance(const vec3& a, const vec3& b) {
    return length(a - b);
  }

  inline friend vec3 normalize(const vec3& a) {
    return a / length(a);
  }

  inline friend float dot(const vec3& a, const vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }
};

///
/// 4D Vector Helper
///
struct UExport vec4 {
  float x, y, z, w;

  inline vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }

  inline vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

  inline vec4(float x) : x(x), y(x), z(x), w(x) {}

  inline vec4(const float val[4]) : x(val[0]), y(val[1]), z(val[2]), w(val[3]) { }

  inline friend bool operator==(const vec4& a, const vec4& b) {
    return !memcmp(&a, &b, sizeof(a));
  }

  inline friend bool operator!=(const vec4& a, const vec4& b) {
    return !(a == b);
  }

  inline friend vec4 operator+(vec4 lhs, const vec4& rhs) { lhs += rhs; return lhs; }

  inline friend vec4 operator-(vec4 lhs, const vec4& rhs) { lhs -= rhs; return lhs; }

  inline friend vec4 operator*(vec4 lhs, const vec4& rhs) { lhs *= rhs; return lhs; }

  inline friend vec4 operator/(vec4 lhs, const vec4& rhs) { lhs /= rhs; return lhs; }

  inline friend vec4 operator+(vec4 lhs, float rhs) { lhs += rhs; return lhs; }

  inline friend vec4 operator-(vec4 lhs, float rhs) { lhs -= rhs; return lhs; }

  inline friend vec4 operator*(vec4 lhs, float rhs) { lhs *= rhs; return lhs; }

  inline friend vec4 operator/(vec4 lhs, float rhs) { lhs /= rhs; return lhs; }

  inline vec4& operator+=(const vec4& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
  }

  inline vec4& operator-=(const vec4& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
  }

  inline vec4& operator*=(const vec4& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    w *= rhs.w;
    return *this;
  }

  inline vec4& operator/=(const vec4& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    w /= rhs.w;
    return *this;
  }

  inline vec4& operator+=(float rhs) {
    x += rhs;
    y += rhs;
    z += rhs;
    w += rhs;
    return *this;
  }

  inline vec4& operator-=(float rhs) {
    x -= rhs;
    y -= rhs;
    z -= rhs;
    w -= rhs;
    return *this;
  }

  inline vec4& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    w *= rhs;
    return *this;
  }

  inline vec4& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    w /= rhs;
    return *this;
  }

  inline friend vec4 min_(const vec4& a, const vec4& b) {
    return{ (b.x < a.x) ? b.x : a.x,
            (b.y < a.y) ? b.y : a.y,
            (b.z < a.z) ? b.z : a.z,
            (b.w < a.w) ? b.w : a.w };
  }

  inline friend vec4 max_(const vec4& a, const vec4& b) {
    return{ (a.x < b.x) ? b.x : a.x,
            (a.y < b.y) ? b.y : a.y,
            (a.z < b.z) ? b.z : a.z,
            (a.w < b.w) ? b.w : a.w };
  }

  inline void load(const float* val) {
    x = val[0];
    y = val[1];
    z = val[2];
    w = val[3];
  }

  inline void store(float* val) const {
    val[0] = x;
    val[1] = y;
    val[2] = z;
    val[3] = w;
  }
};

///
/// Point is typedef'd to a 2D vector
///
typedef vec2 Point;

///
/// Float Rectangle Helper
///
struct UExport Rect {
  float left, top, right, bottom;

  static constexpr Rect MakeEmpty() { return Rect{ 0.0f, 0.0f, 0.0f, 0.0f }; }

  inline float width() const { return right - left; }
  inline float height() const { return bottom - top; }
  inline float x() const { return left; }
  inline float y() const { return top; }
  inline float center_x() const { return (left + right) * 0.5f; }
  inline float center_y() const { return (top + bottom) * 0.5f; }

  inline Point origin() const { return { left, top }; }

  inline void SetEmpty() { *this = MakeEmpty(); }

  inline bool IsEmpty() const {
    return *this == MakeEmpty();
  }

  inline bool IsValid() const {
    return width() > 0 && height() > 0;
  }

  inline void Inset(float dx, float dy) {
    left += dx;
    top += dy;
    right -= dx;
    bottom -= dy;
  }

  inline void Outset(float dx, float dy) {
    Inset(-dx, -dy);
  }

  inline void Move(float dx, float dy) {
    left += dx;
    top += dy;
    right += dx;
    bottom += dy;
  }

  inline float area() const {
    return width() * height();
  }

  inline void Join(const Rect& rhs) {
    // if we are empty, just assign
    if (IsEmpty()) {
      *this = rhs;
    }
    else {
      if (rhs.left < left) left = rhs.left;
      if (rhs.top < top) top = rhs.top;
      if (rhs.right > right) right = rhs.right;
      if (rhs.bottom > bottom) bottom = rhs.bottom;
    }
  }

  inline void Join(const Point& p) {
    // if we are empty, just assign
    if (IsEmpty()) {
      *this = { p.x, p.y, p.x, p.y };
    }
    else {
      if (p.x < left) left = p.x;
      if (p.y < top) top = p.y;
      if (p.x > right) right = p.x;
      if (p.y > bottom) bottom = p.y;
    }
  }

  inline bool Contains(const Point& p) const {
    return p.x >= left && p.x <= right &&
           p.y >= top && p.y <= bottom;
  }

  inline bool Contains(const Rect& r) const {
    return left <= r.left && top <= r.top &&
           right >= r.right && bottom >= r.bottom;
  }

  inline bool Intersects(const Rect& rhs) const {
    return !(rhs.left > right ||
             rhs.right < left ||
             rhs.top > bottom ||
             rhs.bottom < top);
  }

  inline Rect Intersect(const Rect& other) const {
    return{ (left         < other.left) ? other.left   : left,
            (top          < other.top)  ? other.top    : top,
            (other.right  < right)      ? other.right  : right,
            (other.bottom < bottom)     ? other.bottom : bottom };
  }

  friend inline bool operator==(const Rect& a, const Rect& b) {
    return !memcmp(&a, &b, sizeof(a));
  }

  friend inline bool operator!=(const Rect& a, const Rect& b) {
    return !(a == b);
  }

  inline vec4 ToVec4() { return vec4 { left, top, right, bottom }; }
};

///
/// Integer Rectangle Helper
///
struct UExport IntRect {
  int left, top, right, bottom;

  static constexpr IntRect MakeEmpty() { return IntRect{ 0, 0, 0, 0 }; }

  inline int width() const { return right - left; }
  inline int height() const { return bottom - top; }
  inline int x() const { return left; }
  inline int y() const { return top; }
  inline int center_x() const { return (int)std::round((left + right) * 0.5f); }
  inline int center_y() const { return (int)std::round((top + bottom) * 0.5f); }

  inline Point origin() const { return Point{ (float)left, (float)top }; }

  inline void SetEmpty() { *this = MakeEmpty(); }

  inline bool IsEmpty() const {
    return *this == MakeEmpty();
  }

  inline bool IsValid() const {
    return width() > 0 && height() > 0;
  }

  inline void Inset(int dx, int dy) {
    left += dx;
    top += dy;
    right -= dx;
    bottom -= dy;
  }

  inline void Outset(int dx, int dy) {
    Inset(-dx, -dy);
  }

  inline void Move(int dx, int dy) {
    left += dx;
    top += dy;
    right += dx;
    bottom += dy;
  }

  inline int area() const {
    return width() * height();
  }

  inline void Join(const IntRect& rhs) {
    // if we are empty, just assign
    if (IsEmpty()) {
      *this = rhs;
    }
    else {
      if (rhs.left < left) left = rhs.left;
      if (rhs.top < top) top = rhs.top;
      if (rhs.right > right) right = rhs.right;
      if (rhs.bottom > bottom) bottom = rhs.bottom;
    }
  }

  inline void Join(const Point& p) {
    // if we are empty, just assign
    if (IsEmpty()) {
      *this = { (int)std::floor(p.x), (int)std::floor(p.y), (int)std::ceil(p.x), (int)std::ceil(p.y) };
    }
    else {
      if ((int)std::floor(p.x) < left) left = (int)std::floor(p.x);
      if ((int)std::floor(p.y) < top) top = (int)std::floor(p.y);
      if ((int)std::ceil(p.x) > right) right = (int)std::ceil(p.x);
      if ((int)std::ceil(p.y) > bottom) bottom = (int)std::ceil(p.y);
    }
  }

  inline bool Contains(const Point& p) const {
    return p.x >= left && p.x <= right &&
      p.y >= top && p.y <= bottom;
  }

  inline bool Contains(const IntRect& r) const {
    return left <= r.left && top <= r.top &&
      right >= r.right && bottom >= r.bottom;
  }

  inline bool Intersects(const IntRect& rhs) const {
    // Since this is mostly used for pixel operations, we only count
    // intersections that have width and height >= 1.
    return !(rhs.left > right - 1 ||
             rhs.right < left ||
             rhs.top > bottom - 1 ||
             rhs.bottom < top);
  }

  inline IntRect Intersect(const IntRect& other) const {
    return{ (left         < other.left) ? other.left   : left,
            (top          < other.top)  ? other.top    : top,
            (other.right  < right)      ? other.right  : right,
            (other.bottom < bottom)     ? other.bottom : bottom };
  }

  friend inline bool operator==(const IntRect& a, const IntRect& b) {
    return !memcmp(&a, &b, sizeof(a));
  }

  friend inline bool operator!=(const IntRect& a, const IntRect& b) {
    return !(a == b);
  }
};

///
/// Rounded Rectangle Helper
///
struct UExport RoundedRect {
  Rect rect;
  float radii_x[4];
  float radii_y[4];

  void SetEmpty();

  bool IsRounded() const;

  // Negative is inside, positive is outside.
  float GetSignedDistance(const Point& p) const;

  // Returns whether or not intersection is found. Can fail if the resulting
  // geometry is not a rounded rectangle.
  bool Intersect(const RoundedRect& other, RoundedRect& result) const;

  void SnapToPixels();

  Rect CalculateInterior() const;
};

}  // namespace ultralight

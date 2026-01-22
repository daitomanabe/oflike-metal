#pragma once

#include <cmath>
#include "../math/ofMath.h"
#include "../../core/Context.h"
#include "ofGraphics.h"

// 3x3 rotation matrix for 3D
struct ofMatrix3 {
  float m[9] = {1,0,0, 0,1,0, 0,0,1};

  static ofMatrix3 identity() {
    return ofMatrix3{1,0,0, 0,1,0, 0,0,1};
  }

  static ofMatrix3 rotationX(float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return ofMatrix3{1,0,0, 0,c,-s, 0,s,c};
  }

  static ofMatrix3 rotationY(float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return ofMatrix3{c,0,s, 0,1,0, -s,0,c};
  }

  static ofMatrix3 rotationZ(float radians) {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return ofMatrix3{c,-s,0, s,c,0, 0,0,1};
  }

  ofVec3f transform(const ofVec3f& v) const {
    return ofVec3f(
      m[0]*v.x + m[1]*v.y + m[2]*v.z,
      m[3]*v.x + m[4]*v.y + m[5]*v.z,
      m[6]*v.x + m[7]*v.y + m[8]*v.z
    );
  }

  ofMatrix3 operator*(const ofMatrix3& other) const {
    ofMatrix3 r;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        r.m[i*3+j] = 0;
        for (int k = 0; k < 3; k++) {
          r.m[i*3+j] += m[i*3+k] * other.m[k*3+j];
        }
      }
    }
    return r;
  }
};

// Simple perspective projection for 3D->2D
inline ofVec2f project3D(const ofVec3f& p, float fov, float centerX, float centerY) {
  float z = p.z + 500;
  if (z < 1) z = 1;
  float scale = fov / z;
  return ofVec2f(centerX + p.x * scale, centerY + p.y * scale);
}

// Draw 3D line with perspective
inline void ofDrawLine3D(const ofVec3f& p1, const ofVec3f& p2, float fov = 500) {
  float cx = oflike::engine().getWindowWidth() / 2.0f;
  float cy = oflike::engine().getWindowHeight() / 2.0f;
  ofVec2f a = project3D(p1, fov, cx, cy);
  ofVec2f b = project3D(p2, fov, cx, cy);
  oflike::engine().drawList().addLine(a.x, a.y, b.x, b.y);
}

// Draw 3D box wireframe
inline void ofDrawBox3D(float x, float y, float z, float w, float h, float d,
                        float rotX = 0, float rotY = 0, float rotZ = 0, float fov = 500) {
  ofVec3f verts[8] = {
    {-w/2, -h/2, -d/2}, {w/2, -h/2, -d/2}, {w/2, h/2, -d/2}, {-w/2, h/2, -d/2},
    {-w/2, -h/2, d/2},  {w/2, -h/2, d/2},  {w/2, h/2, d/2},  {-w/2, h/2, d/2}
  };

  ofMatrix3 rot = ofMatrix3::rotationX(rotX) * ofMatrix3::rotationY(rotY) * ofMatrix3::rotationZ(rotZ);

  for (int i = 0; i < 8; i++) {
    verts[i] = rot.transform(verts[i]);
    verts[i].x += x;
    verts[i].y += y;
    verts[i].z += z;
  }

  ofDrawLine3D(verts[0], verts[1], fov);
  ofDrawLine3D(verts[1], verts[2], fov);
  ofDrawLine3D(verts[2], verts[3], fov);
  ofDrawLine3D(verts[3], verts[0], fov);
  ofDrawLine3D(verts[4], verts[5], fov);
  ofDrawLine3D(verts[5], verts[6], fov);
  ofDrawLine3D(verts[6], verts[7], fov);
  ofDrawLine3D(verts[7], verts[4], fov);
  ofDrawLine3D(verts[0], verts[4], fov);
  ofDrawLine3D(verts[1], verts[5], fov);
  ofDrawLine3D(verts[2], verts[6], fov);
  ofDrawLine3D(verts[3], verts[7], fov);
}

// Draw 3D sphere wireframe
inline void ofDrawSphere3D(float x, float y, float z, float radius,
                           float rotX = 0, float rotY = 0, int resolution = 12, float fov = 500) {
  ofMatrix3 rot = ofMatrix3::rotationX(rotX) * ofMatrix3::rotationY(rotY);

  for (int i = 0; i < resolution; i++) {
    float lat = PI * (float)i / resolution - HALF_PI;
    float nextLat = PI * (float)(i + 1) / resolution - HALF_PI;

    for (int j = 0; j < resolution * 2; j++) {
      float lon = TWO_PI * (float)j / (resolution * 2);
      float nextLon = TWO_PI * (float)(j + 1) / (resolution * 2);

      ofVec3f p1(
        radius * std::cos(lat) * std::cos(lon),
        radius * std::sin(lat),
        radius * std::cos(lat) * std::sin(lon)
      );
      ofVec3f p2(
        radius * std::cos(lat) * std::cos(nextLon),
        radius * std::sin(lat),
        radius * std::cos(lat) * std::sin(nextLon)
      );
      ofVec3f p3(
        radius * std::cos(nextLat) * std::cos(lon),
        radius * std::sin(nextLat),
        radius * std::cos(nextLat) * std::sin(lon)
      );

      p1 = rot.transform(p1);
      p2 = rot.transform(p2);
      p3 = rot.transform(p3);

      p1.x += x; p1.y += y; p1.z += z;
      p2.x += x; p2.y += y; p2.z += z;
      p3.x += x; p3.y += y; p3.z += z;

      ofDrawLine3D(p1, p2, fov);
      ofDrawLine3D(p1, p3, fov);
    }
  }
}

// Draw 3D cone wireframe
inline void ofDrawCone3D(float x, float y, float z, float radius, float height,
                         float rotX = 0, float rotY = 0, int resolution = 12, float fov = 500) {
  ofMatrix3 rot = ofMatrix3::rotationX(rotX) * ofMatrix3::rotationY(rotY);

  ofVec3f tip(0, -height/2, 0);
  tip = rot.transform(tip);
  tip.x += x; tip.y += y; tip.z += z;

  for (int i = 0; i < resolution; i++) {
    float angle = TWO_PI * (float)i / resolution;
    float nextAngle = TWO_PI * (float)(i + 1) / resolution;

    ofVec3f p1(radius * std::cos(angle), height/2, radius * std::sin(angle));
    ofVec3f p2(radius * std::cos(nextAngle), height/2, radius * std::sin(nextAngle));

    p1 = rot.transform(p1);
    p2 = rot.transform(p2);

    p1.x += x; p1.y += y; p1.z += z;
    p2.x += x; p2.y += y; p2.z += z;

    ofDrawLine3D(p1, p2, fov);
    ofDrawLine3D(p1, tip, fov);
  }
}

// Draw 3D cylinder wireframe
inline void ofDrawCylinder3D(float x, float y, float z, float radius, float height,
                             float rotX = 0, float rotY = 0, int resolution = 12, float fov = 500) {
  ofMatrix3 rot = ofMatrix3::rotationX(rotX) * ofMatrix3::rotationY(rotY);

  for (int i = 0; i < resolution; i++) {
    float angle = TWO_PI * (float)i / resolution;
    float nextAngle = TWO_PI * (float)(i + 1) / resolution;

    ofVec3f top1(radius * std::cos(angle), -height/2, radius * std::sin(angle));
    ofVec3f top2(radius * std::cos(nextAngle), -height/2, radius * std::sin(nextAngle));
    ofVec3f bot1(radius * std::cos(angle), height/2, radius * std::sin(angle));
    ofVec3f bot2(radius * std::cos(nextAngle), height/2, radius * std::sin(nextAngle));

    top1 = rot.transform(top1); top2 = rot.transform(top2);
    bot1 = rot.transform(bot1); bot2 = rot.transform(bot2);

    top1.x += x; top1.y += y; top1.z += z;
    top2.x += x; top2.y += y; top2.z += z;
    bot1.x += x; bot1.y += y; bot1.z += z;
    bot2.x += x; bot2.y += y; bot2.z += z;

    ofDrawLine3D(top1, top2, fov);
    ofDrawLine3D(bot1, bot2, fov);
    ofDrawLine3D(top1, bot1, fov);
  }
}

// Draw 3D axis
inline void ofDrawAxis3D(float size, float fov = 500) {
  ofVec3f origin(0, 0, 0);
  ofVec3f xAxis(size, 0, 0);
  ofVec3f yAxis(0, size, 0);
  ofVec3f zAxis(0, 0, size);

  ofSetColor(255, 0, 0);
  ofDrawLine3D(origin, xAxis, fov);

  ofSetColor(0, 255, 0);
  ofDrawLine3D(origin, yAxis, fov);

  ofSetColor(0, 0, 255);
  ofDrawLine3D(origin, zAxis, fov);
}

// Draw 3D grid
inline void ofDrawGrid3D(float stepSize, int numberOfSteps, float fov = 500) {
  float size = stepSize * numberOfSteps;

  ofSetColor(80);
  for (int i = -numberOfSteps; i <= numberOfSteps; i++) {
    float pos = i * stepSize;
    ofDrawLine3D(ofVec3f(-size, 0, pos), ofVec3f(size, 0, pos), fov);
    ofDrawLine3D(ofVec3f(pos, 0, -size), ofVec3f(pos, 0, size), fov);
  }
}

#include "ofSpherePrimitive.h"
#include "../math/ofVectorMath.h"
#include <cmath>

ofSpherePrimitive::ofSpherePrimitive()
  : radius_(50.0f)
  , resolution_(20)
{
  generateMesh();
}

ofSpherePrimitive::ofSpherePrimitive(float radius, int resolution)
  : radius_(radius)
  , resolution_(resolution)
{
  generateMesh();
}

void ofSpherePrimitive::set(float radius, int resolution) {
  radius_ = radius;
  resolution_ = resolution;
  generateMesh();
}

void ofSpherePrimitive::setRadius(float radius) {
  radius_ = radius;
  generateMesh();
}

float ofSpherePrimitive::getRadius() const {
  return radius_;
}

void ofSpherePrimitive::setResolution(int resolution) {
  resolution_ = resolution;
  generateMesh();
}

int ofSpherePrimitive::getResolution() const {
  return resolution_;
}

void ofSpherePrimitive::generateMesh() {
  mesh_.clear();
  mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

  // UV sphere algorithm
  // resolution_ = number of segments in latitude and longitude
  int latSegments = resolution_;
  int lonSegments = resolution_;

  // Generate vertices
  for (int lat = 0; lat <= latSegments; ++lat) {
    float theta = lat * M_PI / latSegments; // 0 to PI (top to bottom)
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);

    for (int lon = 0; lon <= lonSegments; ++lon) {
      float phi = lon * 2.0f * M_PI / lonSegments; // 0 to 2*PI (around)
      float sinPhi = std::sin(phi);
      float cosPhi = std::cos(phi);

      // Vertex position
      float x = radius_ * sinTheta * cosPhi;
      float y = radius_ * cosTheta;
      float z = radius_ * sinTheta * sinPhi;
      glm::vec3 pos(x, y, z);
      mesh_.addVertex(pos);

      // Normal (same as normalized position for sphere centered at origin)
      glm::vec3 normal = glm::normalize(pos);
      mesh_.addNormal(normal);

      // Texture coordinates
      float u = (float)lon / lonSegments;
      float v = (float)lat / latSegments;
      mesh_.addTexCoord(glm::vec2(u, v));

      // Color (white by default)
      mesh_.addColor(ofColor(255, 255, 255, 255));
    }
  }

  // Generate indices (triangles)
  for (int lat = 0; lat < latSegments; ++lat) {
    for (int lon = 0; lon < lonSegments; ++lon) {
      int first = lat * (lonSegments + 1) + lon;
      int second = first + lonSegments + 1;

      // First triangle
      mesh_.addIndex(first);
      mesh_.addIndex(second);
      mesh_.addIndex(first + 1);

      // Second triangle
      mesh_.addIndex(second);
      mesh_.addIndex(second + 1);
      mesh_.addIndex(first + 1);
    }
  }
}

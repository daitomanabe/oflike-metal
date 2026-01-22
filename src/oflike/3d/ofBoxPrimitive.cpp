#include "ofBoxPrimitive.h"
#include "../math/ofVectorMath.h"

ofBoxPrimitive::ofBoxPrimitive()
  : width_(100.0f)
  , height_(100.0f)
  , depth_(100.0f)
  , resolutionWidth_(1)
  , resolutionHeight_(1)
  , resolutionDepth_(1)
{
  generateMesh();
}

ofBoxPrimitive::ofBoxPrimitive(float size)
  : width_(size)
  , height_(size)
  , depth_(size)
  , resolutionWidth_(1)
  , resolutionHeight_(1)
  , resolutionDepth_(1)
{
  generateMesh();
}

ofBoxPrimitive::ofBoxPrimitive(float width, float height, float depth)
  : width_(width)
  , height_(height)
  , depth_(depth)
  , resolutionWidth_(1)
  , resolutionHeight_(1)
  , resolutionDepth_(1)
{
  generateMesh();
}

void ofBoxPrimitive::set(float size) {
  width_ = size;
  height_ = size;
  depth_ = size;
  generateMesh();
}

void ofBoxPrimitive::set(float width, float height, float depth) {
  width_ = width;
  height_ = height;
  depth_ = depth;
  generateMesh();
}

void ofBoxPrimitive::setWidth(float w) {
  width_ = w;
  generateMesh();
}

void ofBoxPrimitive::setHeight(float h) {
  height_ = h;
  generateMesh();
}

void ofBoxPrimitive::setDepth(float d) {
  depth_ = d;
  generateMesh();
}

float ofBoxPrimitive::getWidth() const { return width_; }
float ofBoxPrimitive::getHeight() const { return height_; }
float ofBoxPrimitive::getDepth() const { return depth_; }

void ofBoxPrimitive::setResolution(int res) {
  resolutionWidth_ = res;
  resolutionHeight_ = res;
  resolutionDepth_ = res;
  generateMesh();
}

void ofBoxPrimitive::setResolutionWidth(int res) {
  resolutionWidth_ = res;
  generateMesh();
}

void ofBoxPrimitive::setResolutionHeight(int res) {
  resolutionHeight_ = res;
  generateMesh();
}

void ofBoxPrimitive::setResolutionDepth(int res) {
  resolutionDepth_ = res;
  generateMesh();
}

int ofBoxPrimitive::getResolutionWidth() const { return resolutionWidth_; }
int ofBoxPrimitive::getResolutionHeight() const { return resolutionHeight_; }
int ofBoxPrimitive::getResolutionDepth() const { return resolutionDepth_; }

void ofBoxPrimitive::generateMesh() {
  mesh_.clear();
  mesh_.setMode(OF_PRIMITIVE_TRIANGLES);

  float halfW = width_ * 0.5f;
  float halfH = height_ * 0.5f;
  float halfD = depth_ * 0.5f;

  // Helper to add a face with resolution subdivision
  auto addFace = [&](glm::vec3 origin, glm::vec3 right, glm::vec3 up, glm::vec3 normal, int resU, int resV) {
    int startIndex = mesh_.getNumVertices();

    // Generate grid of vertices
    for (int v = 0; v <= resV; v++) {
      for (int u = 0; u <= resU; u++) {
        float uf = (float)u / (float)resU;
        float vf = (float)v / (float)resV;

        glm::vec3 pos = origin + right * uf + up * vf;
        mesh_.addVertex(pos);
        mesh_.addNormal(normal);
        mesh_.addTexCoord(glm::vec2(uf, vf));
      }
    }

    // Generate triangles
    for (int v = 0; v < resV; v++) {
      for (int u = 0; u < resU; u++) {
        int i0 = startIndex + v * (resU + 1) + u;
        int i1 = i0 + 1;
        int i2 = i0 + (resU + 1);
        int i3 = i2 + 1;

        // Two triangles per quad
        mesh_.addIndex(i0);
        mesh_.addIndex(i1);
        mesh_.addIndex(i2);

        mesh_.addIndex(i1);
        mesh_.addIndex(i3);
        mesh_.addIndex(i2);
      }
    }
  };

  // Front face (looking at +Z)
  addFace(
    glm::vec3(-halfW, -halfH, halfD),  // origin
    glm::vec3(width_, 0, 0),            // right
    glm::vec3(0, height_, 0),           // up
    glm::vec3(0, 0, 1),                 // normal
    resolutionWidth_, resolutionHeight_
  );

  // Right face (looking at +X)
  addFace(
    glm::vec3(halfW, -halfH, halfD),
    glm::vec3(0, 0, -depth_),
    glm::vec3(0, height_, 0),
    glm::vec3(1, 0, 0),
    resolutionDepth_, resolutionHeight_
  );

  // Left face (looking at -X)
  addFace(
    glm::vec3(-halfW, -halfH, -halfD),
    glm::vec3(0, 0, depth_),
    glm::vec3(0, height_, 0),
    glm::vec3(-1, 0, 0),
    resolutionDepth_, resolutionHeight_
  );

  // Back face (looking at -Z)
  addFace(
    glm::vec3(halfW, -halfH, -halfD),
    glm::vec3(-width_, 0, 0),
    glm::vec3(0, height_, 0),
    glm::vec3(0, 0, -1),
    resolutionWidth_, resolutionHeight_
  );

  // Top face (looking at +Y)
  addFace(
    glm::vec3(-halfW, halfH, -halfD),
    glm::vec3(width_, 0, 0),
    glm::vec3(0, 0, depth_),
    glm::vec3(0, 1, 0),
    resolutionWidth_, resolutionDepth_
  );

  // Bottom face (looking at -Y)
  addFace(
    glm::vec3(-halfW, -halfH, halfD),
    glm::vec3(width_, 0, 0),
    glm::vec3(0, 0, -depth_),
    glm::vec3(0, -1, 0),
    resolutionWidth_, resolutionDepth_
  );
}

ofMesh ofBoxPrimitive::getSideMesh(int side) const {
  if (side < 0 || side >= SIDES_TOTAL) {
    return ofMesh();
  }

  ofMesh sideMesh;
  sideMesh.setMode(OF_PRIMITIVE_TRIANGLES);

  // Calculate vertices per face
  int resU = (side == SIDE_FRONT || side == SIDE_BACK) ? resolutionWidth_ : resolutionDepth_;
  int resV = (side == SIDE_TOP || side == SIDE_BOTTOM) ? resolutionDepth_ : resolutionHeight_;
  int vertsPerFace = (resU + 1) * (resV + 1);
  int indicesPerFace = resU * resV * 6;

  // Calculate starting index for this face
  int faceStartVertex = 0;
  int faceStartIndex = 0;
  for (int i = 0; i < side; i++) {
    int prevResU = (i == SIDE_FRONT || i == SIDE_BACK) ? resolutionWidth_ : resolutionDepth_;
    int prevResV = (i == SIDE_TOP || i == SIDE_BOTTOM) ? resolutionDepth_ : resolutionHeight_;
    faceStartVertex += (prevResU + 1) * (prevResV + 1);
    faceStartIndex += prevResU * prevResV * 6;
  }

  // Extract vertices for this face
  const auto& vertices = mesh_.getVertices();
  const auto& normals = mesh_.getNormals();
  const auto& texCoords = mesh_.getTexCoords();

  for (int i = 0; i < vertsPerFace; i++) {
    int idx = faceStartVertex + i;
    if (idx < vertices.size()) {
      sideMesh.addVertex(vertices[idx]);
      if (idx < normals.size()) sideMesh.addNormal(normals[idx]);
      if (idx < texCoords.size()) sideMesh.addTexCoord(texCoords[idx]);
    }
  }

  // Extract indices for this face (remapped to local index space)
  const auto& indices = mesh_.getIndices();
  for (int i = 0; i < indicesPerFace; i++) {
    int idx = faceStartIndex + i;
    if (idx < indices.size()) {
      uint32_t localIndex = indices[idx] - faceStartVertex;
      sideMesh.addIndex(localIndex);
    }
  }

  return sideMesh;
}

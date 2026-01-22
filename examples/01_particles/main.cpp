// Particles Example - ported from openFrameworks
// Demonstrates: particles, vectors, random, alpha blending

#include "../../src/oflike/ofMain.h"
#include <vector>

struct Particle {
  ofVec2f pos;
  ofVec2f vel;
  ofVec2f acc;
  float life;
  float maxLife;
  ofColor color;

  Particle(float x, float y) {
    pos = ofVec2f(x, y);
    vel = ofVec2f(ofRandom(-2, 2), ofRandom(-2, 2));
    acc = ofVec2f(0, 0);
    maxLife = ofRandom(60, 120);
    life = maxLife;
    color = ofColor::fromHsb(ofRandom(255), 200, 255);
  }

  void applyForce(ofVec2f force) {
    acc += force;
  }

  void update() {
    vel += acc;
    pos += vel;
    acc = ofVec2f(0, 0);
    life -= 1;

    // Friction
    vel *= 0.98f;
  }

  void draw() {
    float alpha = ofMap(life, 0, maxLife, 0, 255);
    ofSetColor(color.r, color.g, color.b, (int)alpha);
    float size = ofMap(life, 0, maxLife, 1, 8);
    ofDrawCircle(pos.x, pos.y, size);
  }

  bool isDead() {
    return life <= 0;
  }
};

class ParticlesApp : public AppBase {
public:
  std::vector<Particle> particles;
  ofVec2f gravity{0, 0.1f};
  bool mouseDown = false;

  void setup() override {
    ofBackground(0);
  }

  void update() override {
    // Add particles at mouse position when mouse is pressed
    if (mouseDown) {
      for (int i = 0; i < 5; i++) {
        particles.emplace_back(ofGetMouseX(), ofGetMouseY());
      }
    }

    // Update all particles
    for (auto& p : particles) {
      p.applyForce(gravity);
      p.update();

      // Bounce off walls
      if (p.pos.x < 0 || p.pos.x > ofGetWidth()) {
        p.vel.x *= -0.8f;
        p.pos.x = ofClamp(p.pos.x, 0, (float)ofGetWidth());
      }
      if (p.pos.y > ofGetHeight()) {
        p.vel.y *= -0.8f;
        p.pos.y = ofGetHeight();
      }
    }

    // Remove dead particles
    particles.erase(
      std::remove_if(particles.begin(), particles.end(),
        [](Particle& p) { return p.isDead(); }),
      particles.end()
    );
  }

  void draw() override {
    ofFill();

    // Draw all particles
    for (auto& p : particles) {
      p.draw();
    }

    // UI
    ofSetColor(255);
    ofDrawBitmapString("Particles: " + ofToString(particles.size()), 10, 20);
    ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate(), 1), 10, 35);
    ofDrawBitmapString("Click and drag to emit particles", 10, ofGetHeight() - 20);
  }

  void mousePressed(float x, float y, int button) override {
    mouseDown = true;
    // Burst of particles on click
    for (int i = 0; i < 50; i++) {
      particles.emplace_back(x, y);
    }
  }

  void mouseReleased(float x, float y, int button) override {
    mouseDown = false;
  }

  void mouseDragged(float x, float y, int button) override {
    mouseDown = true;
  }

  void keyPressed(int key) override {
    if (key == 'c' || key == 'C') {
      particles.clear();
    }
    if (key == ' ') {
      // Burst at center
      for (int i = 0; i < 100; i++) {
        Particle p(ofGetWidth() / 2, ofGetHeight() / 2);
        p.vel = ofVec2f(ofRandom(-5, 5), ofRandom(-10, -2));
        particles.push_back(p);
      }
    }
  }
};

OF_MAIN(ParticlesApp);

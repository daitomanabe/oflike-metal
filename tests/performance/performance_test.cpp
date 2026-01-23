// Performance Tests for oflike-metal
// Benchmarks drawing operations and measures performance metrics (Phase 17.2)

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <numeric>
#include <algorithm>
#include <cmath>

// Include all graphics headers
#include "ofGraphics.h"
#include "ofColor.h"
#include "ofVec2f.h"
#include "ofVec3f.h"
#include "ofMatrix4x4.h"
#include "ofPath.h"
#include "ofPolyline.h"

// ANSI color codes for output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define CYAN "\033[36m"
#define RESET "\033[0m"

// Performance metrics
struct PerfMetrics {
    std::string name;
    std::vector<double> timings_ms;
    size_t iterations;

    double mean() const {
        if (timings_ms.empty()) return 0.0;
        return std::accumulate(timings_ms.begin(), timings_ms.end(), 0.0) / timings_ms.size();
    }

    double median() const {
        if (timings_ms.empty()) return 0.0;
        auto sorted = timings_ms;
        std::sort(sorted.begin(), sorted.end());
        size_t mid = sorted.size() / 2;
        if (sorted.size() % 2 == 0) {
            return (sorted[mid - 1] + sorted[mid]) / 2.0;
        }
        return sorted[mid];
    }

    double min() const {
        if (timings_ms.empty()) return 0.0;
        return *std::min_element(timings_ms.begin(), timings_ms.end());
    }

    double max() const {
        if (timings_ms.empty()) return 0.0;
        return *std::max_element(timings_ms.begin(), timings_ms.end());
    }

    double stddev() const {
        if (timings_ms.empty()) return 0.0;
        double m = mean();
        double variance = 0.0;
        for (const auto& t : timings_ms) {
            variance += (t - m) * (t - m);
        }
        return std::sqrt(variance / timings_ms.size());
    }

    double ops_per_second() const {
        double m = mean();
        if (m == 0.0) return 0.0;
        return (iterations * 1000.0) / m;
    }
};

// Benchmark utilities
class PerformanceBenchmark {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    void start() {
        start_time_ = Clock::now();
    }

    double stop_ms() {
        auto end_time = Clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
        return duration.count() / 1000.0;
    }

private:
    TimePoint start_time_;
};

// ============================================================
// Benchmark Functions
// ============================================================

PerfMetrics benchmark_2d_primitives(size_t iterations = 10000) {
    PerfMetrics metrics;
    metrics.name = "2D Primitives (1000 shapes each)";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            float x = (i % 100) * 10.0f;
            float y = (i / 100) * 10.0f;

            ofDrawCircle(x, y, 5.0f);
            ofDrawRectangle(x + 100, y, 10, 10);
            ofDrawEllipse(x + 200, y, 15, 10);
            ofDrawTriangle(x, y, x + 10, y, x + 5, y + 10);
            ofDrawLine(x, y, x + 10, y + 10);
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_3d_primitives(size_t iterations = 1000) {
    PerfMetrics metrics;
    metrics.name = "3D Primitives (100 meshes each)";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            float x = (i % 10) * 20.0f;
            float y = (i / 10) * 20.0f;

            ofDrawBox(x, y, 0, 10);
            ofDrawSphere(x + 50, y, 0, 5);
            ofDrawCone(x + 100, y, 0, 5, 10);
            ofDrawCylinder(x + 150, y, 0, 5, 10);
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_matrix_operations(size_t iterations = 100000) {
    PerfMetrics metrics;
    metrics.name = "Matrix Operations";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofPushMatrix();
            ofTranslate(10.0f, 20.0f, 30.0f);
            ofRotateX(45.0f);
            ofRotateY(30.0f);
            ofRotateZ(60.0f);
            ofScale(2.0f, 2.0f, 2.0f);
            ofPopMatrix();
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_color_operations(size_t iterations = 1000000) {
    PerfMetrics metrics;
    metrics.name = "Color Operations";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofColor c1(255, 128, 64);
            ofColor c2 = ofColor::fromHsb(180, 255, 255);
            ofColor c3 = c1.getLerped(c2, 0.5f);
            ofColor c4 = c3.getInverted();
            float hue = c4.getHue();
            (void)hue; // Suppress unused variable warning
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_path_construction(size_t iterations = 10000) {
    PerfMetrics metrics;
    metrics.name = "Path Construction";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofPath path;
            path.moveTo(0, 0);
            path.lineTo(100, 0);
            path.lineTo(100, 100);
            path.lineTo(0, 100);
            path.close();
            path.curveTo(50, 50);
            path.bezierTo(50, 100, 100, 100, 100, 50);
            path.arc(200, 200, 50, 50, 0, 180);
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_polyline_operations(size_t iterations = 10000) {
    PerfMetrics metrics;
    metrics.name = "Polyline Operations";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofPolyline poly;
            poly.addVertex(0, 0);
            poly.addVertex(100, 0);
            poly.addVertex(100, 100);
            poly.addVertex(0, 100);
            poly.close();

            float perimeter = poly.getPerimeter();
            float area = poly.getArea();
            ofVec2f centroid = poly.getCentroid2D();
            (void)perimeter; (void)area; (void)centroid;
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_shape_building(size_t iterations = 10000) {
    PerfMetrics metrics;
    metrics.name = "Shape Building (ofBeginShape/ofEndShape)";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofBeginShape();
            ofVertex(0, 0);
            ofVertex(100, 0);
            ofVertex(100, 100);
            ofVertex(0, 100);
            ofEndShape(true);
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

PerfMetrics benchmark_vector_math(size_t iterations = 1000000) {
    PerfMetrics metrics;
    metrics.name = "Vector Math";
    metrics.iterations = iterations;

    const size_t runs = 10;
    for (size_t run = 0; run < runs; ++run) {
        PerformanceBenchmark bench;
        bench.start();

        for (size_t i = 0; i < iterations; ++i) {
            ofVec3f v1(1.0f, 2.0f, 3.0f);
            ofVec3f v2(4.0f, 5.0f, 6.0f);
            ofVec3f v3 = v1 + v2;
            ofVec3f v4 = v1.cross(v2);
            float d = v1.dot(v2);
            float len = v3.length();
            ofVec3f normalized = v4.getNormalized();
            (void)d; (void)len; (void)normalized;
        }

        metrics.timings_ms.push_back(bench.stop_ms());
    }

    return metrics;
}

// ============================================================
// Results Display
// ============================================================

void print_metrics(const PerfMetrics& metrics) {
    std::cout << "\n" << CYAN << "[BENCHMARK] " << metrics.name << RESET << "\n";
    std::cout << "  Iterations:    " << metrics.iterations << " ops/run\n";
    std::cout << "  Runs:          " << metrics.timings_ms.size() << "\n";
    std::cout << "  Mean:          " << metrics.mean() << " ms\n";
    std::cout << "  Median:        " << metrics.median() << " ms\n";
    std::cout << "  Min:           " << metrics.min() << " ms\n";
    std::cout << "  Max:           " << metrics.max() << " ms\n";
    std::cout << "  Std Dev:       " << metrics.stddev() << " ms\n";
    std::cout << GREEN << "  Throughput:    " << metrics.ops_per_second() << " ops/sec" << RESET << "\n";
}

void print_summary(const std::vector<PerfMetrics>& all_metrics) {
    std::cout << "\n" << std::string(70, '=') << "\n";
    std::cout << "PERFORMANCE TEST SUMMARY\n";
    std::cout << std::string(70, '=') << "\n";

    std::cout << "\n" << YELLOW << "Throughput Comparison:" << RESET << "\n";
    for (const auto& m : all_metrics) {
        std::cout << "  " << m.name << ": "
                  << GREEN << m.ops_per_second() << " ops/sec" << RESET << "\n";
    }

    std::cout << "\n" << BLUE << "Note: These benchmarks measure API overhead and call performance.\n";
    std::cout << "GPU rendering performance requires running actual applications with\n";
    std::cout << "frame timing measurements using ofGetFrameRate() and ofGetLastFrameTime().\n";
    std::cout << "\nFor visual rendering performance, run examples and monitor FPS:\n";
    std::cout << "  - examples/06_3d_primitives (many 3D shapes)\n";
    std::cout << "  - examples/09_mesh (large meshes)\n";
    std::cout << "  - examples/10_fbo (complex rendering pipelines)" << RESET << "\n";

    std::cout << std::string(70, '=') << "\n";
    std::cout << GREEN << "\n✓ ALL PERFORMANCE BENCHMARKS COMPLETE!\n" << RESET;
    std::cout << std::string(70, '=') << "\n";
}

// ============================================================
// Main Test Runner
// ============================================================

int main(int argc, char** argv) {
    std::cout << "\n";
    std::cout << std::string(70, '=') << "\n";
    std::cout << "OFLIKE-METAL PERFORMANCE TESTS\n";
    std::cout << "Phase 17.2: パフォーマンステスト\n";
    std::cout << std::string(70, '=') << "\n";

    std::vector<PerfMetrics> all_metrics;

    try {
        std::cout << "\n" << YELLOW << "=== Running Performance Benchmarks ===" << RESET << "\n";
        std::cout << "(This may take a few minutes...)\n";

        // Run all benchmarks
        std::cout << "\n[1/8] Benchmarking vector math..." << std::flush;
        auto m1 = benchmark_vector_math();
        print_metrics(m1);
        all_metrics.push_back(m1);

        std::cout << "\n[2/8] Benchmarking color operations..." << std::flush;
        auto m2 = benchmark_color_operations();
        print_metrics(m2);
        all_metrics.push_back(m2);

        std::cout << "\n[3/8] Benchmarking matrix operations..." << std::flush;
        auto m3 = benchmark_matrix_operations();
        print_metrics(m3);
        all_metrics.push_back(m3);

        std::cout << "\n[4/8] Benchmarking path construction..." << std::flush;
        auto m4 = benchmark_path_construction();
        print_metrics(m4);
        all_metrics.push_back(m4);

        std::cout << "\n[5/8] Benchmarking polyline operations..." << std::flush;
        auto m5 = benchmark_polyline_operations();
        print_metrics(m5);
        all_metrics.push_back(m5);

        std::cout << "\n[6/8] Benchmarking shape building..." << std::flush;
        auto m6 = benchmark_shape_building();
        print_metrics(m6);
        all_metrics.push_back(m6);

        std::cout << "\n[7/8] Benchmarking 2D primitives..." << std::flush;
        auto m7 = benchmark_2d_primitives();
        print_metrics(m7);
        all_metrics.push_back(m7);

        std::cout << "\n[8/8] Benchmarking 3D primitives..." << std::flush;
        auto m8 = benchmark_3d_primitives();
        print_metrics(m8);
        all_metrics.push_back(m8);

    } catch (const std::exception& e) {
        std::cout << RED << "\nException caught: " << e.what() << RESET << "\n";
        return 1;
    }

    print_summary(all_metrics);

    return 0;
}

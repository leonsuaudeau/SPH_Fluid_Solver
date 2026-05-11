#include "screen_recorder.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include "glad/glad.h"

void ScreenRecorder::update(const float dt, const bool enabled, const int width, const int height) {
    accumulated_time += dt;
    if (accumulated_time > 0.0167f && enabled) {
        accumulated_time = 0;
        std::vector<unsigned char> pixels(width * height * 3);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
        std::ofstream out("../output/frame_" + std::to_string(frame_count++) + ".ppm", std::ios::binary);
        out << "P6\n" << width << " " << height << "\n255\n";

        for (int y = height - 1; y >= 0; --y) {
            out.write(reinterpret_cast<char*>(pixels.data() + y * width * 3), width * 3);
        }

        out.close();
    }
}

#include <vector>
#include "particle.h"

int main() {
    std::vector<Particle2D> particles;

    for (int y = -10; y <= 10; y++) {
        for (int x = -10; x <= 10; x++) {
            particles.emplace_back(Particle2D(x, y, 0, 0, 1));
        }
    }

    return 0;
}
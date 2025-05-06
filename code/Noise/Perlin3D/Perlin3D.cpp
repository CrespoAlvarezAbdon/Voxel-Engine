#include "Perlin3D.hpp"

#include <algorithm>
#include <random>
#include <numeric>

namespace VoxelEng {

	const unsigned int Perlin3D::nPermutations_ = 256;

	void Perlin3D::setSeed(unsigned int seed) {

        noise::setSeed(seed);

		permutations.resize(nPermutations_);
		std::iota(permutations.begin(), permutations.end(), 0);

		std::default_random_engine engine(seed);
		std::shuffle(permutations.begin(), permutations.end(), engine);

		permutations.insert(permutations.end(), permutations.begin(), permutations.end());

	}

	float Perlin3D::get(float x, float y, float z) {
	
        x += 0.0001;
        y += 0.0001;
        z += 0.0001;

        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        // Find relative x, y, z of point in cube
        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        // Compute fade curves
        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        // Hash coordinates of the cube corners
        int A = permutations[X] + Y, AA = permutations[A] + Z, AB = permutations[A + 1] + Z;
        int B = permutations[X + 1] + Y, BA = permutations[B] + Z, BB = permutations[B + 1] + Z;

        // Blend results from 8 corners of cube
        return lerp(w,
            lerp(v,
                lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)),
                lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))
            ),
            lerp(v,
                lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))
            )
        );
	
	}

    double Perlin3D::fade(double t) {

        return t * t * t * (t * (t * 6 - 15) + 10);

    }

    double Perlin3D::lerp(double t, double a, double b) {

        return a + t * (b - a);

    }

    double Perlin3D::grad(int hash, double x, double y, double z) {

        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);

    }

}
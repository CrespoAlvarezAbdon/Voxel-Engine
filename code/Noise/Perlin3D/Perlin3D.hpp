#ifndef _VOXELENG_PERLIN3D_
#define _VOXELENG_PERLIN3D_

#include <vector>
#include <Noise/noise.h>

namespace VoxelEng {

	class Perlin3D : public noise {

	public:

		// Observers.

		/**
		* @brief Get noise value at the given coordinates.
		* @param x First dimension coordinate.
		* @param y Second dimension coordinate.
		* @param z Third dimension coordinate.
		*/
		float get(float x, float y, float z);


		// Modifiers.

		void setSeed(unsigned int seed);

	private:
		static const unsigned int nPermutations_;

		std::vector<int> permutations;

		static double fade(double t);

		static double lerp(double t, double a, double b);

		static double grad(int hash, double x, double y, double z);

	};

}

#endif
#ifndef _VOXELENG_BLOCK_LIGHT_
#define _VOXELENG_BLOCK_LIGHT_

#include <cstdint>

#include <utilities.h>
#include <vec.h>
#include <Graphics/Lighting/definitions.hpp>

namespace VoxelEng {

	/**
	* @brief RGBA light applied to blocks in a chunk.
	*/
	struct blockLight {

	public:

		// Constructors.

		/**
		* @brief Default class constructor.
		*/
		blockLight();

		/**
		* @brief Class constructor.
		* @param redI Red intensity.
		* @param greenI Green intensity.
		* @param blueI Blue intensity.
		* @param alphaI Alpha intensity.
		* @param redV Red value.
		* @param greenV Green value.
		* @param blueV Blue value.
		* @param alphaV Alpha value.
		*/
		blockLight(lightIntensity redI, lightIntensity greenI, lightIntensity blueI, lightIntensity alphaI,
			lightValue redV, lightValue greenV, lightValue blueV, lightValue alphaV);

		/**
		* @brief Class constructor.
		* @param rgbaI RGBA vector to get each channel intensity from.
		* @param rgbaV RGBA vector to get each channel values from.
		*/
		explicit blockLight(const basicUVec4& rgbaI, const basicVec4& rgbaV);


		// Observers.

		/**
		* @brief Check whether the light intensity from c1 is greather than the one from c2 or otherwise.
		* Positive light intensity is always greater than negative light intensity.
		* @param c1 The first light intensity.
		* @param c2 The second light intensity.
		* @return Whether the light intensity from c1 is greather than the one from c2 (true) or otherwise (false).
		*/
		static bool greaterThan(lightIntensity c1, lightIntensity c2);

		lightValue getWithIntensity(colorChannel channel) const;

		lightIntensity redIntensity() const;

		lightIntensity greenIntensity() const;

		lightIntensity blueIntensity() const;

		lightIntensity alphaIntensity() const;

		lightIntensity intensity(colorChannel channel) const;

		lightValue redValue() const;

		lightValue greenValue() const;

		lightValue blueValue() const;

		lightValue alphaValue() const;

		lightValue value(colorChannel channel) const;

		/**
		* @brief Get whether all the intensity and values for all channels for this blocklight are 0.
		* @returns Whether all the intensity and values for all channels for this blocklight are 0 (true) or otherwise (false).
		*/
		bool isZero() const;


		/**
		* @brief Obtain this block light's with a specified atenuation applied to all specified channels.
		* @param channel Channels to apply the atenuation.
		*/
		blockLight decreased(colorChannel channel, byte atenuation = 1) const;


		// Modifiers.

		void fromRGBA(const basicUVec4& rgbaIntensity, const basicVec4& rgbaColor);

		void redIntensity(lightIntensity intensity);

		void greenIntensity(lightIntensity intensity);

		void blueIntensity(lightIntensity intensity);

		void alphaIntensity(lightIntensity intensity);

		void intensity(lightIntensity intensity, colorChannel channel);

		void redValue(lightValue value);

		void greenValue(lightValue value);

		void blueValue(lightValue value);

		void alphaValue(lightValue value);

		void value(lightValue value, colorChannel channel);

		void intensityAndValue(lightIntensity intensity, lightValue value, colorChannel channel);

		void copy(const blockLight& source, colorChannel channel);

		void clear();

	private:

		uint16_t intensityBits_;
		basicVec4 value_;

	};

	inline blockLight::blockLight()
	: intensityBits_(0), value_(basicVec4Zero)
	{}

	inline blockLight::blockLight(
		lightIntensity redI, lightIntensity greenI, lightIntensity blueI, lightIntensity alphaI,
		lightValue red, lightValue greenV, lightValue blueV, lightValue alphaV)
	: intensityBits_(0), value_(basicVec4Zero) {
	
		redIntensity(redI);
		greenIntensity(greenI);
		blueIntensity(blueI);
		alphaIntensity(alphaI);
	
	}

	inline blockLight::blockLight(const basicUVec4& rgbaI, const basicVec4& rgbaC)
		: intensityBits_(0), value_(basicVec4Zero) {

		fromRGBA(rgbaI, rgbaC);

	}

	inline lightValue blockLight::getWithIntensity(colorChannel channel) const {
	
		return value(channel) * (intensity(channel) / static_cast<float>(kLightMaxIntensity));
	
	}

	inline lightIntensity blockLight::redIntensity() const {
	
		return static_cast<sbyte>((intensityBits_ >> 12) & 0xF);
	
	}

	inline lightIntensity blockLight::greenIntensity() const {

		return static_cast<sbyte>((intensityBits_ >> 8) & 0xF);

	}

	inline lightIntensity blockLight::blueIntensity() const {

		return static_cast<sbyte>((intensityBits_ >> 4) & 0xF);

	}

	inline lightIntensity blockLight::alphaIntensity() const {

		return static_cast<sbyte>(intensityBits_ & 0xF);

	}

	inline void blockLight::redIntensity(lightIntensity intensity) {

		intensityBits_ = (intensityBits_ & ~(0xF << 12)) | ((intensity & 0xF) << 12);

	}

	inline void blockLight::greenIntensity(lightIntensity intensity) {

		intensityBits_ = (intensityBits_ & ~(0xF << 8)) | ((intensity & 0xF) << 8);

	}

	inline void blockLight::blueIntensity(lightIntensity intensity) {

		intensityBits_ = (intensityBits_ & ~(0xF << 4)) | ((intensity & 0xF) << 4);

	}

	inline void blockLight::alphaIntensity(lightIntensity intensity) {

		intensityBits_ = (intensityBits_ & ~0xF) | (intensity & 0xF);

	}

	inline void blockLight::redValue(lightValue value) {

		value_.x = value;

	}

	inline void blockLight::greenValue(lightValue value) {

		value_.y = value;

	}

	inline void blockLight::blueValue(lightValue value) {

		value_.z = value;

	}

	inline void blockLight::alphaValue(lightValue value) {

		value_.w = value;

	}

	inline lightValue blockLight::redValue() const {

		return value_.x;

	}

	inline lightValue blockLight::greenValue() const {

		return value_.y;

	}

	inline lightValue blockLight::blueValue() const {

		return value_.z;

	}

	inline lightValue blockLight::alphaValue() const {

		return value_.w;

	}

	inline bool blockLight::isZero() const {
	
		return intensityBits_ == 0 && value_ == basicVec4Zero;
	
	}

	inline void blockLight::clear() {
	
		intensityBits_ = 0;
		value_ = basicVec4Zero;
	
	}

}

#endif
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

		// Initializers.

		/**
		* @brief Initialize the blockLight system.
		*/
		static void init();


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
		explicit blockLight(const basicUVec4& rgbaI, const basicVec4& rgbaColor);


		// Observers.

		/**
		* @brief Check whether the light intensity from c1 is greather than the one from c2 or otherwise.
		* Positive light intensity is always greater than negative light intensity.
		* @param c1 The first light intensity.
		* @param c2 The second light intensity.
		* @return Whether the light intensity from c1 is greather than the one from c2 (true) or otherwise (false).
		*/
		static bool greaterThan(lightIntensity c1, lightIntensity c2);

		static bool initialised();

		static const blockLight& zero();

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

		/**
		* @brief Get whether the block lights are equal or not.
		* @param l Left operand.
		* @return Whether the block lights are equal (true) or not (false).
		*/
		bool operator==(const blockLight& l) const;


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


		// Deinitializers.

		/**
		* @brief Deinitialize the blockLight system.
		*/
		static void reset();

	private:

		static blockLight kBlockLightZero_;
		static bool initialised_;

		uint16_t intensityBits_;
		basicUVec4 values_;
		basicUVec4 negativeValues_;

	};

	inline void blockLight::init() {
	
		kBlockLightZero_ = blockLight();
	
	}

	inline blockLight::blockLight()
	: intensityBits_(0), values_(basicUVec4Zero), negativeValues_(basicUVec4Zero)
	{}

	inline blockLight::blockLight(
		lightIntensity redI, lightIntensity greenI, lightIntensity blueI, lightIntensity alphaI,
		lightValue redV, lightValue greenV, lightValue blueV, lightValue alphaV)
	: intensityBits_(0), values_(basicUVec4Zero), negativeValues_(basicUVec4Zero) {
	
		redIntensity(redI);
		greenIntensity(greenI);
		blueIntensity(blueI);
		alphaIntensity(alphaI);
		redValue(redV);
		greenValue(greenV);
		blueValue(blueV);
		alphaValue(alphaV);
	
	}

	inline blockLight::blockLight(const basicUVec4& rgbaI, const basicVec4& rgbaColor)
	: intensityBits_(0), values_(basicUVec4Zero), negativeValues_(basicUVec4Zero) {

		fromRGBA(rgbaI, rgbaColor);

	}

	inline bool blockLight::initialised() {
	
		return initialised_;
	
	}

	inline const blockLight& blockLight::zero() {
	
		return kBlockLightZero_;
	
	}

	inline lightValue blockLight::getWithIntensity(colorChannel channel) const {
	
		return value(channel) * (intensity(channel) / static_cast<float>(kLightMaxIntensity));
	
	}

	inline lightIntensity blockLight::redIntensity() const {
	
		return static_cast<byte>((intensityBits_ >> 12) & 0xF);
	
	}

	inline lightIntensity blockLight::greenIntensity() const {

		return static_cast<byte>((intensityBits_ >> 8) & 0xF);

	}

	inline lightIntensity blockLight::blueIntensity() const {

		return static_cast<byte>((intensityBits_ >> 4) & 0xF);

	}

	inline lightIntensity blockLight::alphaIntensity() const {

		return static_cast<byte>(intensityBits_ & 0xF);

	}

	inline bool blockLight::operator==(const blockLight& l) const {
	
		return intensityBits_ == l.intensityBits_ && values_ == l.values_ && negativeValues_ == l.negativeValues_;
	
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

		if (value > 0)
			values_.x = value;
		else if (value < 0)
			negativeValues_.x = value;
		else {
		
			values_.x = value;
			negativeValues_.x = value;
		
		}

	}

	inline void blockLight::greenValue(lightValue value) {

		if (value > 0)
			values_.y = value;
		else if (value < 0)
			negativeValues_.y = value;
		else {

			values_.y = value;
			negativeValues_.y = value;

		}

	}

	inline void blockLight::blueValue(lightValue value) {

		if (value > 0)
			values_.z = value;
		else if (value < 0)
			negativeValues_.z = value;
		else {

			values_.z = value;
			negativeValues_.z = value;

		}

	}

	inline void blockLight::alphaValue(lightValue value) {

		if (value > 0)
			values_.w = value;
		else if (value < 0)
			negativeValues_.w = value;
		else {

			values_.w = value;
			negativeValues_.w = value;

		}

	}

	inline lightValue blockLight::redValue() const {
		
		return values_.x - negativeValues_.x;

	}

	inline lightValue blockLight::greenValue() const {

		return values_.y - negativeValues_.y;

	}

	inline lightValue blockLight::blueValue() const {

		return values_.z - negativeValues_.z;

	}

	inline lightValue blockLight::alphaValue() const {

		return values_.w - negativeValues_.w;

	}

	inline bool blockLight::isZero() const {
	
		return *this == kBlockLightZero_;
	
	}

	inline void blockLight::clear() {
	
		intensityBits_ = 0;
		values_ = basicUVec4Zero;
		negativeValues_ = basicUVec4Zero;
	
	}

	inline void blockLight::reset() 
	{}

}

#endif
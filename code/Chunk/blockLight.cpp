#include "blockLight.hpp"

namespace VoxelEng {

	bool blockLight::greaterThan(lightIntensity c1, lightIntensity c2) {

		return 
			c1 > 0 && c2 > 0 ? c1 > c2 : 
			c1 < 0 && c2 < 0 ? c1 < c2 :
			c1 < 0 && c2 > 0 ? c2 :
			c1 > 0 && c2 < 0 ? c1 : true;
	
	}

	lightIntensity blockLight::intensity(colorChannel channel) const {
	
		switch (channel) {

			case colorChannel::RED:
				return redIntensity();
				break;
			case colorChannel::GREEN:
				return greenIntensity();
				break;
			case colorChannel::BLUE:
				return blueIntensity();
				break;
			case colorChannel::ALPHA:
				return alphaIntensity();
				break;
			case colorChannel::ALL:
				logger::errorLog("colorChannel::ALL cannot be used on this method");
				break;
			case colorChannel::RGB:
				logger::errorLog("colorChannel::RGB cannot be used on this method");
				break;
			default:
				logger::errorLog("Unsupported color channel");
				break;

		}
	
	}

	lightValue blockLight::value(colorChannel channel) const {

		switch (channel) {

			case colorChannel::RED:
				return redValue();
				break;
			case colorChannel::GREEN:
				return greenValue();
				break;
			case colorChannel::BLUE:
				return blueValue();
				break;
			case colorChannel::ALPHA:
				return alphaValue();
				break;
			case colorChannel::ALL:
				logger::errorLog("colorChannel::ALL cannot be used on this method");
				break;
			case colorChannel::RGB:
				logger::errorLog("colorChannel::RGB cannot be used on this method");
				break;
			default:
				logger::errorLog("Unsupported color channel");
				break;

		}

	}

	blockLight blockLight::decreased(colorChannel channel, byte atenuation) const {
	
		blockLight bl = *this;
		lightIntensity red = bl.redIntensity();
		lightIntensity green = bl.greenIntensity();
		lightIntensity blue = bl.blueIntensity();
		lightIntensity alpha = bl.alphaIntensity();
		
		switch (channel) {

		case colorChannel::RED:
			bl.redIntensity(red > 0 ? red - atenuation : red < 0 ? red + atenuation : red);
			break;
		case colorChannel::GREEN:
			bl.greenIntensity(green > 0 ? green - atenuation : green < 0 ? green + atenuation : green);
			break;
		case colorChannel::BLUE:
			bl.blueIntensity(blue > 0 ? blue - atenuation : blue < 0 ? blue + atenuation : blue);
			break;
		case colorChannel::ALPHA:
			bl.alphaIntensity(alpha > 0 ? alpha - atenuation : alpha < 0 ? alpha + atenuation : alpha);
			break;
		case colorChannel::ALL:
			bl.redIntensity(red > 0 ? red - atenuation : red < 0 ? red + atenuation : red);
			bl.greenIntensity(green > 0 ? green - atenuation : green < 0 ? green + atenuation : green);
			bl.blueIntensity(blue > 0 ? blue - atenuation : blue < 0 ? blue + atenuation : blue);
			bl.alphaIntensity(alpha > 0 ? alpha - atenuation : alpha < 0 ? alpha + atenuation : alpha);
			break;
		case colorChannel::RGB:
			bl.redIntensity(red > 0 ? red - atenuation : red < 0 ? red + atenuation : red);
			bl.greenIntensity(green > 0 ? green - atenuation : green < 0 ? green + atenuation : green);
			bl.blueIntensity(blue > 0 ? blue - atenuation : blue < 0 ? blue + atenuation : blue);
			break;
		default:
			logger::errorLog("Unsupported color channel");
			break;

		}

		return bl;
	
	}

	void blockLight::fromRGBA(const basicUVec4& rgbaI, const basicVec4& rgbaC) {

		intensityBits_ =
			(static_cast<uint16_t>(rgbaI.x & 0xF) << 12) |
			(static_cast<uint16_t>(rgbaI.y & 0xF) << 8) |
			(static_cast<uint16_t>(rgbaI.z & 0xF) << 4) |
			(static_cast<uint16_t>(rgbaI.w & 0xF)
		);

		value_ = rgbaC;
	
	}

	void blockLight::intensity(lightIntensity intensity, colorChannel channel) {
	
		switch (channel) {

			case colorChannel::RED:
				redIntensity(intensity);
				break;
			case colorChannel::GREEN:
				greenIntensity(intensity);
				break;
			case colorChannel::BLUE:
				blueIntensity(intensity);
				break;
			case colorChannel::ALPHA:
				alphaIntensity(intensity);
				break;
			case colorChannel::ALL:
				redIntensity(intensity);
				greenIntensity(intensity);
				blueIntensity(intensity);
				alphaIntensity(intensity);
				break;
			case colorChannel::RGB:
				redIntensity(intensity);
				greenIntensity(intensity);
				blueIntensity(intensity);
				break;
			default:
				logger::errorLog("Unsupported color channel");
				break;

		}
	
	}

	void blockLight::value(lightValue value, colorChannel channel) {

		switch (channel) {

			case colorChannel::RED:
				redValue(value);
				break;
			case colorChannel::GREEN:
				greenValue(value);
				break;
			case colorChannel::BLUE:
				blueValue(value);
				break;
			case colorChannel::ALPHA:
				alphaValue(value);
				break;
			case colorChannel::ALL:
				redValue(value);
				greenValue(value);
				blueValue(value);
				alphaValue(value);
				break;
			case colorChannel::RGB:
				redValue(value);
				greenValue(value);
				blueValue(value);
				break;
			default:
				logger::errorLog("Unsupported color channel");
				break;

		}

	}

	void blockLight::intensityAndValue(lightIntensity intensity, lightValue value, colorChannel channel) {

		switch (channel) {

		case colorChannel::RED:
			redIntensity(intensity);
			redValue(value);
			break;
		case colorChannel::GREEN:
			greenIntensity(intensity);
			greenValue(value);
			break;
		case colorChannel::BLUE:
			blueIntensity(intensity);
			blueValue(value);
			break;
		case colorChannel::ALPHA:
			alphaIntensity(intensity);
			alphaValue(value);
			break;
		case colorChannel::ALL:
			redIntensity(intensity);
			greenIntensity(intensity);
			blueIntensity(intensity);
			alphaIntensity(intensity);
			redValue(value);
			greenValue(value);
			blueValue(value);
			alphaValue(value);
			break;
		case colorChannel::RGB:
			redIntensity(intensity);
			greenIntensity(intensity);
			blueIntensity(intensity);
			redValue(value);
			greenValue(value);
			blueValue(value);
			break;
		default:
			logger::errorLog("Unsupported color channel");
			break;

		}

	}

	void blockLight::copy(const blockLight& source, colorChannel channel) {
	
		switch (channel) {

		case colorChannel::RED:
			redIntensity(source.redIntensity());
			redValue(source.redValue());
			break;
		case colorChannel::GREEN:
			greenIntensity(source.greenIntensity());
			greenValue(source.greenValue());
			break;
		case colorChannel::BLUE:
			blueIntensity(source.blueIntensity());
			blueValue(source.blueValue());
			break;
		case colorChannel::ALPHA:
			alphaIntensity(source.alphaIntensity());
			alphaValue(source.alphaValue());
			break;
		case colorChannel::ALL:
			redIntensity(source.redIntensity());
			greenIntensity(source.greenIntensity());
			blueIntensity(source.blueIntensity());
			alphaIntensity(source.alphaIntensity());
			redValue(source.redValue());
			greenValue(source.greenValue());
			blueValue(source.blueValue());
			alphaValue(source.alphaValue());
			break;
		case colorChannel::RGB:
			redIntensity(source.redIntensity());
			greenIntensity(source.greenIntensity());
			blueIntensity(source.blueIntensity());
			redValue(source.redValue());
			greenValue(source.greenValue());
			blueValue(source.blueValue());
			break;
		default:
			logger::errorLog("Unsupported color channel");
			break;

		}
	
	}

}
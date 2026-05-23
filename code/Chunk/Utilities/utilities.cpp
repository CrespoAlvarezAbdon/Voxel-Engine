#include "utilities.hpp"

#include <Graphics/Lighting/Lights/PointLight/pointLight.h>
#include <Utilities/Logger/logger.h>

namespace VoxelEng {

    namespace chunkUtility {
    
        void setBlockLight(const ivec3& inChunkPos, const varRef& emittedLight,
            colorChannel channel, lightIntensity& intensity, lightValue& value) {

            varBase::varType blockLightType = emittedLight.isNull() ? varBase::varType::NOTYPE : emittedLight.getVarType();
            switch (blockLightType) {

                case varBase::varType::NOTYPE:
                    intensity = 0;
                    value = 0;
                    break;

                case varBase::varType::POINTLIGHT:
                {
                    const pointLight& light = *emittedLight.pointer<pointLight>();
                    intensity = light.intensity(channel);
                    value = light.ambient(channel);
                    break;
                }
                default:
                    logger::errorLog("Unsupported block light type " + static_cast<int>(blockLightType));
                    break;

            }

        }
    
    }

}
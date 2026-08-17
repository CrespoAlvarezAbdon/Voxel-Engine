#include "chunkVertexBuffer.h"

#include <stdexcept>
#include "Utilities/Logger/logger.h"

namespace VoxelEng {

    const chunkVertexBufferZone& chunkVertexBuffer::bufferZone(const ivec3& chunkPos, geometryType chunkGeometryType) {

        if (chunkGeometryType == geometryType::NONE)
            throw std::runtime_error("Invalid chunk geometry type NONE");

        bool isTranslucidGeometry = chunkGeometryType == geometryType::TRANSLUCENT;
        std::unordered_map<ivec3, chunkVertexBufferZone>& bufferZones =
            isTranslucidGeometry ? chunkTranslucidVertexBufferZones_ : chunkVertexBufferZones_;

        if (bufferZones.contains(chunkPos))
            return bufferZones[chunkPos];
        else
            throw std::runtime_error("There is no chunk position " + std::to_string(chunkPos)
                + " with an associated buffer zone for " + (isTranslucidGeometry ? "translucid" : "opaque/transparent") + " geometry");

    }

    void chunkVertexBuffer::pushDynamicData(const ivec3& chunkPos, const chunkRenderingData& chunkRenderData, geometryType chunkGeometryType) {

        if (chunkGeometryType == geometryType::NONE)
            throw std::runtime_error("Invalid chunk geometry type NONE");

        bool isTranslucidGeometry = chunkGeometryType == geometryType::TRANSLUCENT;
        const void* data = isTranslucidGeometry ? chunkRenderData.translucentVertices.data() : chunkRenderData.vertices.data();
        long long size = (isTranslucidGeometry ? chunkRenderData.translucentVertices.size() : chunkRenderData.vertices.size()) * sizeof(vertex);
        const chunkExtraRenderingData& extraRenderData = chunkRenderData.extraRenderingData;

        if (size <= 0)
            throw std::runtime_error("Size of the data to push cannot be equal to or lower than 0");

        if (data == nullptr)
            throw std::runtime_error("The provided pointer for the data to be pushed cannot be null");

        std::unordered_map<ivec3, chunkVertexBufferZone>& bufferZones =
            isTranslucidGeometry ? chunkTranslucidVertexBufferZones_ : chunkVertexBufferZones_;

        auto itPreexistingZone = bufferZones.find(chunkPos);
        if (itPreexistingZone == bufferZones.end()) { 

            // There is no buffer zone previously assigned to this chunk.

            auto itFit = freedZonesBySize_.lower_bound({ 0, size }); // Find a 'freedzone' with 'freedzone.size' >= 'size'
            if (itFit == freedZonesBySize_.end()) {

                // Case where there is no freed zone with size greater than or equal to the required by this chunk's vertex data.

                if (lastPushedBytePos_ + size < maxSize_) {

                    bufferZones[chunkPos] = { lastPushedBytePos_ , size, extraRenderData };
                    glBufferSubData(GL_ARRAY_BUFFER, lastPushedBytePos_, size, data);
                    lastPushedBytePos_ += size;

                }
                else
                    throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                        "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + size));

            }
            else {

                // Case where there is a suitable freed zone available.

                if (itFit->size == size) {

                    bufferZones[chunkPos] = { itFit->startPos , size, extraRenderData };
                    glBufferSubData(GL_ARRAY_BUFFER, itFit->startPos, size, data);

                    freedZones_.erase(itFit);
                    freedZonesBySize_.erase(itFit);

                }
                else { // itFit->size > size

                    bufferZones[chunkPos] = { itFit->startPos , size, extraRenderData };
                    glBufferSubData(GL_ARRAY_BUFFER, itFit->startPos, size, data);

                    freedZonesBySize::iterator itSize = freedZonesBySize_.insert(
                        { itFit->startPos + size, itFit->size - size }).first;
                    auto itDebug = freedZones_.insert(itSize);

                    freedZones_.erase(itFit);
                    freedZonesBySize_.erase(itFit);

                }

            }

        }
        else { 

            // There is already an entry of the specified geometry type of the given chunk position.

            chunkVertexBufferZone& preexistingZone = (itPreexistingZone->second);
            if (size < preexistingZone.size) {
            
                // Create free zone with the unused space.
                freedZonesBySize::iterator itSize = freedZonesBySize_.insert(
                    { preexistingZone.startPos + size, preexistingZone.size - size }).first;
                auto itDebug = freedZones_.insert(itSize);

                // Update buffer zone.
                preexistingZone.size = size;
                preexistingZone.extraRenderingData = extraRenderData;
                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }
            else if (size == preexistingZone.size) {
            
                preexistingZone.extraRenderingData = extraRenderData;
                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }
            else if (size > preexistingZone.size) {
            
                if(preexistingZone.startPos + preexistingZone.size == lastPushedBytePos_) { 
                
                    // If this preexisting zone is the last buffer zone.

                    preexistingZone.size = size;
                    preexistingZone.extraRenderingData = extraRenderData;
                    if (preexistingZone.startPos + preexistingZone.size < maxSize_)
                        lastPushedBytePos_ = preexistingZone.startPos + preexistingZone.size;
                    else
                        throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                            "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + preexistingZone.size));
                
                }
                else {
                
                    // If this preexisting zone is NOT the last buffer zone, try to reallocate to a freed zone or allocate a new buffer zone.
                    
                    auto itFit = freedZonesBySize_.lower_bound({ 0, size }); // Find a 'freedzone' with 'freedzone.size' >= 'size'
                    if (itFit == freedZonesBySize_.end()) {
                    
                        // There is no freed zone with size greater than or equal to the required by this chunk's vertex data.
                        preexistingZone.startPos = lastPushedBytePos_;
                        preexistingZone.size = size;
                        preexistingZone.extraRenderingData = extraRenderData;
                        if (preexistingZone.startPos + preexistingZone.size < maxSize_)
                            lastPushedBytePos_ = preexistingZone.startPos + preexistingZone.size;
                        else
                            throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                                "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + size));
                    
                    }
                    else {

                        // Case where there is a suitable freed zone available.
                    
                        if (itFit->size == size) {

                            preexistingZone.startPos = itFit->startPos;
                            preexistingZone.size = size;
                            preexistingZone.extraRenderingData = extraRenderData;

                            freedZones_.erase(itFit);
                            freedZonesBySize_.erase(itFit);

                        }
                        else { // itFit->size > size 

                            preexistingZone.startPos = itFit->startPos;
                            preexistingZone.size = size;
                            preexistingZone.extraRenderingData = extraRenderData;

                            freedZonesBySize::iterator itSize = freedZonesBySize_.insert({ itFit->startPos + size, itFit->size - size }).first;
                            auto itDebug = freedZones_.insert(itSize);

                            freedZones_.erase(itFit);
                            freedZonesBySize_.erase(itFit);

                        }
                    
                    }
                
                }

                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }

        }

    }

    void chunkVertexBuffer::freeDynamicData(const ivec3& chunkPos, geometryType chunkGeometryType) {

        if (chunkGeometryType == geometryType::NONE)
            throw std::runtime_error("Invalid chunk geometry type NONE");

        std::unordered_map<ivec3, chunkVertexBufferZone>& bufferZones =
            chunkGeometryType == geometryType::TRANSLUCENT ? chunkTranslucidVertexBufferZones_ : chunkVertexBufferZones_;

        if (bufferZones.contains(chunkPos)) {

            freedZonesBySize::iterator itSize = freedZonesBySize_.insert(bufferZones[chunkPos]).first;
            freedZones::iterator it = freedZones_.insert(itSize).first;
            bufferZones.erase(chunkPos);

            // Merge with previous freed zone if possible to avoid memory fragmentation.
            if (it != freedZones_.begin()) {

                freedZones::iterator itPrevious = std::prev(it);
                if (itPrevious->operator->()->startPos + itPrevious->operator->()->size == it->operator->()->startPos) {

                    chunkVertexBufferZone mergedZone = 
                        { itPrevious->operator->()->startPos, itPrevious->operator->()->size + it->operator->()->size };
      
                    freedZonesBySize_.erase(*it);
                    freedZonesBySize_.erase(*itPrevious);
                    freedZones_.erase(it);
                    freedZones_.erase(itPrevious);

                    itSize = freedZonesBySize_.insert(mergedZone).first;
                    it = freedZones_.insert(itSize).first;

                }

            }

            // Merge with next freed zone if possible to avoid memory fragmentation.
            freedZones::iterator itNext = std::next(it);
            if (itNext != freedZones_.end()) {

                if (it->operator->()->startPos + it->operator->()->size == itNext->operator->()->startPos) {

                    chunkVertexBufferZone mergedZone = { it->operator->()->startPos, it->operator->()->size + itNext->operator->()->size };
                    
                    freedZonesBySize_.erase(*it);
                    freedZonesBySize_.erase(*itNext);
                    freedZones_.erase(it);
                    freedZones_.erase(itNext);

                    itSize = freedZonesBySize_.insert(mergedZone).first;
                    freedZones_.insert(itSize).first;

                }

            }

        }

    }

}
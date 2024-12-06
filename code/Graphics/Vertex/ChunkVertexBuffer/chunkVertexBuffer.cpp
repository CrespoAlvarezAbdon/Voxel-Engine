#include "chunkVertexBuffer.h"

#include <stdexcept>

#include "Utilities/Logger/logger.h"

namespace VoxelEng {

    const chunkVertexBufferZone& chunkVertexBuffer::bufferZone(const vec3& chunkPos) {

        if (chunkVertexBufferZones_.contains(chunkPos))
            return chunkVertexBufferZones_.at(chunkPos);
        else
            throw std::runtime_error("There is no chunk with chunk position " + std::to_string(chunkPos)
                + " with an associated buffer zone");

    }

    void chunkVertexBuffer::pushDynamicData(const vec3& chunkPos, const void* data, long long size) {

        if (size <= 0)
            throw std::runtime_error("Size of the data to push cannot be equal to or lower than 0");

        if (data == nullptr)
            throw std::runtime_error("The provided pointer for the data to be pushed cannot be null");

        auto itPreexistingZone = chunkVertexBufferZones_.find(chunkPos);
        if (itPreexistingZone == chunkVertexBufferZones_.end()) {

            // Use freed zone if possible.
            if (freedZonesBySize_.begin() == freedZonesBySize_.end() || size > freedZonesBySize_.begin()->size) {

                // There is no freed zone with size greater than or equal to the required by this chunk's vertex data.
                if (lastPushedBytePos_ + size < maxSize_) {

                    chunkVertexBufferZones_[chunkPos] = { lastPushedBytePos_ , size };
                    glBufferSubData(GL_ARRAY_BUFFER, lastPushedBytePos_, size, data);
                    lastPushedBytePos_ += size;

                }
                else
                    throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                        "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + size));

            }
            else {

                if (size == freedZonesBySize_.begin()->size) {

                    chunkVertexBufferZones_[chunkPos] = { freedZonesBySize_.begin()->startPos , size };
                    glBufferSubData(GL_ARRAY_BUFFER, freedZonesBySize_.begin()->startPos, size, data);

                    freedZonesBySize_.erase(freedZonesBySize_.begin());
                    freedZones_.erase(freedZonesBySize_.begin());

                }
                else { // size < freedZonesBySize_.begin()->size

                    chunkVertexBufferZones_[chunkPos] = { freedZonesBySize_.begin()->startPos , size };
                    glBufferSubData(GL_ARRAY_BUFFER, freedZonesBySize_.begin()->startPos, size, data);

                    freedZonesBySize::iterator itSize = freedZonesBySize_.insert({ freedZonesBySize_.begin()->startPos + size, freedZonesBySize_.begin()->size - size  }).first;
                    auto itDebug = freedZones_.insert(itSize);

                    if (itDebug.first.operator*().operator*().size < 0)
                        throw std::runtime_error("Size cannot be negative");

                    freedZonesBySize_.erase(freedZonesBySize_.begin());
                    freedZones_.erase(freedZonesBySize_.begin());

                }

            }

        }
        else {

            chunkVertexBufferZone& preexistingZone = (itPreexistingZone->second);
            if (size < preexistingZone.size) {
            
                // Create free zone with the unused space.
                freedZonesBySize::iterator itSize = freedZonesBySize_.insert({ preexistingZone.startPos + size, preexistingZone.size - size }).first;
                auto itDebug = freedZones_.insert(itSize);

                if (itDebug.first.operator*().operator*().size < 0)
                    throw std::runtime_error("Size cannot be negative");

                // Update buffer zone.
                preexistingZone.size = size;

                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }
            else if (size == preexistingZone.size) {
            
                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }
            else if (size > preexistingZone.size) {
            
                if(preexistingZone.startPos + preexistingZone.size == lastPushedBytePos_) {
                
                    preexistingZone.size = size;
                    if (preexistingZone.startPos + preexistingZone.size < maxSize_)
                        lastPushedBytePos_ = preexistingZone.startPos + preexistingZone.size;
                    else
                        throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                            "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + preexistingZone.size));
                
                }
                else {
                
                    // Use freed zone if possible.
                    if (freedZonesBySize_.begin() == freedZonesBySize_.end() || size > freedZonesBySize_.begin()->size) {
                    
                        // There is no freed zone with size greater than or equal to the required by this chunk's vertex data.
                        preexistingZone.startPos = lastPushedBytePos_;
                        preexistingZone.size = size;
                        if (preexistingZone.startPos + preexistingZone.size < maxSize_)
                            lastPushedBytePos_ = preexistingZone.startPos + preexistingZone.size;
                        else
                            throw std::runtime_error("Cannot push data beyond VBO max size " + std::to_string(maxSize_) +
                                "\nData was attempted to be pushed to position " + std::to_string(lastPushedBytePos_ + size));
                    
                    }
                    else {
                    
                        if (size == freedZonesBySize_.begin()->size) {

                            preexistingZone.startPos = freedZonesBySize_.begin()->startPos;
                            preexistingZone.size = size;

                            freedZonesBySize_.erase(freedZonesBySize_.begin());
                            freedZones_.erase(freedZonesBySize_.begin());

                        }
                        else { // size < freedZonesBySize_.begin()->size

                            preexistingZone.startPos = freedZonesBySize_.begin()->startPos;
                            preexistingZone.size = size;

                            freedZonesBySize::iterator itSize = freedZonesBySize_.insert({ freedZonesBySize_.begin()->startPos + size, freedZonesBySize_.begin()->size - size }).first;
                            auto itDebug = freedZones_.insert(itSize);

                            if (itDebug.first.operator*().operator*().size < 0)
                                throw std::runtime_error("Size cannot be negative");

                            freedZonesBySize_.erase(freedZonesBySize_.begin());
                            freedZones_.erase(freedZonesBySize_.begin());

                        }
                    
                    }
                
                }

                glBufferSubData(GL_ARRAY_BUFFER, preexistingZone.startPos, preexistingZone.size, data);
            
            }

        }
    }

    void chunkVertexBuffer::freeDynamicData(const vec3& chunkPos) {

        if (chunkVertexBufferZones_.contains(chunkPos)) {

            // ME HA SALIDO QUE FREEDZONESBYSIZE Y FREEDZONES TIENEN DISTINTOS TAMAÑOS. ASI QUE EN ALGÚN MOMENTO SOLO SE ESTÁN INSERTANDO O BORRANDO EN UNO SOLO DE LOS DOS.
            freedZonesBySize::iterator itSize = freedZonesBySize_.insert(chunkVertexBufferZones_[chunkPos]).first;
            freedZones::iterator it = freedZones_.insert(itSize).first;
            if (it.operator*().operator*().size < 0)
                throw std::runtime_error("Size cannot be negative");
            chunkVertexBufferZones_.erase(chunkPos);

            logger::debugLog("flag 1");

            // Merge free zones if possible to avoid memory fragmentation.

            if (it != freedZones_.begin()) {

                freedZones::iterator itPrevious = --it;
                it++;

                if (itPrevious->operator->()->startPos + itPrevious->operator->()->size == it->operator->()->startPos) {

                    chunkVertexBufferZone mergedZone = { itPrevious->operator->()->startPos, itPrevious->operator->()->size + it->operator->()->size };
                    
                    if (mergedZone.size < 0)
                        throw std::runtime_error("Size cannot be negative");
                    
                    freedZonesBySize_.erase(*it);
                    freedZonesBySize_.erase(*itPrevious);
                    freedZones_.erase(it);
                    freedZones_.erase(itPrevious);

                    itSize = freedZonesBySize_.insert(mergedZone).first;
                    it = freedZones_.insert(itSize).first;

                    if (it.operator*().operator*().size < 0)
                        throw std::runtime_error("Size cannot be negative");

                    logger::debugLog("flag 2");

                }

            }

            freedZones::iterator itNext = ++it;
            it--;

            if (itNext != freedZones_.end()) {

                if (it->operator->()->startPos + it->operator->()->size == itNext->operator->()->startPos) {

                    chunkVertexBufferZone mergedZone = { it->operator->()->startPos, it->operator->()->size + itNext->operator->()->size };
                    
                    if (mergedZone.size < 0)
                        throw std::runtime_error("Size cannot be negative");

                    logger::debugLog("flag 3");
                    
                    freedZonesBySize_.erase(*it);
                    freedZonesBySize_.erase(*itNext);
                    freedZones_.erase(it);
                    freedZones_.erase(itNext);

                    itSize = freedZonesBySize_.insert(mergedZone).first;
                    freedZones_.insert(itSize).first;

                    if (it.operator*().operator*().size < 0)
                        throw std::runtime_error("Size cannot be negative");

                }

            }

        }

    }

}
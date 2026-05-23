#include "chunkEnums.hpp"

namespace VoxelEng {

	bool isPriority(chunkJobType type) {
	
        bool itIsPriority = false;

        switch (type) {

            case chunkJobType::PRIORITYREMESH:
            case chunkJobType::PRIORITYREMESH_ADDEDLIGHT:
            case chunkJobType::PRIORITYREMESH_REMOVEDLIGHT:
                itIsPriority = true;
                break;
            default:
                break;

        }

        return itIsPriority;
	
	}

}
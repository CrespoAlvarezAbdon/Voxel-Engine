#include "gui.h"



namespace VoxelEng {

	// GUIManager.
	glm::mat4 GUIManager::projectionMatrix_;
	std::unordered_map<unsigned int, GUIElement*> GUIManager::activeGUIelements_,
												  GUIManager::inactiveGUIelements_;

	void GUIManager::initialize(int screenWidth, int screenHeight) {

		projectionMatrix_ = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);

	}

}
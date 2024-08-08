#include "UBOs.h"
namespace VoxelEng {

	template <typename T>
	requires std::default_initializable<T>
	bool UBO<T>::initialised_ = false;

	template <typename T>
	requires std::default_initializable<T>
	std::string UBO<T>::typeName_ = "";

}
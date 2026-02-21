/**
* @file chunk.h
* @version 1.0
* @date 20/04/2023
* @author Abdon Crespo Alvarez
* @title Chunk.
* @brief Contains the chunk and chunkManager classes as well as some
* auxiliary data structures and types used with them.
*/
#ifndef _VOXELENG_CHUNK_
#define _VOXELENG_CHUNK_

#include <atomic>
#include <barrier>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <initializer_list>
#include <list>
#include <thread>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <time.h>
#include <functional>
#include <atomicRecyclingPool.h>
#include <definitions.h>
#include <event.h>
#include <listener.h>
#include <threadPool.h>
#include <palette.h>
#include <vec.h>
#include <utilities.h>
#include <Block/block.h>
#include <Block/blockState.hpp>
#include <Block/lightData.hpp>
#include <Block/Properties/blockProperty.hpp>
#include <Chunk/blockLightMod.h>
#include <Chunk/chunkBlockData.hpp>
#include <Chunk/chunkDefinitions.h>
#include <Chunk/chunkEnums.hpp>
#include <Chunk/chunkRenderingData.hpp>
#include <Chunk/chunkVBOop.hpp>
#include <Chunk/neighborsInfo.h>
#include <Graphics/Lighting/definitions.hpp>
#include <Graphics/Textures/texture.h>
#include <Graphics/Shaders/shader.h>
#include <Graphics/Models/model.h>
#include <Graphics/Vertex/vertex.h>
#include <Graphics/Vertex/VertexBufferLayout/vertexBufferLayout.h>
#include <Registry/RegistryInsOrdered/registryInsOrdered.h>
#include <Utilities/BlockViewDir/blockViewDir.hpp>
#include <Utilities/Padded3DArray/Padded3DArray.hpp>

#if GRAPHICS_API == OPENGL

#include <GL/glew.h>
#include <glm.hpp>
#include <hash.hpp>

#endif

namespace VoxelEng {

	/////////////////////////
	//Forward declarations.//
	/////////////////////////

	class chunkManager;
	class chunkVertexBuffer;


	////////////
	//Classes.//
	////////////

	/**
	* @brief Represents a section of the voxel world, with its blocks, mesh, position and other infomation.
	* Chunks that are marked as 'dirty' or 'changed' will have their mesh regenerated.
	*/
	class chunk {

	public:

		bool poraqui = false;
		bool poraqui2 = false;
		bool poraqui3 = false;
		bool poraqui4 = false;
		bool poraqui5 = false;
		bool poraqui6 = false;
		bool poraqui7 = false;
		bool poraqui8 = false;
		bool poraqui9 = false;
		bool poraqui10 = false;
		bool poraqui11 = false;
		bool poraqui12 = false;
		bool poraqui13 = false;
		bool poraqui14 = false;
		chunkLoadStatus poraqui15 = chunkLoadStatus::NOTLOADED;
		bool poraqui16 = false;
		bool poraqui17 = false;
		bool poraqui18 = false;
		bool poraqui19 = false;
		bool poraqui20 = false;
		bool poraqui21 = false;
		bool poraqui22 = false;
		bool poraqui23 = false;
		bool poraqui24 = false;
		bool poraqui25 = false;
		bool poraqui26 = false;
		bool poraqui27 = false;
		bool poraqui28 = false;
		bool poraqui29 = false;
		bool wasMadeEmpty = false;

		// Initialisers.

		/**
		* @brief Initialise static members of the chunk class.
		* Allocate any resources that are needed on initialisation.
		*/
		static void init();


		// Constructors.

		/**
		* @brief Default constructor. Sets all attributes to default values
		* and does not generate anything in the chunk.
		*/
		chunk();

		/**
		* @brief Construct an empty dirty chunk.
		* If 'empty' is true then the chunk block data will be filled with null block IDs (block ID 0).
		* If 'empty' is false then the chunk block data will be filled with the selected world generator.
		*/
		chunk(bool empty, const ivec3& chunkPos = vec3Zero);

		/**
		* @brief Copy constructor.
		*/
		chunk(chunk& source);


		// Observers.

		/**
		* @brief Returns true if the static members of this class are initialised
		* or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the pointer to the first element of the chunk's block array.
		*/
		const Padded3DArray<unsigned short>& blocks() const;

		/**
		* @brief Get the block data of this chunk.
		* @returns The block data of this chunk (local IDs, whether they are opaque or not, and their block light values).
		*/
		chunkBlockData blockData();

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		template <typename T>
		requires std::is_base_of_v<blockProperty, T> && std::is_default_constructible_v<T>
		const blockState& get(GLbyte x, GLbyte y, GLbyte z);

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		*/
		template <typename T>
		requires std::is_base_of_v<blockProperty, T> && std::is_default_constructible_v<T>
		const blockState& get(const ivec3& inChunkPos);

		/**
		* @brief Get the light data at the specified chunk-local coordinates.
		*/
		lightData getLight(GLbyte x, GLbyte y, GLbyte z);

		/**
		* @brief Get the light data at the specified chunk-local coordinates.
		*/
		lightData getLight(const ivec3& inChunkPos);

		/**
		* @brief Get the block at the specified chunk-local coordinates.
		* @param firstIndex First dimension coordinate.
		* @param secondIndex Second dimension coordinate.
		* @param neighbor Direction to follow to reach the neighbor chunk.
		*/
		const block& getNeighborBlock(GLbyte firstIndex, GLbyte secondIndex, blockViewDir neighbor);

		/**
		* @brief Get chunk's x axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte x() const;

		/**
		* @brief Get chunk's y axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte y() const;

		/**
		* @brief Get chunk's z axis coordinate (chunk-grid coordinate system).
		*/
		GLbyte z() const;

		/**
		* @brief Get chunk's coordinate (chunk-grid coordinate systems).
		*/
		const ivec3& chunkPos() const;

		/**
		* @brief Get this chunk's global position.
		*/
		const vec3& pos() const;

		/**
		* @brief Get this chunk's rendering data object.
		*/
		const chunkRenderingData& renderingData() const;

		/**
		* @brief Returns true if this chunk has been modified
		* since being generated or loaded from disk or
		* false otherwise.
		*/
		bool modified() const;

		/**
		* @brief Get whether this chunk's mesh needs to be regenerated or not.
		* @returnsWhether this chunk's mesh needs to be regenerated (true) or not (false).
		*/
		bool needsRemesh() const;

		/**
		* @brief Get whether this chunk was loaded from disk or not.
		* @returnsWhether this chunk was loaded from disk (true) or not (false).
		*/
		bool loadedFromDisk() const;

		/**
		* @brief Returns the chunk's status.
		*/
		chunkLoadStatus loadStatus() const;

		/**
		* @brief Get the number of non-null opaque blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned short nOpaqueBlocks() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +X edge.
		*/
		unsigned short nOpaqueBlocksPlusX() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -X edge.
		*/
		unsigned short nOpaqueBlocksMinusX() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +Y edge.
		*/
		unsigned short nOpaqueBlocksPlusY() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -Y edge.
		*/
		unsigned short nOpaqueBlocksMinusY() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's +Z edge.
		*/
		unsigned short nOpaqueBlocksPlusZ() const;

		/**
		* @brief Returns the number of non-null opaque blocks in the chunk's -Z edge.
		*/
		unsigned short nOpaqueBlocksMinusZ() const;

		/**
		* @brief Get the total number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		unsigned short nTotalBlocks() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +X edge.
		*/
		unsigned short nTotalBlocksPlusX() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -X edge.
		*/
		unsigned short nTotalBlocksMinusX() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +Y edge.
		*/
		unsigned short nTotalBlocksPlusY() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -Y edge.
		*/
		unsigned short nTotalBlocksMinusY() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's +Z edge.
		*/
		unsigned short nTotalBlocksPlusZ() const;

		/**
		* @brief Returns the total number of non-null blocks in the chunk's -Z edge.
		*/
		unsigned short nTotalBlocksMinusZ() const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		*/
		bool isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		*/
		bool isEmptyBlock(const ivec3& inChunkPos) const;

		/**
		* @brief Returns true if the specified block in in-chunk coordinates is
		* the empty block or false otherwise.
		* @param firstIndex. First dimension coordinate of the block.
		* @param secondIndex. Second dimension coordinate of the block.
		* @param neighbor. The direction to follow to reach the neighbor.
		*/
		bool isEmptyNeighborBlock(unsigned int firstIndex, unsigned int secondIndex, blockViewDir neighbor);

		/**
		* @brief Returns the chunk's palette that maps the local block IDs with the global block IDs.
		*/
		const palette<unsigned short, unsigned int>& getPalette() const;

		/**
		* @brief Returns the chunk's palette count that contains the number of global IDs mapped to
		* local IDs for this chunk.
		*/
		const std::unordered_map<unsigned short, unsigned short>& getPaletteCount() const;

		/**
		* @brief Get the free local IDs available for this chunk's block palette.
		* @returns The free local IDs available for this chunk's block palette.
		*/
		const std::unordered_set<unsigned short>& getFreeLocalIDs() const;

		/**
		* @brief Get the positions of all the point lights affecting this chunk.
		* @returns The positions of all the point lights affecting this chunk.
		*/
		const std::unordered_set<ivec3>& getFloodPointLightPositions() const;

		/**
		* @brief Get the block light color at the given chunk relative pos.
		*/
		const basicVec4& getBlockLightColor(const ivec3& chunkRelPos);

		/**
		* @brief Get the block light level at the given chunk relative pos.
		*/
		char getBlockLightLevel(const ivec3& chunkRelPos);

		/**
		* @brief Get whether this chunk has owners or not.
		* @returns Whether this chunks has owners (true) or not (false).
		*/
		bool isOwned() const;

		/**
		* @brief Get whether the chunk should unload inmediately after being released of its last owner or not.
		* @returns Whether the chunk should unload inmediately after being released of its last owner (true) or not (false).
		*/
		bool unloadWhenNoOwners() const;

		/**
		* @brief Get the chunks currently owned by this chunk.
		* WARNING. To be called ONLY inside this chunk's loading stage.
		* @returns The chunks currently owned by this chunk.
		*/
		const std::unordered_map<ivec3, chunk*>& ownedChunks() const;


		// Modifiers.

		/**
		* @brief Get the chunk's block array.
		* @returns The chunk's block array.
		*/
		Padded3DArray<unsigned short>& blocks();

		/**
		* @brief Returns the chunk's palette that maps the local block IDs with the global block IDs.
		*/
		palette<unsigned short, unsigned int>& getPalette();

		/**
		* @brief Returns the chunk's palette count that contains the number of global IDs mapped to
		* local IDs for this chunk.
		*/
		std::unordered_map<unsigned short, unsigned short>& getPaletteCount();

		/**
		* @brief Get the free local IDs available for this chunk's block palette.
		* @returns The free local IDs available for this chunk's block palette.
		*/
		std::unordered_set<unsigned short>& getFreeLocalIDs();

		/**
		* @brief Get the positions of all the point lights affecting this chunk.
		* @returns The positions of all the point lights affecting this chunk.
		*/
		std::unordered_set<ivec3>& getFloodPointLightPositions();

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise.
		* WARNING. For world generators that use this method: 'modification' must be set to false.
		*/
		const block& setBlock(sbyte x, sbyte y, sbyte z, const block& block, bool modification = true);

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise.
		* WARNING. For world generators that use this method: 'modification' must be set to false.
		*/
		const block& setBlock(const ivec3& chunkRelPos, const block& block, bool modification = true);

		/**
		* @brief Sets the value of a block within the chunk.
		* Returns the replaced block.
		* 'modification' tells if the call to this method is NOT part of the
		* chunk's generation process or otherwise. For world generators that use this method, it must be set to false.
		*/
		const block& setBlock(unsigned int linearIndex, const block& block, bool modification = true);

		/**
		* Apply the differences in block light when a block is replaced with another one.
		* @param oldB The replaced block.
		* @param b The block that replaces.
		* @param pos The chunk-local-grid coordinates of the blocks.
		*/
		void replaceBlockLight(const block& oldB, const block& b, byte x, byte y, byte z);

		/**
		* Apply the differences in block light when a block is replaced with another one.
		* @param oldB The replaced block.
		* @param b The block that replaces.
		* @param pos The chunk-local-grid coordinates of the blocks.
		*/
		void replaceBlockLight(const block& oldB, const block& b, const ivec3& pos);

		/**
		* Apply the differences in block light when a block is replaced with another one.
		* @param oldB The replaced block.
		* @param b The block that replaces.
		* @param pos The chunk-local-grid coordinates of the blocks.
		*/
		void setBlockLight(const block& b, byte x, byte y, byte z);

		/**
		* Apply the differences in block light when a block is replaced with another one.
		* @param oldB The replaced block.
		* @param b The block that replaces.
		* @param pos The chunk-local-grid coordinates of the blocks.
		*/
		void setBlockLight(const block& b, const ivec3& pos);

		/**
		* @brief Set the chunk's chunk position.
		*/
		void chunkPos(const ivec3& newChunkPos);

		/**
		* @brief Returns the chunk's rendering data object.
		*/
		chunkRenderingData& renderingData();

		/**
		* @brief Locks the blocks' data mutex.
		*/
		std::shared_mutex& blocksDataMutex();

		/**
		* @brief Locks the rendering data mutex for shared ownership.
		*/
		void lockSharedRenderingDataMutex();

		/**
		* @brief Returns true if the rendering data mutex could be locked or false otherwise.
		*/
		bool tryLockRenderingDataMutex();

		/**
		* @brief Unlocks the rendering data mutex for shared ownership.
		* WARNING. Said lock must be unlock by the same thread that locked it previously.
		*/
		void unlockSharedRenderingDataMutex();

		/**
		* @brief Unlocks the rendering data mutex.
		* WARNING. Said lock must be unlock by the same thread that locked it previously.
		*/
		void unlockRenderingDataMutex();

		/**
		* @brief Set wheter this chunk's terrain has been modified and it's mesh needs to be regenerated (true) or not (false).
		* @param newValue Value to set to this flag.
		*/
		void needsRemesh(bool newValue);
		
		/**
		* @brief Set whether this chunk has been loaded from disk (true) or has been generated (false).
		* @param newValue Value to set to this flag.
		*/
		void loadedFromDisk(bool newValue);

		/**
		* @brief Returns true if this chunk has been modified
		* since being generated or loaded from disk or
		* false otherwise.
		*/
		void modified(bool newValue);

		/**
		* @brief Regenerate the chunk's mesh.
		* @param forceRemesh Whether the remesh operation ignores the needsRemesh_ flag (true) or not (false).
		* @returns The chunk's mesh size.
		*/
		std::size_t renewMesh(bool forceRemesh);

		/**
		* @brief Clear all data related to block light in the chunk.
		*/
		void clearBlockLight();

		/**
		* @brief Add the given block light modification to the chunk.
		* @param x Chunk-local-grid X-axis coordinate.
		* @param y Chunk-local-grid Y-axis coordinate.
		* @param z Chunk-local-grid Z-axis coordinate.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void addBlockLight(char x, char y, char z, const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Apply the given block light modification to the chunk.
		* @param inChunkPos Chunk-local-grid coordinates.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void addBlockLight(const ivec3& inChunkPos, const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Apply the given block light modification to the chunk.
		* @param inChunkPos Chunk-local-grid coordinates.
		* @param xOffset Chunk-local-grid X-axis coordinate offset.
		* @param yOffset Chunk-local-grid Y-axis coordinate offset.
		* @param zOffset Chunk-local-grid Z-axis coordinate offset.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void addBlockLight(const ivec3& inChunkPos, char xOffset, char yOffset, char zOffset,
			const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Apply the given block light modification to the chunk.
		* @param x Chunk-local-grid X-axis coordinate.
		* @param y Chunk-local-grid Y-axis coordinate.
		* @param z Chunk-local-grid Z-axis coordinate.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void applyBlockLight(char x, char y, char z, const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Apply the given block light modification to the chunk.
		* @param inChunkPos Chunk-local-grid coordinates.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void applyBlockLight(const ivec3& inChunkPos, const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Apply the given block light modification to the chunk.
		* @param inChunkPos Chunk-local-grid coordinates.
		* @param xOffset Chunk-local-grid X-axis coordinate offset.
		* @param yOffset Chunk-local-grid Y-axis coordinate offset.
		* @param zOffset Chunk-local-grid Z-axis coordinate offset.
		* @param color Block light's color.
		* @param intensity Block light's intensity.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void applyBlockLight(const ivec3& inChunkPos, char xOffset, char yOffset, char zOffset,
			const basicVec4& color, char intensity, bool markToBeRemeshed);

		/**
		* @brief Remove any block light applied to the specified position in the chunk.
		* @param inChunkPos Chunk-local-grid coordinates.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void removeBlockLight(const ivec3& inChunkPos, bool markToBeRemeshed);

		/**
		* @brief Remove any block light applied to the specified position in the chunk.
		* @param x Chunk-local-grid X-axis coordinate.
		* @param y Chunk-local-grid Y-axis coordinate.
		* @param z Chunk-local-grid Z-axis coordinate.
		* @param markToBeRemeshed Whether to set the chunk as in need to be remeshed (true) or not (false).
		*/
		void removeBlockLight(char x, char y, char z, bool markToBeRemeshed);

		/**
		* @brief The chunk's block data will be filled with null blocks, leaving the chunk "empty of blocks".
		*/
		void makeEmpty();

		/**
		* @brief Set the chunk's load status.
		* @param level The chunk load status to set.
		*/
		void loadStatus(chunkLoadStatus level);

		/**
		* @brief Get the mutex guarding the chunk's load status.
		* @returns The mutex guarding the chunk's load status.
		*/
		std::recursive_mutex& loadStatusMutex();

		/**
		* @brief Set the number of non-null opaque blocks (blocks with ID != 0) that exist in the chunk.
		*/
		void nOpaqueBlocks(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +X edge.
		*/
		void nOpaqueBlocksPlusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -X edge.
		*/
		void nOpaqueBlocksMinusX(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +Y edge.
		*/
		void nOpaqueBlocksPlusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -Y edge.
		*/
		void nOpaqueBlocksMinusY(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's +Z edge.
		*/
		void nOpaqueBlocksPlusZ(unsigned short newValue);

		/**
		* @brief Set the number of non-null opaque blocks in the chunk's -Z edge.
		*/
		void nOpaqueBlocksMinusZ(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks (blocks with ID != 0) that exist in the chunk.
		*/
		void nTotalBlocks(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +X edge.
		*/
		void nTotalBlocksPlusX(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -X edge.
		*/
		void nTotalBlocksMinusX(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +Y edge.
		*/
		void nTotalBlocksPlusY(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -Y edge.
		*/
		void nTotalBlocksMinusY(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's +Z edge.
		*/
		void nTotalBlocksPlusZ(unsigned short newValue);

		/**
		* @brief Set the total number of non-null blocks in the chunk's -Z edge.
		*/
		void nTotalBlocksMinusZ(unsigned short newValue);

		/**
		* @brief Executed when the frontier chunk is unloaded.
		* Checks if the neighbors of this chunk become frontier chunks
		* after this one stops being loaded.
		*/
		void onUnloadAsFrontier();

		/**
		* @brief Things to execute just after finishing generation/loading from disk go here.
		*/
		void postGenPass();

		/**
		* @brief Increase chunk owner counter.
		* @param chunkPos Chunk position of the owner.
		*/
		void addOwner(const ivec3& chunkPos);

		/**
		* @brief Decrease chunk owner counter.
		* @param chunkPos Chunk position of the owner.
		*/
		void removeOwner(const ivec3& chunkPos);

		/**
		* @brief Set whether the chunk should unload inmediately after being released of its last owner or not.
		* @param Whether the chunk should unload inmediately after being released of its last owner (true) or not (false).
		*/
		void unloadWhenNoOwners(bool value);

		/**
		* @brief Get and own the chunk in the specified chunk-grid coordinates and its neighbors.
		* @param Whether the coordinates used for keys to accessing the stored owned chunks are relative 
		* to this chunk's chunk-grid-coordinates (true) or not (false).
		*/
		void getAndOwnChunks(bool relativeCoords = true);

		/**
		* @brief Get and own the chunk in the specified chunk-grid coordinates and its nearby chunks specified with the
		* nChunksX, nChunksY and nChunksZ parameters.
		* In consequence, the collection of owned chunks would be a box of chunks with its starting corner at the chunk located at 'chunkPos'.
		* @param nChunksX Number of chunks in X axis to pick starting from the specified chunk.
		* @param nChunksY Number of chunks in Y axis to pick starting from the specified chunk.
		* @param nChunksZ Number of chunks in Z axis to pick starting from the specified chunk.
		*/
		void getAndOwnChunks(int nChunksX, int nChunksY, int nChunksZ, bool relativeCoords = true);

		/**
		* @brief Get and own the chunks that correspond to the specified positions of block light data to set.
		* The coordinates used for keys to accessing the stored owned chunks are relative.
		* @param data Collection of block light data to set that determine the chunks to get and own.
		*/
		void getAndOwnChunks(const lightDataToSet& data);

		/**
		* @brief Disown this and the chunks owned by this chunk.
		* @param ownedChunks Collection of owned chunks.
		*/
		void disownChunks();

		/**
		* @brief Get the mutex of the chunks currently owned by this chunk.
		* @returns The mutex of the chunks currently owned by this chunk.
		*/
		std::recursive_mutex& ownedChunksMutex();

		/**
		* @brief Get the chunks currently owned by this chunk.
		* WARNING. To be called ONLY inside this chunk's loading stage.
		* @returns The chunks currently owned by this chunk.
		*/
		std::unordered_map<ivec3, chunk*>& ownedChunks();


		// Destructors.

		~chunk();


		// Clean up.
		
		/**
		* @brief Clean up any resources allocated for this system.
		*/ 
		static void reset();
		
	private:

		/*
		Attributes.
		*/

		static bool initialised_;
		static const model* blockVertices_;
		static const modelTriangles* blockTriangles_;
		static const modelNormals* blockNormals_;
		
		// Serializable data.
		palette<unsigned short, unsigned int> palette_;
		std::unordered_map<unsigned short, unsigned short> paletteCount_;
		std::unordered_set<unsigned short> freeLocalIDs_;

		// Chunk block data (serializable).
		Padded3DArray<unsigned short> blocksLocalIDs_;
		Padded3DArray<byte> isOpaque_;
		Padded3DArray<basicVec4> blockLightColor_; // Lighting color value in the specific block without light level applied. 4ºth value is alpha.
		Padded3DArray<char> blockLightLevel_; // Lighting value in the specific block. TODO. DELETE SINCE THIS IS ONLY USEFUL FOR DEBUGGING A SINGLE LIGHT.
		std::unordered_set<ivec3> floodPointLightPositions_;

		bool modified_;
		
		std::atomic<short> nOpaqueBlocks_;
		std::atomic<short> nOpaqueBlocksPlusX_;
		std::atomic<short> nOpaqueBlocksMinusX_;
		std::atomic<short> nOpaqueBlocksPlusY_;
		std::atomic<short> nOpaqueBlocksMinusY_;
		std::atomic<short> nOpaqueBlocksPlusZ_;
		std::atomic<short> nOpaqueBlocksMinusZ_;
		std::atomic<short> nTotalBlocks_;
		std::atomic<short> nTotalBlocksPlusX_;
		std::atomic<short> nTotalBlocksMinusX_;
		std::atomic<short> nTotalBlocksPlusY_;
		std::atomic<short> nTotalBlocksMinusY_;
		std::atomic<short> nTotalBlocksPlusZ_;
		std::atomic<short> nTotalBlocksMinusZ_;

		std::atomic<bool> needsRemesh_;
		std::atomic<bool> loadedFromDisk_;

		std::recursive_mutex ownersMutex_;
		std::set<ivec3> owners_;
		bool unloadWhenNoOwners_;

		chunkLoadStatus loadStatus_;
		std::recursive_mutex loadStatusMutex_;

		ivec3 chunkPos_;

		chunkRenderingData renderingData_;
		std::shared_mutex renderingDataMutex_;

		/*
		Used for reading the block data in a chunk.
		All meshing threads only read this data, so they access it
		in shared mode, while any thread that can alter this data
		must access it in exclusive/unique mode.
		*/
		std::shared_mutex blocksMutex_;

		// TO BE USED WHEN THE CHUNK IS BEING LOADED.
		std::recursive_mutex ownedChunksMutex_;
		std::unordered_map<ivec3, chunk*> ownedChunks_;


		/*
		Methods.
		*/
		
		/*
		Returns whether there was a block change (true) or not (false).
		*/
		bool placeNewBlock_(unsigned short& oldLocalID, const block& newBlock);

		basicVec4 getBlockLightAverage_(const basicVec4& blockLightOwn,
			const basicVec3& blockLightCoords1, const basicVec3& blockLightCoords2, const basicVec3& blockLightCords3);

		void getAndOwnChunksWithOffset_(int negOffsetX, int negOffsetY, int negOffsetZ, int offsetX, int offsetY, int offsetZ, 
			bool relativeCoords);

	};

	typedef std::map<ivec3, chunk*> chunksMap;

	inline const Padded3DArray<unsigned short>& chunk::blocks() const {

		return blocksLocalIDs_;

	}

	inline chunkBlockData chunk::blockData() {
	
		return { &blocksLocalIDs_, &isOpaque_, &blockLightColor_, &blockLightLevel_, &floodPointLightPositions_};
	
	}

	inline bool chunk::initialised() {
	
		return initialised_;
	
	}

	template <typename T>
	requires std::is_base_of_v<blockProperty, T> && std::is_default_constructible_v<T>
	const blockState& chunk::get(GLbyte x, GLbyte y, GLbyte z) {

		logger::errorLog("Unimplemented chunk::get for type " + std::to_string(typeid(T).name()));
		return T();

	}

	template <typename T>
	requires std::is_base_of_v<blockProperty, T>&& std::is_default_constructible_v<T>
	inline const blockState& chunk::get(const ivec3& inChunkPos) {

		return get<T>(inChunkPos.x, inChunkPos.y, inChunkPos.z);

	}

	inline lightData chunk::getLight(GLbyte x, GLbyte y, GLbyte z) {
	
		return {blockLightLevel_[x][y][z], blockLightColor_[x][y][z]};
	
	}

	inline lightData chunk::getLight(const ivec3& inChunkPos) {
	
		return getLight(inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}

	inline GLbyte chunk::x() const {

		return chunkPos_.x;

	}

	inline GLbyte chunk::y() const {

		return chunkPos_.y;

	}

	inline GLbyte chunk::z() const {

		return chunkPos_.z;

	}

	inline const ivec3& chunk::chunkPos() const {

		return chunkPos_;

	}

	inline const vec3& chunk::pos() const {

		return vec3{ (float)chunkPos_.x * CHUNK_SIZE, (float)chunkPos_.y * CHUNK_SIZE, (float)chunkPos_.z * CHUNK_SIZE };

	}

	inline const chunkRenderingData& chunk::renderingData() const {

		return renderingData_;

	}

	inline bool chunk::modified() const {

		return modified_;

	}

	inline bool chunk::needsRemesh() const {

		return needsRemesh_;

	}

	inline bool  chunk::loadedFromDisk() const {
	
		return loadedFromDisk_;
	
	}

	inline chunkLoadStatus chunk::loadStatus() const {

		return loadStatus_;

	}

	inline unsigned short chunk::nOpaqueBlocks() const {

		return nOpaqueBlocks_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusX() const {
	
		return nOpaqueBlocksPlusX_;
	
	}

	inline unsigned short chunk::nOpaqueBlocksMinusX() const {

		return nOpaqueBlocksMinusX_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusY() const {

		return nOpaqueBlocksPlusY_;

	}

	inline unsigned short chunk::nOpaqueBlocksMinusY() const {

		return nOpaqueBlocksMinusY_;

	}

	inline unsigned short chunk::nOpaqueBlocksPlusZ() const {

		return nOpaqueBlocksPlusZ_;

	}

	inline unsigned short chunk::nOpaqueBlocksMinusZ() const {

		return nOpaqueBlocksMinusZ_;

	}

	inline unsigned short chunk::nTotalBlocks() const {

		return nTotalBlocks_;

	}

	inline unsigned short chunk::nTotalBlocksPlusX() const {

		return nTotalBlocksPlusX_;

	}

	inline unsigned short chunk::nTotalBlocksMinusX() const {

		return nTotalBlocksMinusX_;

	}

	inline unsigned short chunk::nTotalBlocksPlusY() const {

		return nTotalBlocksPlusY_;

	}

	inline unsigned short chunk::nTotalBlocksMinusY() const {

		return nTotalBlocksMinusY_;

	}

	inline unsigned short chunk::nTotalBlocksPlusZ() const {

		return nTotalBlocksPlusZ_;

	}

	inline unsigned short chunk::nTotalBlocksMinusZ() const {

		return nTotalBlocksMinusZ_;

	}

	inline bool chunk::isEmptyBlock(GLbyte x, GLbyte y, GLbyte z) const {
	
		return blocksLocalIDs_.at(x,y,z) == 0;
	
	}

	inline bool chunk::isEmptyBlock(const ivec3& inChunkPos) const {
	
		return isEmptyBlock(inChunkPos.x, inChunkPos.y, inChunkPos.z);
	
	}

	inline const palette<unsigned short, unsigned int>& chunk::getPalette() const {
	
		return palette_;
	
	}

	inline const std::unordered_map<unsigned short, unsigned short>& chunk::getPaletteCount() const {
	
		return paletteCount_;
	
	}

	inline const std::unordered_set<unsigned short>& chunk::getFreeLocalIDs() const {
	
		return freeLocalIDs_;
	
	}

	inline const std::unordered_set<ivec3>& chunk::getFloodPointLightPositions() const {
	
		return floodPointLightPositions_;
	
	}

	inline const basicVec4& chunk::getBlockLightColor(const ivec3& chunkRelPos) {
	
		// TODO. DO AN AT METHOD FOR THE PADDEDARRAY CLASS.
		return blockLightColor_[chunkRelPos.x][chunkRelPos.y][chunkRelPos.z];
	
	}

	inline char chunk::getBlockLightLevel(const ivec3& chunkRelPos) {
	
		return blockLightLevel_[chunkRelPos.x][chunkRelPos.y][chunkRelPos.z];
	
	}

	inline bool chunk::isOwned() const {
	
		return !owners_.empty();
	
	}

	inline bool chunk::unloadWhenNoOwners() const {
	
		return unloadWhenNoOwners_;
	
	}

	inline const std::unordered_map<ivec3, chunk*>& chunk::ownedChunks() const {
	
		return ownedChunks_;
	
	}

	inline Padded3DArray<unsigned short>& chunk::blocks() {
	
		return blocksLocalIDs_;
	
	}

	inline palette<unsigned short, unsigned int>& chunk::getPalette() {
	
		return palette_;
	
	}

	inline std::unordered_map<unsigned short, unsigned short>& chunk::getPaletteCount() {
	
		return paletteCount_;
	
	}

	inline std::unordered_set<unsigned short>& chunk::getFreeLocalIDs() {
	
		return freeLocalIDs_;
	
	}

	inline std::unordered_set<ivec3>& chunk::getFloodPointLightPositions() {

		return floodPointLightPositions_;

	}

	inline const block& chunk::setBlock(const ivec3& chunkRelPos, const block& b, bool modification) {

		return setBlock(chunkRelPos.x, chunkRelPos.y, chunkRelPos.z, b, modification);

	}

	inline const block& chunk::setBlock(unsigned int linearIndex, const block& b, bool modification) {

		return setBlock(linearToVec3(linearIndex, CHUNK_SIZE, CHUNK_SIZE), b, modification);

	}

	inline void chunk::replaceBlockLight(const block& oldB, const block& b, byte x, byte y, byte z) {
	
		return replaceBlockLight(oldB, b, ivec3(x, y, z));
	
	}

	inline void chunk::setBlockLight(const block& b, byte x, byte y, byte z) {

		return setBlockLight(b, ivec3(x, y, z));

	}

	inline chunkRenderingData& chunk::renderingData() {

		return renderingData_;

	}

	inline std::shared_mutex& chunk::blocksDataMutex() {
	
		return blocksMutex_;
	
	}

	inline void chunk::lockSharedRenderingDataMutex() {
	
		renderingDataMutex_.lock_shared();
	
	}

	inline bool chunk::tryLockRenderingDataMutex() {

		 return renderingDataMutex_.try_lock();

	}

	inline void chunk::unlockSharedRenderingDataMutex() {
	
		renderingDataMutex_.unlock_shared();
	
	}

	inline void chunk::unlockRenderingDataMutex() {

		renderingDataMutex_.unlock();

	}

	inline void chunk::needsRemesh(bool newValue) {

		needsRemesh_ = newValue;

	}

	inline void chunk::loadedFromDisk(bool newValue) {
	
		loadedFromDisk_ = newValue;
	
	}

	inline void chunk::modified(bool newValue) {

		modified_ = newValue;

	}

	inline void chunk::addBlockLight(const ivec3& inChunkPos, const basicVec4& color, char intensity, bool markToBeRemeshed) {

		addBlockLight(inChunkPos.x, inChunkPos.y, inChunkPos.z, color, intensity, markToBeRemeshed);

	}

	inline void chunk::addBlockLight(const ivec3& inChunkPos, char xOffset, char yOffset, char zOffset,
		const basicVec4& color, char intensity, bool markToBeRemeshed) {

		addBlockLight(inChunkPos.x + xOffset, inChunkPos.y + yOffset, inChunkPos.z + zOffset, color, intensity, markToBeRemeshed);

	}

	inline void chunk::applyBlockLight(const ivec3& inChunkPos, const basicVec4& color, char intensity, bool markToBeRemeshed) {
	
		applyBlockLight(inChunkPos.x, inChunkPos.y, inChunkPos.z, color, intensity, markToBeRemeshed);
	
	}

	inline void chunk::applyBlockLight(const ivec3& inChunkPos, char xOffset, char yOffset, char zOffset,
		const basicVec4& color, char intensity, bool markToBeRemeshed) {
	
		applyBlockLight(inChunkPos.x + xOffset, inChunkPos.y + yOffset, inChunkPos.z + zOffset, color, intensity, markToBeRemeshed);
	
	}

	inline void chunk::removeBlockLight(const ivec3& inChunkPos, bool markToBeRemeshed) {
	
		removeBlockLight(inChunkPos.x, inChunkPos.y, inChunkPos.z, markToBeRemeshed);
	
	}

	inline void chunk::loadStatus(chunkLoadStatus level) {

		loadStatus_ = level;

	}

	inline std::recursive_mutex& chunk::loadStatusMutex() {
	
		return loadStatusMutex_;
	
	}

	inline void chunk::nOpaqueBlocks(unsigned short newValue) {

		nOpaqueBlocks_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusX(unsigned short newValue) {

		nOpaqueBlocksPlusX_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusX(unsigned short newValue) {

		nOpaqueBlocksMinusX_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusY(unsigned short newValue) {

		nOpaqueBlocksPlusY_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusY(unsigned short newValue) {

		nOpaqueBlocksMinusY_ = newValue;

	}

	inline void chunk::nOpaqueBlocksPlusZ(unsigned short newValue) {

		nOpaqueBlocksPlusZ_ = newValue;

	}

	inline void chunk::nOpaqueBlocksMinusZ(unsigned short newValue) {

		nOpaqueBlocksMinusZ_ = newValue;

	}

	inline void chunk::nTotalBlocks(unsigned short newValue) {

		nTotalBlocks_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusX(unsigned short newValue) {

		nTotalBlocksPlusX_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusX(unsigned short newValue) {

		nTotalBlocksMinusX_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusY(unsigned short newValue) {

		nTotalBlocksPlusY_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusY(unsigned short newValue) {

		nTotalBlocksMinusY_ = newValue;

	}

	inline void chunk::nTotalBlocksPlusZ(unsigned short newValue) {

		nTotalBlocksPlusZ_ = newValue;

	}

	inline void chunk::nTotalBlocksMinusZ(unsigned short newValue) {

		nTotalBlocksMinusZ_ = newValue;

	}

	inline void chunk::getAndOwnChunks(bool relativeCoords) {

		getAndOwnChunksWithOffset_(-1, -1, -1, 1, 1, 1, relativeCoords);

	}

	inline void chunk::getAndOwnChunks(int nChunksX, int nChunksY, int nChunksZ, bool relativeCoords) {

		getAndOwnChunksWithOffset_(0, 0, 0, nChunksX, nChunksY, nChunksZ, relativeCoords);

	}

	inline std::recursive_mutex& chunk::ownedChunksMutex() {
	
		return ownedChunksMutex_;
	
	}

	inline std::unordered_map<ivec3, chunk*>& chunk::ownedChunks() {
	
		return ownedChunks_;
	
	}

	inline chunk::~chunk() {}


	// 'chunkEvent' class.

	/**
	* @brief Event concerning a specific chunk.
	*/
	class chunkEvent : public event {

	public:

		// Constructors.

		/**
		* Class constructor.
		*/
		chunkEvent(const std::string& name);


		// Observers.

		/**
		* Returns the last chunk XZ coordinates stored within the event.
		*/
		const ivec2& chunkPosXZ() const;


		// Modifiers.

		/**
		* Assign parameters that will be used by the listeners notify() method.
		*/
		void notify(int chunkPosX, int chunkPosZ);

		/**
		* Assign parameters that will be used by the listeners notify() method.
		*/
		void notify(const ivec2& chunkPosXZ);

	protected:

		ivec2 chunkPosXZ_;

	private:

		virtual void notify() {};

	};

	inline chunkEvent::chunkEvent(const std::string& name)
	: event(name), chunkPosXZ_{ 0, 0 }
	{}

	inline const ivec2& chunkEvent::chunkPosXZ() const {
	
		return chunkPosXZ_;
	
	}

	inline void chunkEvent::notify(int chunkPosX, int chunkPosZ) {
	
		notify(ivec2{ chunkPosX, chunkPosZ });
	
	}

	
	// 'chunkManager' class.
	
	/**
	* @brief Used for managing the chunks' life cycle, level loading...
	*/
	class chunkManager {

	public:

		/*
		Methods.
		*/

		// Initializers.

		/**
		* @brief Initialise the chunk management system.
		*/
		static void init();


		// Observers.

		/**
		* @brief Returns true if the system is initialised or false otherwise.
		*/
		static bool initialised();

		/**
		* @brief Returns the system's dictionary of currently in player's view.
		*/
		static const chunksMap& chunks();

		/**
		* @brief Returns the system's dictionary of chunks that are not in player's view but currently loaded.
		*/
		static const chunksMap& simulatedChunks();

		/**
		* @brief Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified with priority updates are updated.
		*/
		static void updatePriorityReadChunkMeshes();

		/**
		* @brief Update the read-only copy of the chunks' meshes that are ready to be sent to GPU.
		* Only the chunks that were modified without priority updates are updated.
		*/
		static void updateReadChunkMeshes();

		/**
		* @brief Swap the write and read chunk mesh buffers so that the rendering thread may obtain the most updated one.
		*/
		static void swapChunkMeshesBuffers();

		/**
		* @brief Swap the write and read chunk mesh priority buffers so that the rendering thread may obtain the most updated one.
		*/
		static void swapChunkMeshesPriorityBuffers();

		/**
		* @brief Get the read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		* @returns The read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		*/
		static std::unordered_map<ivec3, chunkVBOop> const * chunkVBOoperationsRead();

		/**
		* @brief Get the read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		* @returns The read-only copy of the operations that need to be performed on
		* the rendering thread's chunk VBO.
		*/
		static std::unordered_map<ivec3, chunkVBOop> const* chunkVBOoperationsPriorityRead();

		/**
		* @brief Returns the current number of chunks to compute i n the X and Z axes from the position of the player.
		*/
		static unsigned int nChunksToCompute();

		/**
		* @brief Get the block ID of a specified block position.
		*/
		static const block& getBlock(int posX, int posY, int posZ);

		/**
		* @brief Get the block ID of a specified block position.
		*/
		static const block& getBlock(const ivec3& blockPos);

		/**
		* @brief Returns true if the given chunk position is inside the level's boundaries or false otherwise.
		*/
		static bool isChunkInWorld(const ivec3& chunkPos);

		/**
		* @brief Get the specified chunk's load status.
		* @param chunkPos The chunk-grid coordinates of the specified chunk.
		* @returns The specified chunk's load status.
		*/
		static chunkLoadStatus getChunkLoadStatus(const ivec3& chunkPos);

		/**
		* @brief Returns true if the given chunk position is inside the level's boundaries or false otherwise.
		*/
		static bool isChunkInWorld(int chunkX, int chunkY, int chunkZ);

		/**
		* @brief Returns the currently opened terrain file.
		*/
		static const std::string& openedTerrainFileName();

		/**
		* @brief Returns true if the block at the specified global coordinates is an empty block
		* or false otherwise.
		*/
		static bool isEmptyBlock(int posX, int posY, int posZ);

		/**
		* @brief Returns true if the specified chunk is inside the player's render distance or
		* false otherwise.
		*/
		static bool chunkInRenderDistance(const chunk* chunk);

		/**
		* @brief Returns true if the specified chunk position is inside the player's render distance or
		* false otherwise.
		* @param chunkPos The specified chunk's position.
		*/
		static bool chunkInRenderDistance(const ivec3& chunkPos);

		/**
		* @brief Returns true if the specified chunk position is inside the player's render distance or
		* false otherwise.
		* @param chunkPos The specified chunk's position.
		* @param The player's position.
		*/
		static bool chunkInRenderDistance(const ivec3& chunkPos, const vec3& playerPos);

		/**
		* @brief Returns true if the specified chunk position is inside the player's render distance or
		* false otherwise.
		*/
		static bool chunkInRenderDistance(int chunkPosX, int chunkPosY, int chunkPosZ);

		/**
		* @brief Returns distance between the player and the specified chunk position in the three axes in chunk coordinates.
		*/
		static ivec3 chunkDistanceToPlayer(const ivec3& chunkPos);

		/**
		* @brief Returns the distance between two chunk positions in chunk coordinates.
		*/
		static ivec3 chunkDistance(const ivec3& chunkPos1, const ivec3& chunkPos2);

		/**
		* @brief Returns the distance between two chunk positions in chunk coordinates.
		*/
		static ivec3 chunkSignedDistance(const ivec3& chunkPos1, const ivec3& chunkPos2);

		/**
		* @brief Returns the SQUARED distance in global position between the specified chunk position and the player's position.
		*/
		static double distanceToPlayer(const ivec3& chunkPos);

		/**
		* @brief Returns the onChunkLoad chunkEvent associated with the chunk management system.
		*/
		static const chunkEvent& onChunkLoadC();

		/**
		* @brief Returns the onChunkUnload chunkEvent associated with the chunk management system.
		*/
		static const chunkEvent& onChunkUnloadC();

		/**
		* @brief Returns the maximun number of chunks to compute according
		* to the current chunk render distance.
		*/
		static unsigned int nMaxChunksToCompute();

		/**
		* @brief Returns the maximun number of vertices to compute for a chunk.
		*/
		static unsigned int nMaxChunkVertsToCompute();

		/**
		* @brief Returns the condition variable associated with the chunk priority updates management thread.
		*/
		static const std::condition_variable& priorityManagerThreadCV_C();

		/**
		* @brief Returns the condition variable associated with the priority new chunks meshes list.
		*/
		static const std::condition_variable& priorityNewChunkMeshesCV_C();

		/**
		* @brief Get the chunk specified at the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		* @returns The specified chunk.
		*/
		static const chunk* getChunkC(const ivec3& chunkPos);

		/**
		* @brief Get the NeighborInfo object corresponding to the chunk of the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		*/
		static neighborsInfo* getChunkNeighborInfo(const ivec3& chunkPos);


		// Modifiers.

		/**
		* @brief Set the number of chunks to compute in the X and Z axes.
		*/
		static void setNChunksToCompute(unsigned int nChunksToCompute);

		/**
		* @brief Set the block ID of a specfied block position.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		static const block& setBlock(const ivec3& blockPos, const block& blockID);

		/**
		* @brief Set the block ID of a specfied block position.
		* 'triggerRenderingSync' tells the chunk manager system to perform a forcible
		* synchronization between all threads related to chunk rendering in order
		* to update the chunk's mesh that was modified by this method.
		*/
		static const block& setBlock(int x, int y, int z, const block& blockID);

		/**
		* @brief Select the neighbor -X chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusX(const ivec3& chunkPos);

		/**
		* @brief Select the neighbor +X chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusX(const ivec3& chunkPos);

		/**
		* @brief Select the neighbor -Y chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusY(const ivec3& chunkPos);

		/**
		* @brief Select the neighbor +Y chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusY(const ivec3& chunkPos);

		/**
		* @brief Select the neighbor -Z chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborMinusZ(const ivec3& chunkPos);

		/**
		* @brief Select the neighbor +Z chunk for the chunk with the specified chunk position
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* neighborPlusZ(const ivec3& chunkPos);

		/**
		* @brief Returns the mutex that guards the registered chunks dictionary.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::recursive_mutex& chunksMutex();

		/**
		* @brief Returns the mutex associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& managerThreadMutex();

		/**
		* @brief Returns the mutex associated with the priority chunk updates management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::mutex& priorityManagerThreadMutex();

		/**
		* @brief Returns the condition variable associated with the chunk management thread.
		* WARNING. Not meant for use in AI mode.
		*/
		static std::condition_variable& managerThreadCV();

		/**
		* @brief Returns the condition variable associated with the chunk priority updates management thread.
		*/
		static std::condition_variable& priorityManagerThreadCV();

		/**
		* @brief Returns the condition variable associated with the priority new chunks meshes list.
		*/
		static std::condition_variable& priorityNewChunkMeshesCV();

		/**
		* @brief Locks the calling thread until:
		* - Minimal terrain has been loaded around the player (infinite world).
		* - The entire level has been loaded (finite world).
		*/
		static void waitInitialTerrainLoaded();

		/**
		* @brief Unloads the chunk at chunk position 'chunkPos', pushing it into a free chunks deque
		* to be reused later for another chunk position of the world. Returns the number of chunks
		* that were converted into frontier chunks because of this operation.
		*/
		static void unloadFrontierChunk(const ivec3& chunkPos);

		/**
		* @brief Method called by the chunk management thread to use with infinite world types.
		* It coordinates all meshing threads and manages the chunk unloading process
		* in order to prevent any race condition between said threads, among other things
		* such as synchronization and data transfering with the rendering thread.
		* NOTE. This method does not handle the chunk priority updates.
		*/
		static void manageChunks();

		/**
		* @brief Similar to chunkManager::manageChunks() but only for priority chunk updates.
		*/
		static void manageChunkPriorityUpdates();

		/**
		* @brief Set 'newName' to "" to clear the opened terrain file name.
		*/
		static void openedTerrainFileName(const std::string& newName);

		/**
		* @brief Load and mesh the chunk in the specified chunk coordinates if visible.
		* Returns true if the chunk could was visible, loaded and meshed or false
		* otherwise.
		*/
		static bool ensureChunkIfVisible(const ivec3& chunkPos);

		/**
		* @brief Load and mesh the chunk in the specified chunk coordinates if visible.
		* Returns true if the chunk could was visible, loaded and meshed or false
		* otherwise.
		*/
		static bool ensureChunkIfVisible(int x, int y, int z);

		/**
		* @brief Serialize the chunk's data in order to save it into auxiliary memory.
		*/
		static std::string serializeChunk(chunk* c);

		/**
		* @brief Deserialize the given chunk data into the given chunk.
		* @param c The given chunk to fill with the given chunk data.
		* @param data The data to fill the given chunk with.
		*/
		static void deserializeChunk(chunk* c, const std::string& data);

		/**
		* @brief Load frontier chunk at specified chunk coordinates and return a pointer to its corresponding object.
		* If it is already loaded, it only returns its corresponding object.
		* NOTE. If it founds serialized data corresponding to this chunk, it will fill the chunk
		* with said data instead of using the world generator.
		* WARNING. DOES NOT CHECK IF THERE IS ALREADY A CHUNK AT THE SPECIFIED POSITION.
		*/
		static chunk* loadFrontierChunk(const ivec3& chunkPos);

		/**
		* @brief Issue a job related to chunk processing.
		* The job will be executed on another thread and will lock the chunk's mutexes that
		* are required.
		* @param type Type of job to issue.
		* @param c The chunk associated with the job to issue.
		* @param pushJobBack. Whether to insert the job at the back of the queue (true) or at the beginning (false).
		*/
		static void issueChunkJob(chunkJobType type, chunk* c, bool pushJobBack = true);

		/** 
		* @brief Used on chunkManager::onUnloadAsFrontier to update the neighbor
		* chunks of a frontier chunk that was unloaded.
		*/
		static void onUnloadAsFrontier(const ivec3& chunkPos);

		/**
		* @brief Convert the specified chunk into a frontier chunk if it is not.
		*/
		static void addFrontier(chunk* chunk);

		/**
		* @brief Renews the mesh of the specified chunk and marks it if it is ready to be drawn.
		* Sets the chunk's status to MESHED.
		* @param c The chunk to remesh.
		* @param isPriorityUpdate Whether the remesh operation has priority (true) or not (false).
		* @param forceRemesh Whether the remesh operation ignores the needsRemesh_ flag (true) or not (false).
		*/
		static void remesh(chunk* c, bool isPriorityUpdate, bool forceRemesh = false);

		/**
		* @brief Renews the mesh of the specified chunk and marks it if it is ready to be drawn.
		*/
		static void renewMesh(const ivec3& chunkPos, bool isPriorityUpdate);

		/**
		* @brief Returns the onChunkLoad chunkEvent associated with the chunk management system.
		*/
		static chunkEvent& onChunkLoad();

		/**
		* @brief Returns the onChunkUnload chunkEvent associated with the chunk management system.
		*/
		static chunkEvent& onChunkUnload();

		/**
		* @brief Get the chunk specified at the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		* @returns The specified chunk.
		*/
		static chunk* getChunk(const ivec3& chunkPos);

		/**
		* @brief Select a chunk with the specified block position by
		* converting the global position cords x, y and z into chunk position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* getChunkByChunkPos(int x, int y, int z);

		/**
		* @brief Select a chunk with the specified global position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* getChunkByRealPos(float x, float y, float z);

		/**
		* @brief Select a chunk with the specified global position.
		* WARNING. Not meant for use in AI mode.
		*/
		static chunk* getChunkByRealPos(const vec3& pos);

		/**
		* @brief Own a chunk, making it unavailable to be freed or have its terrain unloaded unless all owners cease owning it.
		* @param chunkPos. The specified chunk's chunk-grid coordinates to add an owner to.
		* @param ownerChunkPos. The specified owner's chunk-grid coordinates.
		*/
		static chunk* addOwner(const ivec3& chunkPos, const ivec3& ownerChunkPos);

		/**
		* @brief Disown a chunk, making it unavailable to be freed or have its terrain unloaded until all owners cease owning it.
		* @param c The chunk to remove the owner from.
		* @param ownerChunkPos. The specified owner's chunk-grid coordinates.
		*/
		static void removeOwner(chunk& c, const ivec3& ownerChunkPos);

		/**
		* @brief Get the mutex that provides mutual exclusion for the dictionary of chunk's neighborInfo objects.
		* @returnsThe mutex that provides mutual exclusion for the dictionary of chunk's neighborInfo objects.
		*/
		static std::mutex& chunkNeighborsInfoMutex();

		/**
		* @brief Get (or create if not existing) a chunk neighbor info object for the chunk corresponding
		* to the given chunk-grid coordinates.
		* @param chunkPos The given chunk-grid coordinates.
		*/
		static std::shared_ptr<neighborsInfo> getOrCreateChunkNeighborInfo(const ivec3& chunkPos);

		/**
		* @brief Add a global tick function to the level.
		* @param c The chunk whose lights are to be processed.
		* @param causesPriorityUpdate Whether the chunks affected by these lights' processing perform
		* priority meshing updates (true) or not (false).
		* @param pushJobBack Whether to push the job to the back of the processing list (true) or the front (false).
		*/
		static void addLightTickJob(chunk* c, bool causesPriorityUpdate, bool pushJobBack);

		/**
		* @brief Create a new chunk object for the specified chunk grid coordinates.
		* @param ownNeighborChunks Whether to automatically own (and register if as simulated if not currently registered) 
		* this chunk's neighbors (true) or not (false).
		* @param isSimulated Whether to register this chunk as a simulated one (true) or a normal one (false).
		*/
		static chunk* registerChunk(const ivec3& chunkPos, bool ownNeighborChunks, bool isSimulated = false);


		// Clean Up.

		static void clearChunks();

		/**
		* @brief Only cleans any resources like chunks created but it does not de-initialise
		* the chunk management system.
		*/
		static void clear();

		/**
		* @brief Frees any memory allocated in the process of generating the world, like
		* all the Chunk objects created to load it.
		*/
		static void reset();

	private:

		/*
		Nested classes/types/enums.
		*/

		/**
		* @brief Comparator function object class used to sort a collection of chunks with how
		* closer they are to the player.
		*/
		class closestChunk {

		public:

			/**
			* @brief NOTE. Per C++ standards, if this operator returns true it means that chunkPos1 goes before
			* chunkPos2 in a properly sorted collection.
			* Used to sort a collection of chunk positions with how
			* closer they are to the player. Returns true if chunkPos1's distance to the player
			* in chunk coordinates is less than the distance of chunkPos2 to the player in chunk coordinates
			* or false otherwise.
			* In case they have the same distance to the player in chunk coordinates, it returns true if chunkPos1 < chunkPos2.
			*/
			inline bool operator()(const ivec3& chunkPos1, const ivec3& chunkPos2) const {

				return distanceToPlayer(chunkPos1) < distanceToPlayer(chunkPos2);

			}

		};

		/**
		* Brief Predicate Function that is used to determine whether a chunk-coordiante position is inside
		* the player's render distance or not.
		*/
		class notInRenderDistance {

		public:

			// Modifiers.

			/**
			* @brief Returns true if the specified chunk-coordinate position is inside the player's
			* render distance or false otherwise.
			*/
			inline bool operator()(const ivec3& chunkPos) const {

				return !chunkInRenderDistance(chunkPos);

			}

		};

		

		/*
		Attributes.
		*/

		static bool initialised_;
		static bool chunkJobSystemOverloaded_;
		static int nChunksToCompute_;

		static std::atomic<bool> clearChunksFlag_;

		/*
		Used to force all meshing threads to synchronize with
		the rendering thread when a high priority
		chunk update is issued and it's imperative than the update
		made is reflected in the rendering thread as quickly as possible.
		*/
		static std::atomic<bool> waitInitialTerrainLoaded_;

		// Chunks that are loaded by the player.
		static chunksMap clientChunks_;

		// Chunks that are not loaded by the player
		// (for example, chunks loaded by an AI agent 
		// that is mining blocks far away from it). TODO <- HACER QUE LOS CHUNKS QUE NO SEAN CARGADOS POR JUGADOR SE METAN AQUI. ESTOS CHUNKS NO SON RENDERIZADOS A NO SER QUE EL PLAYER ESTÉ, EN CUYO CASO ESTARÍA SOLO COMO CLIENT_CHUNK.
													// ASI, SI SE PIDE UN CHUNK POR UN METODO GENÉRICO, SI NO SE ENCUENTRA ESE CHUNK EN CLIENT CHUNKS, SE CARGA COMO SIMULATED CHUNK.
		static chunksMap simulatedChunks_;

		static std::unordered_set<ivec3>* chunkMeshes_; // Chunk meshes generated by renewMesh() without priority.
		static std::unordered_map<ivec3, chunkVBOop>* chunkVBOopsWrite_;
		static std::unordered_map<ivec3, chunkVBOop>* chunkVBOopsPriorityWrite_;
		static std::unordered_map<ivec3, chunkVBOop>* chunkVBOopsRead_;
		static std::unordered_map<ivec3, chunkVBOop>* chunkVBOopsPriorityRead_;

		static std::list<chunk*> newChunkMeshes_;
		static std::list<chunk*> priorityNewChunkMeshes_;
			                      
		static std::mutex managerThreadMutex_;
		static std::mutex priorityManagerThreadMutex_;
		static std::mutex priorityNewChunkMeshesMutex_;
		static std::mutex priorityUpdatesRemainingMutex_;
		static std::mutex loadingTerrainMutex_;
		static std::recursive_mutex newChunkMeshesMutex_,
						            chunksMutex_;
		static std::condition_variable managerThreadCV_,
									   priorityManagerThreadCV_,
									   priorityNewChunkMeshesCV_,
									   loadingTerrainCV_;;

		static std::unordered_map<unsigned int, std::unordered_map<ivec3, bool>> AIChunkAvailable_;
		static std::unordered_map<unsigned int, std::unordered_map<ivec3, chunk*>> AIagentChunks_;
		static unsigned int selectedAIWorld_;
		static bool originalWorldAccess_;

		static ivec3 playerChunkPosCopy_; // Copy of the last value of the player's position in chunk coordinates.

		static std::list<ivec3> frontierChunks_;
		static std::unordered_map<ivec3, std::list<ivec3>::iterator> frontierChunksSet_;
		static std::list<ivec3>::iterator frontierIt_;

		static closestChunk closestChunk_;

		static threadPool* chunkTasks_;
		static threadPool* priorityChunkTasks_;
		static std::unordered_map<ivec3, unsigned int> currentJobsPerChunk_;
		static atomicRecyclingPool<job>* loadChunkJobs_;
		static atomicRecyclingPool<chunk> chunksPool_;

		static std::mutex lightTickFunctionsMutex_;
		static atomicRecyclingPool<job>* lightJobs_;
		static std::list<tickFunc> pendingLightJobs_;
		static threadPool* lightTasks_;

		static chunkEvent onChunkLoad_;
		static chunkEvent onChunkUnload_;

		static std::mutex chunkNeighborsInfoMutex_;
		static std::unordered_map<ivec3, std::shared_ptr<neighborsInfo>> chunkNeighborsInfo_;

		static chunkVertexBuffer* vbo_;

		static std::string openedTerrainFileName_; // TODO. DEPRECEATED. REPLACE WITH THE WORLD SYSTEM EQUIVALENT NAMED 'currentWorldPath_'.

		/*
		Methods.
		*/

		// Observers.

		static chunkExistence chunkExists_(const ivec3& chunkPos);

		static chunk* getChunk_(const ivec3& chunkPos);

		static chunk* getChunk_(const ivec3& chunkPos, chunkExistence& existence, bool createIfDoesntExist = false);

		static const block& getBlockOGWorld_(int posX, int posY, int posZ);

		// Modifiers.

		static void pushNewChunkMesh_(bool isPriorityUpdate, chunk* c, std::size_t meshSize);

		static void simulatedChunkToCommon_(chunk* c);

		static void loadOrRemesh_(chunk* c);

		// Increase own neighborinfo object pass counter by 1 and for its neighbors objects too. 
		// Cannot exceed 27. If it reaches 27 (number of neighbors a chunk can have + 1)
		static void increaseNeighborInfoPassCounter_(chunk* c, bool sendLoad2JobWhenRequired = false); 
		
		// Decrease own neighborinfo object pass counter by 1 and for its neighbors objects too. Erase any object whose counter reaches 0.--
		static void decreaseNeighborInfoPassCounter_(chunk& c); 

		static void processWorldLightUpdates_();

		// Recalculate all blocklights from chunk c towards itself and its neighbors.
		// WARNING. DOESN'T CLEAR PREVIOUS APPLIED LIGHTS.
		static void recalculateBlockLight_(chunk& c, bool priorityUpdate);

		// Recalculate all blocklights from chunk c towards itself and its neighbors after a block light removal.
		static void recalculateBlockLightAfterRemoval_(chunk& c, bool priorityUpdate, std::initializer_list<ivec3> blockLightsToRemove);

		/*static void removeLightChannel_(colorChannel channel, blockLightMod& floodLight, const std::unordered_map<ivec3, chunk*>& ownedChunks,
			std::unordered_map<ivec3, chunkBlockData>& ownedBlockData, std::unordered_map<ivec3, Padded3DArray<char>>& blockLightIntensity,
			const ivec3& chunkPosOffset, const ivec3& chunkRelPos);*/

		// Used in recalculateBlockLight_ to get required data related to the lighting of the chunk and its neighbors 
		// that are going to get recalculated.
		static bool getDataForCalculatingBlockLight_(chunk& c, const std::unordered_map<ivec3, chunk*>& ownedChunks,
			std::unordered_map<ivec3, chunkBlockData>& ownedBlockData, std::unordered_map<ivec3, Padded3DArray<char>>& blockLightIntensity);

		/*
		Jobs.
		*/

		static void loadChunkJob(void* data);

		static void loadChunkJobPass2(void* data);

		static void remeshChunkJob(void* data);

		static void unloadAndSaveChunkJob(void* data);

		static void priorityRemeshChunkJob(void* data);

		static void processLightJob(void*);

	};

	inline bool chunkManager::initialised() {
	
		return initialised_;
	
	}

	inline const chunksMap& chunkManager::chunks() {

		return clientChunks_;

	}

	inline const chunksMap& chunkManager::simulatedChunks() {

		return simulatedChunks_;

	}

	inline std::unordered_map<ivec3, chunkVBOop> const * chunkManager::chunkVBOoperationsRead() {
	
		return chunkVBOopsRead_;
	
	}

	inline std::unordered_map<ivec3, chunkVBOop> const* chunkManager::chunkVBOoperationsPriorityRead() {

		return chunkVBOopsPriorityRead_;

	}

	inline unsigned int chunkManager::nChunksToCompute() {

		return nChunksToCompute_;

	}

	inline const block& chunkManager::getBlock(const ivec3& blockPos) {

		return getBlock(blockPos.x, blockPos.y, blockPos.z);

	}

	inline bool chunkManager::chunkInRenderDistance(const chunk* chunk) {

		return chunkInRenderDistance(chunk->chunkPos());

	}

	inline bool chunkManager::chunkInRenderDistance(int chunkPosX, int chunkPosY, int chunkPosZ) {
	
		return chunkInRenderDistance(ivec3{ chunkPosX, chunkPosY, chunkPosZ });
	
	}

	inline bool chunkManager::isChunkInWorld(int chunkX, int chunkY, int chunkZ) {

		return isChunkInWorld(ivec3{ chunkX, chunkY, chunkZ });

	}

	inline const std::string& chunkManager::openedTerrainFileName() {

		return openedTerrainFileName_;
		
	}

	inline const chunkEvent& chunkManager::onChunkLoadC() {
	
		return onChunkLoad_;
	
	}

	inline const chunkEvent& chunkManager::onChunkUnloadC() {
	
		return onChunkUnload_;
	
	}

	inline unsigned int chunkManager::nMaxChunksToCompute() {

		return nChunksToCompute_ * 2 * nChunksToCompute_ * 2 * totalYChunks;

	}

	inline unsigned int chunkManager::nMaxChunkVertsToCompute() {

		return nBlocksChunk * 36; // 6 faces * 6 verticesPerFace = maximum 36 vertices per block.
	
	}

	inline const std::condition_variable& chunkManager::priorityManagerThreadCV_C() {
	
		return priorityManagerThreadCV_;
	
	}

	inline const std::condition_variable& chunkManager::priorityNewChunkMeshesCV_C() {
	
		return priorityNewChunkMeshesCV_;
	
	}

	inline const chunk* chunkManager::getChunkC(const ivec3& chunkPos) {
	
		std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
		return clientChunks_.contains(chunkPos) ? clientChunks_[chunkPos] : simulatedChunks_.contains(chunkPos) ? simulatedChunks_[chunkPos] : nullptr;

	}
	
	inline const block& chunkManager::setBlock(const ivec3& pos, const block& blockID) {

		return setBlock(pos.x, pos.y, pos.z, blockID);

	}

	inline std::recursive_mutex& chunkManager::chunksMutex() {

		return chunksMutex_;

	}

	inline std::mutex& chunkManager::managerThreadMutex() {

		return managerThreadMutex_;

	}

	inline std::mutex& chunkManager::priorityManagerThreadMutex() {

		return priorityManagerThreadMutex_;

	}

	inline std::condition_variable& chunkManager::managerThreadCV() {

		return managerThreadCV_;

	}

	inline std::condition_variable& chunkManager::priorityManagerThreadCV() {

		return priorityManagerThreadCV_;

	}

	inline std::condition_variable& chunkManager::priorityNewChunkMeshesCV() {
	
		return priorityNewChunkMeshesCV_;
	
	}

	inline bool chunkManager::ensureChunkIfVisible(int chunkPosX, int chunkPosY, int chunkPosZ) {

		return ensureChunkIfVisible(ivec3{ chunkPosX, chunkPosY, chunkPosZ });

	}

	inline chunkEvent& chunkManager::onChunkLoad() {
	
		return onChunkLoad_;
	
	}

	inline chunkEvent& chunkManager::onChunkUnload() {

		return onChunkUnload_;

	}

	inline chunk* chunkManager::getChunk(const ivec3& chunkPos) {

		return getChunk_(chunkPos);

	}

	inline chunk* chunkManager::getChunkByRealPos(const vec3& pos) {

		return getChunkByRealPos(pos.x, pos.y, pos.z);

	}

	inline std::mutex& chunkManager::chunkNeighborsInfoMutex() {
	
		return chunkNeighborsInfoMutex_;
	
	}

	inline void chunkManager::clearChunks() {

		clearChunksFlag_ = true;

	}

	inline chunkExistence chunkManager::chunkExists_(const ivec3& chunkPos) {

		std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
		return clientChunks_.contains(chunkPos) ? chunkExistence::COMMON : simulatedChunks_.contains(chunkPos) ? chunkExistence::SIMULATED : chunkExistence::NOEXISTS;

	}

	inline chunk* chunkManager::getChunk_(const ivec3& chunkPos) {

		std::unique_lock<std::recursive_mutex> lock(chunksMutex_);
		return clientChunks_.contains(chunkPos) ? clientChunks_[chunkPos] : simulatedChunks_.contains(chunkPos) ? simulatedChunks_[chunkPos] : nullptr;

	}

}

#endif
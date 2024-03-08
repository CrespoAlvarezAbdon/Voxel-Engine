#ifndef _VOXELENG_TREENODE_
#define _VOXELENG_TREENODE_

namespace VoxelEng {

	/*W.I.P*/

	template<int nChildren>
	struct TreeNode {
	public:

		// Constructors.

		TreeNode();


		// Observers.

		const TreeNode& operator[](unsigned int index) const;

		const TreeNode& at(unsigned int index) const;


		// Modifiers.

		TreeNode& operator[](unsigned int index);

		TreeNode& at(unsigned int index);

		void insert();

		void insert(const TreeNode&);

		// El bool es para devolver false si no se pudo eliminar el nodo porque tiene hijos y la opcion recursively no está marcada.
		bool removeChild(unsigned int index, bool recursively);

		bool removeChildren(bool recursively);


	private:

		TreeNode** children_;

	};

}

#endif
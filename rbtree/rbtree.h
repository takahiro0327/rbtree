#ifndef ___RBTREE_H___
#define ___RBTREE_H___

#include <stdint.h>

#ifdef __cplusplus
extern "C" 
{
#endif

	typedef struct RB_NODE_ST
	{
		struct RB_NODE_ST* pLeft;
		struct RB_NODE_ST* pRight;

		union
		{
			struct RB_NODE_ST* pParent;
			uintptr_t			parentAndColor;
		};

	} RB_NODE;

	typedef struct RB_TREE_ST
	{
		RB_NODE* pRoot;

	} RB_TREE;

	extern void RBTreeInsert(RB_TREE* pTree, RB_NODE* pNode, RB_NODE* pParent);
	extern void RBTreeErase(RB_TREE* pTree, RB_NODE* pNode);

	inline const RB_NODE* RBTreeFirst(const RB_TREE* pTree)
	{
		const RB_NODE* pNode = pTree->pRoot;

		if (pNode != NULL)
		{
			while (pNode->pLeft)
			{
				pNode = pNode->pLeft;
			}
		}

		return (RB_NODE*)pNode;
	}

	inline const RB_NODE* RBTreeLast(const RB_TREE* pTree)
	{
		const RB_NODE* pNode = pTree->pRoot;

		if (pNode != NULL)
		{
			while (pNode->pRight)
			{
				pNode = pNode->pRight;
			}
		}

		return (RB_NODE*)pNode;
	}

	inline RB_NODE* RBTreeNext(const RB_NODE* pNode)
	{
		RB_NODE* pChild = pNode->pRight;

		if (pChild)
		{
			RB_NODE* pNext;

			while ((pNext = pChild->pLeft))
			{
				pChild = pNext;
			}

			return pChild;
		}

		RB_NODE* pParent = (RB_NODE*)(pNode->parentAndColor & ~1);

		while (pParent && pNode == pParent->pRight)
		{
			pNode = pParent;
			pParent = (RB_NODE*)(pNode->parentAndColor & ~1);
		}

		return pParent;
	}

	inline RB_NODE* RBTreePrev(const RB_NODE* pNode)
	{
		RB_NODE* pChild = pNode->pLeft;

		if (pChild)
		{
			RB_NODE* pNext;

			while ((pNext = pChild->pRight))
			{
				pChild = pNext;
			}

			return pChild;
		}

		RB_NODE* pParent = (RB_NODE*)(pNode->parentAndColor & ~1);

		while (pParent && pNode == pParent->pLeft)
		{
			pNode = pParent;
			pParent = (RB_NODE*)(pNode->parentAndColor & ~1);
		}

		return pParent;
	}

#ifdef __cplusplus
}
#endif


#endif

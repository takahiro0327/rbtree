#include "rbtree.h"
#include <stdio.h>
#include <assert.h>


#define SET_BLACK_AND_PARENT( pNode, pParent )		do{ (pNode)->parentAndColor = (uintptr_t)(pParent) + BLACK; }while(0)
#define SET_RED_AND_PARENT( pNode, pParent )		do{ (pNode)->parentAndColor = (uintptr_t)(pParent); }while(0)
#define GET_PARENT(pNode)							((RB_NODE*)((pNode)->parentAndColor & ~COLOR_MASK))
#define SET_PARENT( pNode, pParent )				do{ (pNode)->parentAndColor = (((pNode)->parentAndColor&COLOR_MASK) + (uintptr_t)(pParent)); }while(0)
#define IS_BLACK(pNode)								((pNode)->parentAndColor&COLOR_MASK)
#define IS_RED(pNode)								(!IS_BLACK(pNode))
#define GET_OPPOSITE_NODE_TYPE( type )				((RB_NODE_TYPE)(RightNodeOffset - (type)))
#define GET_CHILD_NODE( pNode, type )				((RB_NODE**)((uint8_t*)(pNode) + (type)))
#define GET_COLOR(pNode)							((pNode)->parentAndColor & COLOR_MASK)

#define	RED				(0)
#define	BLACK			(1)
#define	COLOR_MASK		(1)

typedef enum RB_NODE_TYPE_EN
{
	LeftNodeOffset = 0,
	RightNodeOffset = sizeof(RB_NODE*),
} RB_NODE_TYPE;

void RBTreeInsert(RB_TREE* pTree, RB_NODE* pNode, RB_NODE* pParent)
{
	pNode->pLeft = pNode->pRight = NULL;

	if (pParent)
	{
		pNode->pParent = pParent;

		for(;;)
		{
			RB_NODE* pGParent = pParent->pParent;

			if ((uintptr_t)pGParent & COLOR_MASK)
			{
				break;
			}

			RB_NODE_TYPE t = LeftNodeOffset;
			RB_NODE* pUncle = *GET_CHILD_NODE(pGParent, LeftNodeOffset);

			if (pParent == pGParent->pLeft)
			{
				t = RightNodeOffset;
				pUncle = *GET_CHILD_NODE(pGParent, RightNodeOffset);
			}

			if (pUncle && IS_RED(pUncle))
			{
				uintptr_t ggparent = pGParent->parentAndColor;
				SET_BLACK_AND_PARENT(pUncle, pGParent);
				SET_BLACK_AND_PARENT(pParent, pGParent);

				if (ggparent == BLACK)		//pGGParent == NULL
				{
					pGParent->parentAndColor = BLACK;
					break;
				}

				RB_NODE* pGGParentRed = (RB_NODE*)(ggparent - BLACK);
				pGParent->pParent = pGGParentRed;
				pParent = pGGParentRed;
				pNode = pGParent;
				continue;
			}

			RB_NODE_TYPE s = GET_OPPOSITE_NODE_TYPE(t);
			RB_NODE* tmp;

			if (*GET_CHILD_NODE(pParent, t) == pNode)
			{
				tmp = *GET_CHILD_NODE(pNode, s);
				*GET_CHILD_NODE(pParent, t) = tmp;
				if (tmp) SET_BLACK_AND_PARENT(tmp, pParent);

				SET_RED_AND_PARENT(pParent, pNode);
				*GET_CHILD_NODE(pNode, s) = pParent;
			}
			else
			{
				pNode = pParent;
			}

			RB_NODE* pGGParent = (RB_NODE*)(pGParent->parentAndColor - BLACK);
			RB_NODE** pTarget = &pTree->pRoot;

			if (pGGParent)
			{
				if (pGParent == pGGParent->pLeft)
				{
					pTarget = &pGGParent->pLeft;
				}
				else
				{
					pTarget = &pGGParent->pRight;
				}
			}

			*pTarget = pNode;
			pNode->parentAndColor = (uintptr_t)pGGParent + BLACK;

			tmp = *GET_CHILD_NODE(pNode, t);
			*GET_CHILD_NODE(pGParent, s) = tmp;
			if (tmp) SET_BLACK_AND_PARENT(tmp, pGParent);

			SET_RED_AND_PARENT(pGParent, pNode);
			*GET_CHILD_NODE(pNode, t) = pGParent;

			break;
		}
	}
	else
	{
		pNode->parentAndColor = BLACK;
	}
}

void RBTreeErase(RB_TREE* pTree, RB_NODE* pNode)
{
	RB_NODE* pParent = GET_PARENT(pNode);
	unsigned int color;

	{
		RB_NODE* pChild;

		if (pNode->pLeft == NULL)
		{
			pChild = pNode->pRight;
		}
		else if (pNode->pRight == NULL)
		{
			pChild = pNode->pLeft;
		}
		else
		{
			RB_NODE* pOld = pNode;

			pNode = pNode->pRight;

			while (pNode->pLeft)
			{
				pNode = pNode->pLeft;
			}

			if (pParent)
			{
				if (pParent->pLeft == pOld)
				{
					pParent->pLeft = pNode;
				}
				else
				{
					pParent->pRight = pNode;
				}
			}
			else
			{
				pTree->pRoot = pNode;
			}

			pChild = pNode->pRight;
			pParent = GET_PARENT(pNode);
			color = GET_COLOR(pNode);

			if (pParent == pOld)
			{
				pParent = pNode;
			}
			else
			{
				if (pChild)
				{
					SET_RED_AND_PARENT(pChild, pParent);
				}

				pParent->pLeft = pChild;
				pNode->pRight = pOld->pRight;
				SET_PARENT(pOld->pRight, pNode);
			}

			pNode->pParent = pOld->pParent;
			pNode->pLeft = pOld->pLeft;
			SET_PARENT(pOld->pLeft, pNode);
			pNode = pChild;

			goto ___blance;
		}

		color = GET_COLOR(pNode);

		if (pChild)
		{
			SET_RED_AND_PARENT(pChild, pParent);
		}

		if (pParent)
		{
			if (pParent->pLeft == pNode)
			{
				pParent->pLeft = pChild;
			}
			else
			{
				pParent->pRight = pChild;
			}
		}
		else
		{
			pTree->pRoot = pChild;
		}

		pNode = pChild;
	}

___blance:

	if (color != RED)
	{
		while ((pNode == NULL || IS_BLACK(pNode)) && pParent)
		{
			RB_NODE_TYPE s = LeftNodeOffset, t = RightNodeOffset;
			RB_NODE* pOther = *GET_CHILD_NODE(pParent, RightNodeOffset);

			if (pParent->pLeft != pNode)
			{
				pOther = *GET_CHILD_NODE(pParent, LeftNodeOffset);
				RB_NODE_TYPE tmp = s;
				s = t;
				t = tmp;
			}
			
			RB_NODE* pGParent = GET_PARENT(pParent);

			if (IS_RED(pOther))
			{
				RB_NODE* pChild = pOther;

				pOther = *GET_CHILD_NODE(pChild, s);
				*GET_CHILD_NODE(pParent, t) = pOther;

				*GET_CHILD_NODE(pChild, s) = pParent;
				SET_BLACK_AND_PARENT(pChild, pGParent);

				if (pGParent)
				{
					if (pParent == pGParent->pLeft)
					{
						pGParent->pLeft = pChild;
					}
					else
					{
						pGParent->pRight = pChild;
					}
				}
				else
				{
					pTree->pRoot = pChild;
				}

				pGParent = pChild;
				RB_NODE* pChild2 = *GET_CHILD_NODE(pOther, t);

				if (pChild2 == NULL || IS_BLACK(pChild2))
				{
					pChild2 = *GET_CHILD_NODE(pOther, s);

					if (pChild2 == NULL || IS_BLACK(pChild2))
					{
						SET_BLACK_AND_PARENT(pParent, pGParent);
						SET_RED_AND_PARENT(pOther, pParent);
						return;
					}

					RB_NODE* pTmp = *GET_CHILD_NODE(pChild2, t);

					if ((*GET_CHILD_NODE(pOther, s) = pTmp))
					{
						SET_BLACK_AND_PARENT(pTmp, pOther);
					}

					*GET_CHILD_NODE(pChild2, t) = pOther;
					SET_BLACK_AND_PARENT(pOther, pChild2);
					pOther = pChild2;
				}
				else
				{
					SET_BLACK_AND_PARENT(pChild2, pOther);
				}

				{
					RB_NODE* pTmp = *GET_CHILD_NODE(pOther, s);

					if ((*GET_CHILD_NODE(pParent, t) = pTmp))
					{
						SET_PARENT(pTmp, pParent);
					}

					*GET_CHILD_NODE(pOther, s) = pParent;
					SET_RED_AND_PARENT(pOther, pGParent);

					*GET_CHILD_NODE(pGParent, s) = pOther;
					SET_BLACK_AND_PARENT(pParent, pOther);
				}

				return;
			}
			else
			{
				RB_NODE* pChild = *GET_CHILD_NODE(pOther, t);

				if (pChild == NULL || IS_BLACK(pChild))
				{
					RB_NODE* pChild2 = *GET_CHILD_NODE(pOther, s);

					if (pChild2 == NULL || IS_BLACK(pChild2))
					{
						SET_RED_AND_PARENT(pOther, pParent);
						pNode = pParent;
						pParent = pGParent;
						continue;
					}

					RB_NODE* pTmp = *GET_CHILD_NODE(pChild2, t);

					if ((*GET_CHILD_NODE(pOther, s) = pTmp))
					{
						SET_BLACK_AND_PARENT(pTmp, pOther);
					}

					*GET_CHILD_NODE(pChild2, t) = pOther;
					SET_BLACK_AND_PARENT(pOther, pChild2);
					pOther = pChild2;
				}
				else
				{
					SET_BLACK_AND_PARENT(pChild, pOther);
				}


				{
					RB_NODE* pTmp = *GET_CHILD_NODE(pOther, s);

					if ((*GET_CHILD_NODE(pParent, t) = pTmp))
					{
						SET_PARENT(pTmp, pParent);
					}

					*GET_CHILD_NODE(pOther, s) = pParent;

					RB_NODE** pTarget = &pTree->pRoot;

					if (pGParent)
					{
						if (pGParent->pLeft == pParent)
						{
							pTarget = &pGParent->pLeft;
						}
						else
						{
							pTarget = &pGParent->pRight;
						}

						pOther->parentAndColor = pParent->parentAndColor;
					}
					else
					{
						SET_BLACK_AND_PARENT(pOther, pGParent);
					}

					*pTarget = pOther;
					SET_BLACK_AND_PARENT(pParent, pOther);
				}

				return;
			}
		}

		if (pNode)
		{
			SET_BLACK_AND_PARENT(pNode, pParent);
		}
	}

	return;
}
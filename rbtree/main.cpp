#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <chrono>
#include "rbtree.h"

using namespace std;


template< typename _Object > class RBTree;

template< typename _Object >
class RBTreeNode : RB_NODE
{
	friend RBTree<_Object>;

public:

	_Object* Next()
	{
		return (_Object*)::RBTreeNext(this);
	}

	const _Object* Next() const
	{
		return (_Object*)::RBTreeNext(this);
	}

	_Object* Prev()
	{
		return (_Object*)::RBTreePrev(this);
	}

	const _Object* Prev() const
	{
		return (_Object*)::RBTreePrev(this);
	}
};

template< typename _Object >
class RBTree : public RB_TREE
{

public:

	RBTree()
	{
		pRoot = NULL;
	}

	typedef _Object					Object;

	enum
	{
		SUCCESS,
		OBJECT_ALREADY_EXIST,
	};

	Object* LowerBound(Object* pObject) const
	{
		RB_NODE* pResult = NULL;
		RB_NODE* pNode = pRoot;

		while (pNode)
		{
			Object* pRight = (Object*)pNode;

			if (*pObject < *pRight)
			{
				pNode = pNode->pLeft;
			}
			else
			{
				pResult = pNode;
				pNode = pNode->pRight;
			}
		}

		return (Object*)pResult;
	}

	Object* UpperBound(Object* pObject) const
	{
		RB_NODE* pResult = NULL;
		RB_NODE* pNode = pRoot;

		while (pNode)
		{
			Object* pRight = (Object*)pNode;

			if (*pRight < *pObject)
			{
				pNode = pNode->pRight;
			}
			else
			{
				pResult = pNode;
				pNode = pNode->pLeft;
			}
		}

		return (Object*)pResult;
	}

	Object* Find(Object* pObject) const
	{
		RB_NODE* pNode = pRoot;

		while (pNode)
		{
			Object* pRight = (Object*)pNode;

			if (*pObject < *pRight)
			{
				pNode = pNode->pLeft;
			}
			else if (*pRight < *pObject)
			{
				pNode = pNode->pRight;
			}
			else
			{
				return (Object*)pNode;
			}
		}

		return NULL;
	}

#if 0
	//同一値があったときにはじく奴
	int Insert(Object* pObject)
	{
		RB_NODE** pNode = &pRoot;
		RB_NODE* pParent = NULL;

		while (*pNode)
		{
			pParent = *pNode;

			Object* pRight = (Object*)pParent;

			if (*pObject < *pRight)
			{
				pNode = &(*pNode)->pLeft;
			}
			else if (*pRight < *pObject)
			{
				pNode = &(*pNode)->pRight;
			}
			else
			{
				return OBJECT_ALREADY_EXIST;
			}
		}

		*pNode = pObject;

		RBTreeInsert(this, (RB_NODE*)pObject, pParent);
		return SUCCESS;
	}
#endif

	int Insert(Object* pObject)
	{
		RB_NODE** pNode = &pRoot;
		RB_NODE* pParent = NULL;

		while (*pNode)
		{
			pParent = *pNode;

			Object* pRight = (Object*)pParent;

			if (*pObject < *pRight)
			{
				pNode = &(*pNode)->pLeft;
			}
			else
			{
				pNode = &(*pNode)->pRight;
			}
		}

		*pNode = pObject;

		RBTreeInsert(this, (RB_NODE*)pObject, pParent);
		return SUCCESS;
	}

	void Erase(Object* pObject)
	{
		RBTreeErase(this, (RB_NODE*)pObject);
	}

	const Object* First() const { return (_Object*)RBTreeFirst(this); }
	const Object* Last() const { return (_Object*)RBTreeLast(this); }

	Object* First() { return (_Object*)RBTreeFirst(this); }
	Object* Last() { return (_Object*)RBTreeLast(this); }
};

struct Object : public RBTreeNode<Object>
{
	int value;

	int operator < (const Object& right) const
	{
		return value < right.value;
	}
};

constexpr int nMaxNodes = 32 * 1024;
static RBTree<Object> tree;
static Object nodes[nMaxNodes];
static Object* pFreeNodeList;

Object* AllocNode()
{
	Object* pNode = pFreeNodeList;

	if (pNode != NULL)
	{
		Object* pNext = *(Object**)pNode;
		pFreeNodeList = pNext;
	}	

	return pNode;
}

void FreeNode(Object* pObject)
{
	*(Object**)pObject = pFreeNodeList;
	pFreeNodeList = pObject;
}

void InitNodes()
{
	pFreeNodeList = &nodes[0];

	for (int i = 0; i < nMaxNodes-1; ++i)
	{
		*(Object**)&nodes[i] = &nodes[i + 1];
	}

	*(Object**)&nodes[nMaxNodes-1] = NULL;
}

#if defined(_DEBUG) || 1
#define VALIDATE
#endif

#define PROFILE


int main()
{
	std::mt19937 engine(123456U);
	std::uniform_int_distribution<int> distribution;

	InitNodes();

#ifdef VALIDATE
	std::vector<int> values;
	values.reserve(nMaxNodes);
#endif

	int64_t bestTime = INT64_MAX;

#ifdef PROFILE
	for (int loop2 = 0; loop2 < 256; ++loop2)
#else
	for (int loop2 = 0; loop2 < 64; ++loop2)
#endif
	{
		auto start = std::chrono::system_clock::now();
		size_t nNodes = 0;

#ifdef VALIDATE
		for (uint32_t loop = 0; loop < (64 << 10); ++loop)
#else
		for (uint32_t loop = 0; loop < (2 << 20); ++loop)
#endif
		{
			int per = distribution(engine) % 100;

			Object* node = NULL;

			if (per < 55 && (node = AllocNode()) != NULL)
			{
				int v = distribution(engine);
				node->value = v;

#ifdef VALIDATE
				values.insert(std::lower_bound(values.begin(), values.end(), v), v);
#endif

				tree.Insert(node);
				++nNodes;
			}
			else if (nNodes > 0)
			{

#ifdef VALIDATE
				size_t removeIndex = distribution(engine) % values.size();
				int remove = values[removeIndex];
				values.erase(values.begin() + removeIndex);
#else
				int remove = tree.First()->value;
#endif

				Object x;
				x.value = remove;

				node = tree.Find(&x);
				if (node == NULL)
					goto _error;
				tree.Erase(node);
				FreeNode(node);
				--nNodes;
			}

#ifdef VALIDATE
			{
				if(nNodes != values.size())
					goto _error;

				size_t index = 0;

				Object* pNode = tree.First();

				while (pNode)
				{
					if (pNode->value != values[index++])
						goto _error;
					pNode = pNode->Next();
				}

				if (values.size() != index)
					goto _error;

				pNode = tree.Last();

				while (pNode)
				{
					if (pNode->value != values[--index])
						goto _error;
					pNode = pNode->Prev();
				}
			}
#endif
		}

		auto end = std::chrono::system_clock::now();

		auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
		std::cout << ".";
		bestTime = std::min(time, bestTime);

		InitNodes();
		tree.pRoot = NULL;

#ifdef VALIDATE
		values.clear();
#endif		
	}

	std::cout << std::endl;
	std::cout << bestTime << std::endl;

	return 0;

_error:

	std::cout << "Error!!!!" << std::endl;

	return -1;
}



/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EMemoryPool.h: class that implements a memory pool. Pool is expandable, but NOT pointer to other pool SAFE.
// TODO: make EMemoryPool thread safe - fix expandability pointer to pool problem!
/*=============================================================================*/
#ifndef ELITE_MEMORYPOOL
#define ELITE_MEMORYPOOL
#include <stdlib.h>
#include "EMemoryPoolHelpers.h"

namespace Elite
{
	template<class T, class = std::enable_if<std::is_base_of<IPoolable<T>, T>::value>>
	class EMemoryPool final
	{
	public:
		//--- Constructors & Destructors ---
		EMemoryPool() = default;
		~EMemoryPool()
		{ DestroyPool(); }

		//--- Public Functions ---
		//Initialize should be called before using MemoryPool. 
		//This prevents memory pool allocation for local objects that are used as parameters for copying Data in pool
		void InitializePool(unsigned int amount, bool isExpandable = false)
		{
			if (m_IsInitialized)
				return;

			m_TotalAmountUnits = amount;
			m_IsExpandable = isExpandable;
			m_IsInitialized = true;

			//Calculate size of pool
			auto sizePerUnit = sizeof(T);
			auto totalSizePool = amount * sizePerUnit;

			//Allocate pool and prepare datamembers for usage
			m_pMemoryBlock = static_cast<T*>(malloc(totalSizePool));
			m_pFreeHead = m_pMemoryBlock;
		}

		void DestroyPool()
		{
			//Safety, pool has to be initialized first and it should have data!
			if (!m_IsInitialized || !m_pMemoryBlock)
				return;
			//Flush pool to call all Destroy() functions
			Flush();
			//Deallocate pool
			free(m_pMemoryBlock);
			m_pMemoryBlock = nullptr;
		}

		T* GetAvailableUnit()
		{
			//Safety, pool has to be initialized first!
			if (!m_IsInitialized)
				return nullptr;
			//If we currently have reached the limit, check if we are allowed to resize
			if (m_CurrentAmountInUse == m_TotalAmountUnits && !m_IsExpandable)
				return nullptr; //If not, return a nullptr
			else if (m_CurrentAmountInUse == m_TotalAmountUnits && m_IsExpandable)
				ExpandPool(); //Else expand the pool

			//Return available head and adjust variables
			auto pAvailableUnit = m_pFreeHead;
			*pAvailableUnit = {}; //Set data of unit to NULL if you want to
			++m_pFreeHead;
			++m_CurrentAmountInUse;
			return pAvailableUnit;
		}

		//Return pointer to first used unit and the amount of units in use.
		//This can be used to iterate over all the active unites.
		std::vector<T*> GetAllActiveUnits() const
		{
			//Local variables
			std::vector<T*> container = {};
			//Safety, pool has to be initialized first!
			if (!m_IsInitialized)
				return container;
			//Reserve space and copy all pointers in container and return.
			//TODO: optimize this!
			container.reserve(m_CurrentAmountInUse);
			auto tempUnit = m_pMemoryBlock;
			for (unsigned int i = 0; i < m_CurrentAmountInUse; ++i)
			{
				container.push_back(tempUnit);
				++tempUnit;
			}
			return container;
		}

		void Flush()
		{
			//Safety, pool has to be initialized first and it should have data!
			if (!m_IsInitialized || !m_pMemoryBlock)
				return;

			//Keep memoryblock, do not reset units, but reset the variables
			T* pCurrentObj = m_pMemoryBlock;
			for (unsigned int i = 0; i < m_CurrentAmountInUse; ++i)
			{
				//Every T inherits from IPoolable, which should provide implementation Destroy()!
				pCurrentObj->Destroy();
				++pCurrentObj;
			}
			m_pFreeHead = m_pMemoryBlock;
			m_CurrentAmountInUse = 0;
		}

	private:
		//--- Private Functions ---
		void ExpandPool()
		{
			//Safety, pool has to be initialized first and it should have data!
			if (!m_IsInitialized || !m_pMemoryBlock)
				return;

			//Allocate new block of double the size
			auto sizePerUnit = sizeof(T);
			auto newSizePool = (m_TotalAmountUnits * 2) * sizePerUnit;
			auto pNewMemoryBlock = static_cast<T*>(malloc(newSizePool));
			//Copy all data from old block to new block
			auto oldSizePool = m_TotalAmountUnits * sizePerUnit;
			memcpy(pNewMemoryBlock, m_pMemoryBlock, oldSizePool);
			//Free old memory block
			free(m_pMemoryBlock);
			//Set new variables
			m_pMemoryBlock = pNewMemoryBlock;
			m_pFreeHead = m_pMemoryBlock + m_TotalAmountUnits; //Set to end of previous block, starting at the beginning of new segment
			m_TotalAmountUnits *= 2;
		}

		//--- Datamembers ---
		T* m_pMemoryBlock = nullptr;
		T* m_pFreeHead = nullptr;
		unsigned int m_TotalAmountUnits = 0;
		unsigned int m_CurrentAmountInUse = 0;
		bool m_IsExpandable = false;
		bool m_IsInitialized = false;
	};
}
#endif
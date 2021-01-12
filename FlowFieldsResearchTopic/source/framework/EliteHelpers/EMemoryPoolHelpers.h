/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EMemoryPoolHelpers.h: Helpers for the memory pool
/*=============================================================================*/
#ifndef ELITE_MEMORYPOOL_HELPERS
#define ELITE_MEMORYPOOL_HELPERS
namespace Elite
{
	//Each item that can be stored in memorypool should inherit from this baseclass
	template<typename poolableType>
	class IPoolable
	{
	public:
		//Every object that is poolable should also contain an empty default constructor that
		//initializes everything to default values (NULL, nullptr, etc.)

		//No virtual, but no implementation, so you cannot use this as an instance.
		//Every Poolable object has to provide implementation of these functions!
		void Initialize();
		void Destroy();
	};
}
#endif
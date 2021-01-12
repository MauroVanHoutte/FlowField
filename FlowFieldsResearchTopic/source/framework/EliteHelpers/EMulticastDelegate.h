/*=============================================================================*/
// Copyright 2017-2018 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EMulticastDelegate.h: class that implements a "multicast delegate". 
// This can take any functionpointer, store it with variables (either values of references)
// and invoke it. Functions do not return anything (except for the FSM Conditional implementation)
// Multicast Feature: functions don't need to have the same signature
// The Multicast Delegate also has a Conditional variant for the Finite State Machine!
/*=============================================================================*/
#ifndef ELITE_MULTICAST_DELEGATE
#define ELITE_MULTICAST_DELEGATE
namespace Elite
{
	//-----------------------------------------------------------------
	// DELEGATE CONTAINER (BASE)
	//----------------------------------------------------------------
	template<typename ret, typename... args>
	struct MulticastContainer
	{
		std::function<ret(args...)> func;
		std::tuple< args... > arguments;

		explicit MulticastContainer(std::function<ret(args...)> f, args... argumentList) :
			func(f), arguments(std::tuple<args...>(argumentList...)) {}
	};

	//-----------------------------------------------------------------
	// BASES (...)
	//----------------------------------------------------------------
	class MulticastBase
	{
	public:
		virtual ~MulticastBase() {};
		virtual void Invoke() {};
	};

	class MulticastConditionBase
	{
	public:
		virtual ~MulticastConditionBase() {};
		virtual bool Invoke() { return false; };
	};

	template<typename ret, typename... args>
	class MulticastDataBase
	{
	protected:
		std::vector < MulticastContainer<ret, args...>> m_functions;

		template<std::size_t... Is>
		ret CallFunction(std::function<ret(args...)>& func, const std::tuple<args...>& tuple, std::index_sequence<Is...>)
		{
			return func(std::get<Is>(tuple)...);
		}
	};

	//-----------------------------------------------------------------
	// DELEGATE CLASS (DELEGATE BASE)
	//----------------------------------------------------------------
	template<typename... args>
	class Multicast final : public MulticastBase, private MulticastDataBase<void, args...>
	{
	public:
		virtual ~Multicast() {};
		explicit Multicast() {};
		explicit Multicast(std::vector<MulticastContainer<void, args...>> dc)
		{
			for (auto c : dc)
				MulticastDataBase<void, args...>::m_functions.push_back(c);
		}

		void Invoke() override
		{
			for (MulticastContainer<void, args...> f : MulticastDataBase<void, args...>::m_functions)
			{
				CallFunction(f.func, f.arguments, std::index_sequence_for<args...>());
			}
		}

		auto Assign(MulticastContainer<void, args...> dc) -> void
		{
			MulticastDataBase<void, args...>::m_functions.push_back(dc);
		}
	};

	//-----------------------------------------------------------------
	// CONDITION CLASS (DELEGATE)
	//-----------------------------------------------------------------
	template<typename... args>
	class MulticastCondition final : public MulticastConditionBase, private MulticastDataBase<bool, args...>
	{
	public:
		explicit MulticastCondition(std::vector<MulticastContainer<bool, args...>> dc)
		{
			for (auto c : dc)
				MulticastDataBase<bool, args...>::m_functions.push_back(c);
		}
		explicit MulticastCondition() {};
		virtual ~MulticastCondition() {};

		bool Invoke() override
		{
			bool returnValue = 0;
			for (MulticastContainer<bool, args...> f : MulticastDataBase<bool, args...>::m_functions)
			{
				returnValue |= CallFunction(f.func, f.arguments, std::index_sequence_for<args...>());
			}
			return returnValue;
		}

		auto Assign(MulticastContainer<bool, args...> dc) -> void
		{
			MulticastDataBase<bool, args...>::m_functions.push_back(dc);
		}
	};
}
#endif
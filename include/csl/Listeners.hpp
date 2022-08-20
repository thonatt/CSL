#pragma once

#include "AlgebraTypes.hpp"
#include "Controllers.hpp"

#include "Functions.hpp"

#include "Preprocessor.hpp"

#include <iostream>

//to define "not" keyword
#ifdef _WIN32	
#include <iso646.h>
#endif

namespace csl
{
	inline OperatorBase* retrieve_expr(const Expr index)
	{
		if (!index)
			return nullptr;

		if (index.is_static())
			return &ShaderController::get_static_memory()[index];

		return &context::get().m_memory_pool[index];
	}

	inline InstructionBase* retrieve_instruction(const InstructionIndex index)
	{
		return &context::get().m_instruction_pool[index];
	}


	// special listeners

	struct WhileListener
	{
		template<typename T>
		WhileListener(T&& condition)
		{
			context::get().begin_while(std::forward<T>(condition));
		}

		operator bool() const {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}
		mutable bool first = true;

		~WhileListener()
		{
			if (context::active())
				context::get().end_while();
		}
	};

	struct IfListener
	{
		template<typename T>
		IfListener(T&& condition)
		{
			context::get().begin_if(std::forward<T>(condition));
		}

		operator bool() const { return true; }

		~IfListener()
		{
			if (context::active())
				context::get().end_if_sub_block();
		}
	};

	struct ElseListener
	{
		ElseListener()
		{
			context::get().begin_else();
		}

		operator bool() const { return false; }

		~ElseListener() {
			context::get().end_if();
		}
	};

	struct ElseIf
	{
		template<typename T>
		ElseIf(T&& condition)
		{
			context::get().begin_else_if(std::forward<T>(condition));
		}

		operator bool() const { return true; }

		~ElseIf()
		{
			if (context::active())
				context::get().end_if_sub_block();
		}
	};

	struct SwitchListener
	{
		template<typename T>
		SwitchListener(T&& condition) : m_active_listener{ context::active() }
		{
			context::get().begin_switch(std::forward<T>(condition));
		}

		~SwitchListener()
		{
			if (context::active())
				context::get().end_switch();
		}

		operator std::size_t()
		{
			if (!m_active_listener)
				return static_cast<std::size_t>(0);

			++pass_count;
			if (pass_count == 1)
				context::g_context_active = false;
			else if (pass_count == 2)
				context::g_context_active = true;

			return (pass_count > 2) ? static_cast<std::size_t>(0) : unlikely_case;
		}

		const bool m_active_listener;
		std::size_t pass_count = 0;
		static constexpr std::size_t unlikely_case = 13;
	};

	struct ForListener
	{
		ForListener()
		{
			context::get().begin_for_body();
		}

		~ForListener()
		{
			context::get().end_for();
		}

		explicit operator bool()
		{
			if (first) {
				first = false;
				return true;
			}
			return false;
		}

		bool first = true;
	};

	struct ReturnKeyword
	{
		ReturnKeyword()
		{
			if (context::active())
				context::get().add_statement<ReturnStatement>();
		}

		template<typename T>
		ReturnKeyword(T&& t)
		{
			if (context::active())
				context::get().add_statement<ReturnStatement>(std::forward<T>(t));
		}
	};

	template<typename A, typename B, typename C>
	typename Infos<B>::Type ternary(A&& condition, B&& lhs, C&& rhs) {
		//static_assert(SameMat<B, C>, "ternary error");
		//static_assert(SameMat<A, Bool>, "ternary error"); TODO
		return { make_expr<TernaryOperator>(get_expr(std::forward<A>(condition)), get_expr(std::forward<B>(lhs)), get_expr(std::forward<C>(rhs))) };
	}

	///////////////////////////
	// definitions requiring listen() definition

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args)
	{
		if (context::active() && context::shader_active())
			return context::get().m_memory_pool.emplace_back<Operator>(std::forward<Args>(args)...);

		Expr handle = ShaderController::get_static_memory().emplace_back<Operator>(std::forward<Args>(args)...);
		handle.m_id |= ExpressionHandle::Static;
		return handle;
	}

	template <typename ... Args>
	Expr make_funcall(const Op op, Args&& ...args)
	{
		return context::get().m_memory_pool.emplace_back<FunCall<sizeof...(Args)>>(op, std::forward<Args>(args)...);
	}

	inline NamedObjectBase::~NamedObjectBase() {
		if (m_flags & ObjFlags::Constructor && m_flags & ObjFlags::Tracked && !(m_flags & ObjFlags::UsedAsRef) && !(m_flags & ObjFlags::BuiltIn) && !(m_flags & ObjFlags::StructMember))
			if (m_expr && context::active() && context::shader_active())
				safe_static_cast<ConstructorBase*>(retrieve_expr(m_expr))->set_as_unused();
	}

	template<typename T, typename Dimensions, typename Qualifiers, typename ... Args>
	Expr create_variable_expr(const std::string& name, const ObjFlags obj_flags, CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args)
	{
		if (!(obj_flags & ObjFlags::Tracked))
			ctor_flags |= CtorFlags::Untracked;

		const Expr expr = make_expr<Constructor<T, sizeof...(Args), Dimensions, Qualifiers>>(name, ctor_flags, variable_id, std::forward<Args>(args)...);
		if (context::active() && context::shader_active())
			context::get().push_expression(expr);

		return expr;
	}

	template <typename Instruction, typename ... Args>
	InstructionIndex make_instruction(Args&& ...args)
	{
		return context::get().m_instruction_pool.emplace_back<Instruction>(std::forward<Args>(args)...);
	}

	inline void ShaderController::set_current_shader(ShaderController* shader)
	{
		context::g_current_shader = shader;
	}

	inline ShaderController* ShaderController::get_current_shader()
	{
		return context::g_current_shader;
	}

	template<typename ReturnTList, typename ...Fs>
	Function<ReturnTList, Fs...>::Function(const std::string& name, Fs&& ...fs) : FuncBase()
	{
		context::get().begin_func<ReturnTList>(name, NamedObjectBase::id, std::forward<Fs>(fs)...);
		((call_with_only_non_default_args(std::forward<Fs>(fs)), context::get().next_overload()), ...);
		context::get().end_func();
	}


	template<typename ReturnTList, typename ...Fs>
	template<typename ...Args>
	typename Function<ReturnTList, Fs...>:: template ReturnType<Args...>
		Function<ReturnTList, Fs...>::operator()(Args&& ...args)
	{
		using This = Function<ReturnTList, Fs...>;
		using RType = ReturnType<Args...>;

		const Expr expr = make_expr<CustomFunCall<This, RType, sizeof...(Args)>>(This::NamedObjectBase::id, get_expr(std::forward<Args>(args))...);

		// In case return type is void, no variable will be returned, so function call must be explicitely pushed.
		if constexpr (std::is_same_v<RType, void>)
			context::get().push_expression(expr);
		else
			return { expr };
	}

#define CSL_IF(condition) \
	if(csl::IfListener _csl_begin_if_{ condition })

#define CSL_ELSE \
	else {} if(csl::ElseListener _csl_begin_else_{}) {} else 

#define CSL_ELSE_IF(condition) \
	else if(false){} csl::context::get().delay_end_if(); \
	if(false){} else if(csl::ElseIf _csl_begin_else_if_{ condition })

#define CSL_WHILE(condition) \
	for(csl::WhileListener _csl_begin_while_{ condition }; _csl_begin_while_; )

#ifdef __clang__
#define CSL_FOR_ARGS(...) \
	_Pragma("clang diagnostic push")							\
	_Pragma("clang diagnostic ignored \"-Wunused-comparison\"") \
	_Pragma("clang diagnostic ignored \"-Wunused-value\"")		\
	__VA_ARGS__;												\
	_Pragma("clang diagnostic pop")
#else
#define CSL_FOR_ARGS(...) \
	__VA_ARGS__;
#endif 

#define CSL_FOR(...) \
	context::get().begin_for();			\
	context::g_context_active = false;	\
	for( __VA_ARGS__ ){ break; }		\
	context::g_context_active = true;	\
	context::get().begin_for_args();	\
	CSL_FOR_ARGS(__VA_ARGS__);			\
	context::get().begin_for_args();	\
	for(ForListener _csl_begin_for_; _csl_begin_for_; )

#define CSL_BREAK \
	if(false){ break; } \
	csl::context::get().add_statement<SpecialStatement<Break>>();

#define CSL_CONTINUE \
	if(false){ continue; } \
	csl::context::get().add_statement<SpecialStatement<Continue>>();

#define CSL_SWITCH(condition) \
	switch(csl::SwitchListener _csl_begin_switch_{ condition })while(_csl_begin_switch_)

#define CSL_CASE(value) \
	csl::context::get().add_case(value); case value 

#define CSL_DEFAULT \
	csl::context::get().add_case(); default

#define CSL_DISCARD \
	csl::context::get().add_statement<csl::SpecialStatement<Discard>>();

#define CSL_TERNARY(condition, left, right) csl::ternary((condition), (left), (right))

#define CSL_RETURN csl::ReturnKeyword _csl_return_statement_

}

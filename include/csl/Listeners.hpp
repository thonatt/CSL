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

	struct WhileListener {
		operator bool() const {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}
		mutable bool first = true;

		~WhileListener() {
			context::get().end_while();
		}
	};

	struct IfListener {
		operator bool() const { return true; }
		~IfListener() {
			context::get().end_if_sub_block();
		}
	};

	struct ElseListener {
		operator bool() const { return false; }
		~ElseListener() {
			context::get().end_if();
		}
	};

	struct SwitchListener
	{
		SwitchListener(const bool active_listener) : m_active_listener(active_listener) { }

		~SwitchListener() {
			context::get().end_switch();
		}

		operator std::size_t() {
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

	struct ForListener {
		~ForListener() {
			context::get().end_for();
		}

		explicit operator bool() {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}

		bool first = true;
	};

	struct ReturnKeyword {
		ReturnKeyword() {
			context::get().add_statement<ReturnStatement>();
		}
		template<typename T>
		ReturnKeyword(T&& t) {
			context::get().add_statement<ReturnStatement>(std::forward<T>(t));
		}
	};

	template<typename A, typename B, typename C>
	typename Infos<B>::Type _csl_ternary(A&& condition, B&& lhs, C&& rhs) {
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
	context::get().check_end_if(); context::get().begin_if(condition); if(IfListener _csl_begin_if_ = {})

#define CSL_ELSE \
	else {} context::get().begin_else(); if(ElseListener _csl_begin_else_ = {}) {} else 

#define CSL_ELSE_IF(condition) \
	else if(false){} context::get().delay_end_if(); context::get().begin_else_if(condition); if(false) {} else if(IfListener _csl_begin_else_if_ = {})

#define CSL_WHILE(condition) \
	context::get().begin_while(get_expr(std::forward<decltype(condition)>(condition))); for(WhileListener _csl_begin_while_; _csl_begin_while_; )

#define CSL_FOR(...) \
	context::get().begin_for(); context::g_context_active = false; for( __VA_ARGS__ ){ break; } context::g_context_active = true;  \
	context::get().begin_for_args(); __VA_ARGS__;  context::get().begin_for_body(); \
	for(ForListener _csl_begin_for_; _csl_begin_for_; )

#define CSL_BREAK \
	if(false){ break; } \
	context::get().add_statement<SpecialStatement<Break>>();

#define CSL_CONTINUE \
	if(false){ continue; } \
	context::get().add_statement<SpecialStatement<Continue>>();

#define CSL_SWITCH(condition) \
	context::get().begin_switch(condition); switch(SwitchListener _csl_begin_switch_ = { context::g_context_active })while(_csl_begin_switch_)

#define CSL_CASE(value) \
	context::get().add_case(get_expr(std::forward<decltype(value)>(value))); case value 

#define CSL_DEFAULT \
	context::get().add_case({}); default

#define CSL_DISCARD \
	_csl_only_available_in_discard_context_();

#define CSL_TERNARY(...) _csl_ternary( __VA_ARGS__ )

#define CSL_RETURN ReturnKeyword _csl_return_statement_

}

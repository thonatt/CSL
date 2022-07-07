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

namespace csl {

	struct MainListener
	{

		MainListener() {
		}

		bool active = true;

		/////////////////////////////////////////////////

		void push_expression(const Expr ex) {
			if (active && current_shader) {
				current_shader->push_expression(ex);
			}
		}

		/////////////////////////////////////////////////

		template<typename S, typename ... Args>
		void add_statement(Args&& ... args) {
			if (active && current_shader) {
				current_shader->add_statement<S, Args...>(std::forward<Args>(args)...);
			}
		}

		/////////////////////////////////////////////////

		void begin_for() {
			if (current_shader) {
				current_shader->begin_for();
			}
			if (!active) {
				std::cout << "nope";
			}
		}

		virtual void begin_for_args() {
			if (active && current_shader) {
				current_shader->begin_for_args();
			}
		}

		void begin_for_body() {
			if (active && current_shader) {
				current_shader->begin_for_body();
			}
		}
		void end_for() {
			if (active && current_shader) {
				current_shader->end_for();
			}
		}

		void stacking_for_condition(const Expr& expr) {
			if (active && current_shader) {
				current_shader->stacking_for_condition(expr);
			}
		}

		/////////////////////////////////////////////////

		template<typename B, typename = std::enable_if_t< Infos<B>::IsScalar&& Infos<B>::IsBool > >
		void begin_if(B&& condition) {
			if (active && current_shader) {
				current_shader->begin_if(get_expr(std::forward<B>(condition)));
			}
		}

		void begin_else() {
			if (active && current_shader) {
				current_shader->begin_else();
			}
		}

		template<typename B, typename = std::enable_if_t< Infos<B>::IsScalar&& Infos<B>::IsBool > >
		void begin_else_if(B&& condition) {
			if (active && current_shader) {
				current_shader->begin_else_if(get_expr(std::forward<B>(condition)));
			}
		}
		void end_if_sub_block() {
			if (active && current_shader) {
				current_shader->end_if_sub_block();
			}
		}
		void end_if() {
			if (active && current_shader) {
				current_shader->end_if();
			}
		}
		void check_begin_if() {
			if (active && current_shader) {
				current_shader->check_begin_if();
			}
		}
		void delay_end_if() {
			if (active && current_shader) {
				current_shader->delay_end_if();
			}
		}

		/////////////////////////////////////////////////

		template<typename B, typename = std::enable_if_t< SameScalarType<B, bool> > >
		void begin_while(B&& condition) {
			if (active && current_shader) {
				current_shader->begin_while(get_expr(std::forward<B>(condition)));
			}
		}

		void end_while() {
			if (active && current_shader) {
				current_shader->end_while();
			}
		}

		/////////////////////////////////////////////////

		template<typename C, typename = std::enable_if_t< IsInteger<C> > >
		void begin_switch(C&& c) {
			if (active && current_shader) {
				current_shader->begin_switch(get_expr(std::forward<C>(c)));
			}
		}

		template<typename C>
		void begin_switch_case(C&& switch_case) {
			if (active && current_shader) {
				current_shader->add_case(get_expr(std::forward<C>(switch_case)));
			}
		}
		void begin_switch_case() {
			if (active && current_shader) {
				current_shader->add_case(Expr());
			}
		}

		void end_switch() {
			if (active && current_shader) {
				current_shader->end_switch();
			}
		}

		//////////////////////////////

		template<typename Struct>
		void add_struct() {
			if (current_shader) {
				current_shader->add_struct<Struct>();
			}
		}

		template<typename Interface>
		void add_named_interface_block(const std::string& name) {
			if (current_shader) {
				current_shader->add_named_interface_block<Interface>(name);
			}
		}

		template<typename QualifierList, typename TypeList, typename ...Strings>
		void add_unnamed_interface_block(Strings&& ... names) {
			if (current_shader) {
				current_shader->add_unnamed_interface_block<QualifierList, TypeList>(std::forward<Strings>(names)...);
			}
		}

		/////////////////////////////////////////////////

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string& name, const std::size_t fun_id, Fs&& ... fs) {
			if (current_shader) {
				current_shader->begin_func<ReturnTList>(name, fun_id, std::forward<Fs>(fs)...);
			}
		}

		void end_func() {
			if (current_shader) {
				current_shader->end_func();
			}
		}

		void next_overload() {
			if (current_shader) {
				current_shader->next_overload();
			}
		}

		ShaderController* current_shader = nullptr;

	};

	inline MainListener& listen() {
		static MainListener overmind;
		return overmind;
	}

	inline OperatorBase* retrieve_expr(const Expr index)
	{
		if (!index) {
			return nullptr;
		}
		if (index.is_static()) {
			return &ShaderController::get_static_memory()[index];
		}
		return &listen().current_shader->m_memory_pool[index];
	}

	inline InstructionBase* retrieve_instruction(const InstructionIndex index)
	{
		return &listen().current_shader->m_instruction_pool[index];
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
			listen().end_while();
		}
	};

	struct IfListener {
		operator bool() const { return true; }
		~IfListener() {
			listen().end_if_sub_block();
		}
	};

	struct ElseListener {
		operator bool() const { return false; }
		~ElseListener() {
			listen().end_if();
		}
	};

	struct SwitchListener {

		SwitchListener(const bool active_listener) : m_active_listener(active_listener) { }

		~SwitchListener() {
			listen().end_switch();
		}

		operator std::size_t() {
			if (!m_active_listener) {
				return static_cast<std::size_t>(0);
			}

			++pass_count;
			if (pass_count == 1) {
				listen().active = false;
			} else if (pass_count == 2) {
				listen().active = true;
			}
			return (pass_count > 2) ? static_cast<std::size_t>(0) : unlikely_case;
		}

		const bool m_active_listener;
		std::size_t pass_count = 0;
		static constexpr std::size_t unlikely_case = 13;
	};

	struct ForListener {
		~ForListener() {
			listen().end_for();
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
			listen().add_statement<ReturnStatement>();
		}
		template<typename T>
		ReturnKeyword(T&& t) {
			listen().add_statement<ReturnStatement>(std::forward<T>(t));
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
		if (listen().current_shader) {
			return listen().current_shader->m_memory_pool.emplace_back<Operator>(std::forward<Args>(args)...);
		} else {
			Expr handle = ShaderController::get_static_memory().emplace_back<Operator>(std::forward<Args>(args)...);
			handle.m_id |= ExpressionHandle::Static;
			return handle;
		}
	}

	template <typename ... Args>
	Expr make_funcall(const Op op, Args&& ...args)
	{
		return listen().current_shader->m_memory_pool.emplace_back<FunCall<sizeof...(Args)>>(op, std::forward<Args>(args)...);
	}

	inline NamedObjectBase::~NamedObjectBase() {
		if (m_flags & ObjFlags::Constructor && m_flags & ObjFlags::Tracked && !(m_flags & ObjFlags::UsedAsRef) && !(m_flags & ObjFlags::BuiltIn) && !(m_flags & ObjFlags::StructMember)) {
			// TODO: Why is third case necessary?
			if (m_expr && listen().current_shader && !listen().current_shader->m_memory_pool.m_objects_offsets.empty()) {
				safe_static_cast<ConstructorBase*>(retrieve_expr(m_expr))->set_as_unused();
			}
		}
	}

	template<typename T, typename Dimensions, typename Qualifiers, typename ... Args>
	Expr create_variable_expr(const std::string& name, const ObjFlags obj_flags, CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args)
	{
		if (!bool(obj_flags & ObjFlags::Tracked))
			ctor_flags |= CtorFlags::Untracked;

		const Expr expr = make_expr<Constructor<T, sizeof...(Args), Dimensions, Qualifiers>>(name, ctor_flags, variable_id, std::forward<Args>(args)...);
		listen().push_expression(expr);
		return expr;
	}

	template <typename Instruction, typename ... Args>
	InstructionIndex make_instruction(Args&& ...args)
	{
		return listen().current_shader->m_instruction_pool.emplace_back<Instruction>(std::forward<Args>(args)...);
	}

	inline void ShaderController::set_current_shader(ShaderController* shader)
	{
		listen().current_shader = shader;
	}

	inline ShaderController* ShaderController::get_current_shader()
	{
		return listen().current_shader;
	}

	template<typename ReturnTList, typename ...Fs>
	Function<ReturnTList, Fs...>::Function(const std::string& name, Fs&& ...fs) : FuncBase()
	{
		listen().begin_func<ReturnTList>(name, NamedObjectBase::id, std::forward<Fs>(fs)...);
		((call_with_only_non_default_args(std::forward<Fs>(fs)), listen().next_overload()), ...);
		listen().end_func();
	}


	template<typename ReturnTList, typename ...Fs>
	template<typename ...Args>
	typename Function<ReturnTList, Fs...>:: template ReturnType<Args...>
		Function<ReturnTList, Fs...>::operator()(Args&& ...args)
	{
		using This = Function<ReturnTList, Fs...>;
		using RType = ReturnType<Args...>;

		const Expr expr = make_expr<CustomFunCall<This, RType, sizeof...(Args)>>(This::NamedObjectBase::id, get_expr(std::forward<Args>(args))...);

		// in case return type is void, no variable will be returned, so function call must be explicitely sent to the listener
		if constexpr (std::is_same_v<RType, void>) {
			listen().push_expression(expr);
		} else {
			return { expr };
		}
	}

#define CSL_IF(condition) \
	listen().check_begin_if(); listen().begin_if(condition); if(IfListener _csl_begin_if_ = {})

#define CSL_ELSE \
	else {} listen().begin_else(); if(ElseListener _csl_begin_else_ = {}) {} else 

#define CSL_ELSE_IF(condition) \
	else if(false){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(IfListener _csl_begin_else_if_ = {})

#define CSL_WHILE(condition) \
	listen().begin_while(condition); for(WhileListener _csl_begin_while_; _csl_begin_while_; )

#define CSL_FOR(...) \
	listen().begin_for(); listen().active = false; for( __VA_ARGS__ ){ break; } listen().active = true;  \
	listen().begin_for_args(); __VA_ARGS__;  listen().begin_for_body(); \
	for(ForListener _csl_begin_for_; _csl_begin_for_; )

#define CSL_BREAK \
	if(false){ break; } \
	listen().add_statement<SpecialStatement<Break>>();

#define CSL_CONTINUE \
	if(false){ continue; } \
	listen().add_statement<SpecialStatement<Continue>>();

#define CSL_SWITCH(condition) \
	listen().begin_switch(condition); switch(SwitchListener _csl_begin_switch_ = { listen().active })while(_csl_begin_switch_)

#define CSL_CASE(value) \
	listen().begin_switch_case(value); case value 

#define CSL_DEFAULT \
	listen().begin_switch_case(); default

#define CSL_DISCARD \
	_csl_only_available_in_discard_context_();

#define CSL_TERNARY(...) _csl_ternary( __VA_ARGS__ )

#define CSL_RETURN ReturnKeyword _csl_return_statement_

}

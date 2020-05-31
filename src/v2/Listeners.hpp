#pragma once

#include "AlgebraTypes.hpp"
#include "Controllers.hpp"

#include "Functions.hpp"

#include "Debug.hpp"

namespace v2 {

	struct MainListener {

		MainListener() {
		}

		bool active = true;

		/////////////////////////////////////////////////

		void push_expression(const Expr& ex) {
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
			if (active && current_shader) {
				current_shader->begin_for();
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

		template<typename B, typename = std::enable_if_t< Infos<B>::IsScalar && Infos<B>::IsBool > >
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

		template<typename B, typename = std::enable_if_t< Infos<B>::IsScalar && Infos<B>::IsBool > >
		void begin_else_if(B&& condition) {
			if (current_shader) {
				current_shader->begin_else_if(get_expr(std::forward<B>(condition)));
			}
		}
		void end_if_sub_block() {
			if (current_shader) {
				current_shader->end_if_sub_block();
			}
		}
		void end_if() {
			if (current_shader) {
				current_shader->end_if();
			}
		}
		void check_begin_if() {
			if (current_shader) {
				current_shader->check_begin_if();
			}
		}
		void delay_end_if() {
			if (current_shader) {
				current_shader->delay_end_if();
			}
		}

		/////////////////////////////////////////////////

		template<typename B, typename = std::enable_if_t< SameScalarType<B, bool> > >
		void begin_while(B&& condition) {
			if (current_shader) {
				current_shader->begin_while(get_expr(std::forward<B>(condition)));
			}
		}

		void end_while() {
			if (current_shader) {
				current_shader->end_while();
			}
		}

		/////////////////////////////////////////////////

		template<typename C, typename = std::enable_if_t< IsInteger<C> > >
		void begin_switch(C&& c) {
			if (current_shader) {
				current_shader->begin_switch(get_expr(std::forward<C>(c)));
			}
		}

		template<typename C>
		void begin_switch_case(C&& switch_case) {
			if (current_shader) {
				current_shader->add_case(get_expr(std::forward<C>(switch_case)));
			}
		}
		void begin_switch_case() {
			if (current_shader) {
				current_shader->add_case(Expr());
			}
		}

		void end_switch() {
			if (current_shader) {
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
		void add_unnamed_interface_block() {
			if (current_shader) {
				current_shader->add_unnamed_interface_block<Interface>();
			}
		}

		/////////////////////////////////////////////////

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::size_t fun_id, Fs&& ... fs) {
			if (current_shader) {
				current_shader->begin_func<ReturnTList>(fun_id, std::forward<Fs>(fs)...);
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

		ShaderController::Ptr current_shader;

	};

	inline MainListener& listen() {
		static MainListener overmind;
		return overmind;
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
		~SwitchListener() {
			listen().end_switch();
		}

		operator std::size_t() const {
			++count;
			if (count == 1) {
				listen().active = false;
			} else if (count == 2) {
				listen().active = true;
			}
			return (count > 2) ? static_cast<std::size_t>(0) : unlikely_case;
		}

		mutable std::size_t count = 0;
		static constexpr std::size_t unlikely_case = 13;
	};

	struct ForListener {
		~ForListener() {
			listen().end_for();
		}

		explicit operator bool() const {
			if (first) {
				first = false;
				return true;
			}
			return false;
		}

		mutable bool first = true;
	};

	/////////////////////////////
	// definitions requiring listen() definition

	template<typename T, typename ... Args>
	Expr create_variable_expr(const ObjFlags obj_flags, const CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args)
	{
		Expr expr = make_expr<ConstructorWrapper>(ConstructorWrapper::create<T>(ctor_flags, variable_id, std::forward<Args>(args)...));
		if (!(obj_flags & ObjFlags::StructMember)) {
			listen().push_expression(expr);
		}
		return expr;
	}

	template<typename ReturnTList, typename ...Fs>
	Function<ReturnTList, Fs...>::Function(const std::string& name, Fs&& ...fs) : FuncBase(name)
	{
		listen().begin_func<ReturnTList>(NamedObjectBase::id, std::forward<Fs>(fs)...);
		((call_with_only_non_default_args(std::forward<Fs>(fs)), listen().next_overload()), ...);
		listen().end_func();
	}


	template<typename ReturnTList, typename ...Fs>
	template<typename ...Args>
	typename Function<ReturnTList, Fs...>:: template ReturnType<Args...>
		Function<ReturnTList, Fs...>::operator()(Args&& ...args)
	{
		using This = Function<ReturnTList, Fs...>;

		const Expr expr = make_expr<CustomFunCall<This, sizeof...(Args)>>(This::NamedObjectBase::id, get_expr(std::forward<Args>(args))...);

		//in case return type is void, no variable will be returned, so function call must be explicitely sent to the listener
		if constexpr (std::is_same_v<ReturnType<Args...>, void>) {
			listen().push_expression(expr);
		} else {
			return ReturnType<Args...>(expr);
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
	listen().add_statement<BreakStatement>();

#define CSL_CONTINUE \
	if(false){ continue; } \
	listen().add_statement<ContinueStatement>();

#define CSL_SWITCH(condition) \
	listen().begin_switch(condition); switch(SwitchListener _csl_begin_switch_ = {})while(_csl_begin_switch_)

#define CSL_CASE(value) \
	listen().begin_switch_case(value); case value 

#define CSL_DEFAULT \
	listen().begin_switch_case(); default

}

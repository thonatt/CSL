#pragma once

#include "AlgebraTypes.hpp"
#include "Controllers.hpp"

#include "NamedObjects.hpp"
#include "Types.hpp"

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

		template<typename B, typename = std::enable_if_t< SameScalarType<B,bool> > >
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

		template<typename B, typename = std::enable_if_t< SameScalarType<B, bool> > >
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
		void begin_func(const std::string& name, Fs&& ... fs) {
			if (current_shader) {
				current_shader->begin_func<ReturnTList>(name, std::forward<Fs>(fs)...);
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


	/////////////////////////////

	template<typename T, typename ... Args>
	Expr create_variable_expr(const ObjFlags obj_flags, const CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args)
	{
		Expr expr = make_expr<ConstructorWrapper>(ConstructorWrapper::create<T>(ctor_flags, variable_id, std::forward<Args>(args)...));
		if (!(obj_flags & ObjFlags::StructMember)) {
			listen().push_expression(expr);
		}
		return expr;
	}

}

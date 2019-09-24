#pragma once

#include "Controllers.hpp"
#include "FunctionHelpers.hpp"
#include "Matrix.hpp"

#define EX(type, var) getExp(std::forward<type>(var))

namespace csl {

	struct MainListener {

		MainListener() {
			//shader = std::make_shared<TShader>();
			//currentShader = shader;
		}

		void add_blank_line(int n = 0) {
			if (currentShader) {
				currentShader->add_blank_line(n);
			}
		}

		/////////////////////////////////////////////////

		void addEvent(const Ex & ex) {
			if (currentShader && active()) {
				currentShader->handleEvent(ex);
			}
		}

		/////////////////////////////////////////////////

		//void add_return_statement(const Ex & ex) {
		//	if (currentShader) {
		//		currentShader->add_return_statement(ex);
		//	}
		//}

		template<typename S, typename ... Args>
		void add_statement(Args && ... args) {
			if (currentShader && active()) {
				currentShader->add_statement<S, Args...>(std::forward<Args>(args)...);
			}
		}


		/////////////////////////////////////////////////

		void begin_for() {
			if (currentShader) {
				currentShader->begin_for();
			}
		}

		virtual void begin_for_args() {
			if (currentShader) {
				currentShader->begin_for_args();
			}
		}

		void begin_for_body() {
			if (currentShader) {
				currentShader->begin_for_body();
			}

		}
		void end_for() {
			if (currentShader) {
				currentShader->end_for();
			}
		}

		void stack_for_condition(const Ex & ex) {
			if (currentShader) {
				currentShader->stack_for_condition(ex);
				//std::cout << "stacked for arg : " << ex->str(0) << std::endl;
			}
		}

		/////////////////////////////////////////////////

		template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
		void begin_if(B && b) {
			if (currentShader) {
				currentShader->begin_if(EX(B, b));
			}
		}
		void begin_else() {
			if (currentShader) {
				currentShader->begin_else();
			}
		}
		template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
		void begin_else_if(B && b) {
			if (currentShader) {
				currentShader->begin_else_if(EX(B, b));
			}
		}
		void end_if_sub_block() {
			if (currentShader) {
				currentShader->end_if_sub_block();
			}
		}
		void end_if() {
			if (currentShader) {
				currentShader->end_if();
			}
		}
		void check_begin_if() {
			if (currentShader) {
				currentShader->check_begin_if();
			}
		}
		void delay_end_if() {
			if (currentShader) {
				currentShader->delay_end_if();
			}
		}

		/////////////////////////////////////////////////

		template<typename B, typename = std::enable_if_t< EqualType<B, Bool> > >
		void begin_while(B && b) {
			if (currentShader) {
				currentShader->begin_while(EX(B, b));
			}
		}

		void end_while() {
			if (currentShader) {
				currentShader->end_while();
			}
		}

		/////////////////////////////////////////////////

		template<typename C, typename = std::enable_if_t< IsInteger<C> > >
		void begin_switch(C && cond) {
			if (currentShader) {
				currentShader->begin_switch(EX(C, cond));
			}
		}

		template<typename C>
		void begin_switch_case(C && _case) {
			if (currentShader) {
				currentShader->add_case(EX(C, _case));
			}
		}
		void begin_switch_case() {
			if (currentShader) {
				currentShader->add_case(Ex());
			}
		}


		void end_switch() {
			if (currentShader) {
				currentShader->end_switch();
			}
		}

		//////////////////////////////

		template<bool dummy, typename ...Args, typename ... Strings>
		void add_struct(const std::string & name, const Strings & ... names) {
			if (currentShader) {
				currentShader->add_struct<Args...>(name, names...);
			}
		}

		template<typename T, typename ...Args, typename ... Strings>
		void add_unnamed_interface_block(const std::string & dummy, Strings & ... names) {
			if (currentShader) {
				currentShader->add_unnamed_interface_block<T, Args...>(names...);
			}
		}

		/////////////////////////////////////////////////

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string & name, const Fs & ... fs) {
			if (currentShader) {
				currentShader->begin_func<ReturnTList>(name, fs...);
			}
		}

		void end_func() {
			if (currentShader) {
				currentShader->end_func();
			}
		}

		void next_overload() {
			if (currentShader) {
				currentShader->next_overload();
			}
		}

		/////////////////////////////////////////////////

		void explore() {
			if (currentShader) {
				std::cout << "############## explore ############" << std::endl;
				currentShader->explore();
				std::cout << "###################################" << std::endl;
			}
		}

		void reset_counters() {
			for (CounterData * counter_ptr : counter_ptrs) {
				*counter_ptr = CounterData();
			}
			counter_ptrs.clear();
		}

		void add_active_counter(CounterData & counter) {
			counter_ptrs.insert(&counter);
		}

		bool & active() { return isListening; }

		//TShader::Ptr shader;
		ShaderBase::Ptr currentShader;

		std::set<CounterData *> counter_ptrs;
		bool isListening = true;
	};

	inline MainListener & listen() {
		static MainListener overmind;
		return overmind;
	}

	template<NamingCaterogy cat>
	inline std::string NamingCounter<cat>::getNextName()
	{
		std::string out = baseName() + std::to_string(counterData.value);
		++counterData.value;

		if (!counterData.is_tracked) {
			listen().add_active_counter(counterData);
			counterData.is_tracked = true;
		}

		return out;
	}

	template<typename T>
	inline void NamedObject<T>::checkName() {

		using AutoName = typename AutoNaming<T>::Type;
		if (*namePtr == "") {
			namePtr = std::make_shared<std::string>(AutoName::getNextName());
		}

	}

	template<typename T, typename ... Args>
	Ex createInit(const stringPtr & name, CtorStatus status, uint ctor_flags, const Args &... args)
	{
		//std::cout << "ctor : " << *name << " " << (bool)(ctor_flags & PARENTHESIS) << std::endl;
		auto ctor = std::make_shared<Constructor<T, sizeof...(args)>>(name, status, ctor_flags, args...);
		Ex expr = std::static_pointer_cast<OperatorBase>(ctor);
		if (!(ctor_flags & IS_BASE)) {
			listen().addEvent(expr);
		}
		return expr;
	}

	template<typename T, typename ... Args>
	Ex createDeclaration(const stringPtr & name, uint flags, const Args &... args) {
		return createInit<T, Args...>(name, DECLARATION, flags, args...);
	}

	struct BeginWhile {
		operator bool() const {
			//std::cout << " BeginWhile operator bool() const " << first << std::endl;
			if (first) {
				first = false;
				return true;
			}
			return false;
		}
		mutable bool first = true;

		~BeginWhile() {
			listen().end_while();
		}
	};

	struct BeginIf {
		operator bool() const { return true; }
		~BeginIf() {
			listen().end_if_sub_block();
		}
	};

	struct BeginElse {
		operator bool() const { return false; }
		~BeginElse() {
			listen().end_if();
		}
	};

	struct BeginSwitch {
		~BeginSwitch() {
			listen().end_switch();
		}
		
		operator int() const {
			++count;
			if (count == 1) {
				listen().active() = false;
			} else if (count == 2) {
				listen().active() = true;
			}
			return count > 2 ? 0 : unlikely_case;
		}

		mutable int count = 0;
		int unlikely_case = 696969;
	};

	struct EndFor {
		~EndFor() {
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

	inline void lineBreak(int n = 1) { listen().add_blank_line(n); }

	template<typename F, typename ...Fs >
	struct CallFuncs<TList<F, Fs...>> {
		static void run(const F & f, const Fs & ... fs) {
			call_with_only_non_default_args(f);
			listen().next_overload();
			CallFuncs<TList<Fs...>>::run(fs...);
		}
	};

	template<typename ReturnTList, typename ... Fs>
	void init_function_declaration(const std::string & fname, const Fs & ...fs)
	{
		listen().begin_func<ReturnTList>(fname, fs...);
		//std::cout << "\t" << fname << std::endl;
		CallFuncs<TList<Fs...>>::run(fs...);
		listen().end_func();
	}

	template<typename ReturnTList, typename FuncTList> template<typename ... Args>
	typename Function<ReturnTList, FuncTList>::template ReturnType<Args...>
		Function<ReturnTList, FuncTList>::operator()(Args && ...args) {

		//in case return type is void, no variable will be returned, so function call must be explicitely sent to the listener
		if (std::is_same<ReturnType<Args...>, void>::value) {
			listen().addEvent(createFCallExp(str(), EX(Args, args)...));
		}

		return ReturnType<Args...>(createFCallExp(str(), EX(Args, args)...));
	}

	inline MatrixConvertor<Bool>::operator bool() &
	{
		//needed as any [variable;] in GL_FOR wont generate any instruction
		//listen().stack_for_condition(NamedObjectBase::getExRef());

		listen().stack_for_condition(static_cast<Bool &>(*this).getExRef());
		return false;
	}

	struct ReturnKeyword {
		ReturnKeyword() {
			listen().add_statement<ReturnStatement>();
		}
		template<typename T>
		ReturnKeyword(T && t) {
			listen().add_statement<ReturnStatement>(EX(T, t));
		}
	};

#define GL_RETURN ReturnKeyword csl_return_statement

#define GL_FOR(...) \
	listen().begin_for(); listen().active() = false; for( __VA_ARGS__ ){break;}  listen().active() = true;  \
	listen().begin_for_args(); __VA_ARGS__;  listen().begin_for_body(); \
	for(EndFor csl_dummy_for; csl_dummy_for; )

#define GL_IF(condition) \
	listen().check_begin_if(); listen().begin_if(condition); if(BeginIf csl_begin_if = {})

#define GL_ELSE \
	else {} listen().begin_else(); if(BeginElse csl_begin_else = {}) {} else 

#define GL_ELSE_IF(condition) \
	else if(false){} listen().delay_end_if(); listen().begin_else_if(condition); if(false) {} else if(BeginIf csl_begin_else_if = {})

#define GL_CONTINUE \
	listen().add_statement<ContinueStatement>();

#define GL_DISCARD \
	listen().add_statement<DiscardStatement>();

#define GL_BREAK \
	if(false){break;} listen().add_statement<BreakStatement>();

#define GL_WHILE(condition) \
	listen().begin_while(condition); for(BeginWhile csl_begin_while = {}; csl_begin_while; )

#define GL_SWITCH(condition) \
	listen().begin_switch(condition); switch(BeginSwitch csl_begin_switch = {})while(csl_begin_switch)

#define GL_CASE(value) \
	listen().begin_switch_case(value); case value 

#define GL_DEFAULT \
	listen().begin_switch_case(); default

	template<typename B, typename A, typename C, typename I = Infos<A>, typename = std::enable_if_t<
		EqualMat<B, Bool> && EqualMat<A, C>
		>> Matrix< I::scalar_type, I::rows, I::cols > csl_ternary(B && condition, A && lhs, C && rhs) {
		return { createExp<Ternary>(EX(B,condition), EX(A,lhs), EX(C,rhs)) };
	}

#define GL_TERNARY(...) csl_ternary( __VA_ARGS__ )

}

#undef EX
#pragma once

#include <string>
#include <tuple>

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include "Context.h"
#include "Algebra.h"
#include "MatrixTypesTest.h"

// Functions

template<typename T> struct FunctionReturnType {
	using type = void;
};

template<typename ReturnType, typename Fun, typename... Args>
struct FunctionReturnType< ReturnType(Fun::*)(Args...) const> {
	using type = std::function<ReturnType(Args...)>;
	using RType = ReturnType;
};


template<typename Lambda> typename FunctionReturnType<decltype(&Lambda::operator())>::type
functionFromLambda(const Lambda &func) { 
	return func;
}

template<typename R, typename F> struct PlugType {
	using type = void;
};
template<typename R, typename ReturnType, typename Fun, typename... Args>
struct PlugType<R, ReturnType(Fun::*)(Args...) const> {
	using type = std::function<R(Args...)>;
};

template<typename R, typename Lambda>
typename PlugType<R, decltype(&Lambda::operator())>::type plugType(const Lambda & f) {
	return {};
}

template<typename R, template<typename...> class Params, typename... Args, std::size_t... I>
R call_from_tuple_helper(std::function<R(Args...)> const&func, Params<Args...> const&params, std::index_sequence<I...>)
{
	return func(std::get<I>(params)...);
}

template<typename R, template<typename...> class Params, typename... Args>
R call_from_tuple(const std::function<R(Args...)> &func, const Params<Args...> &params)
{
	return call_from_tuple_helper(func, params, std::index_sequence_for<Args...>{});
}

template <typename... Args>
void call(const std::string & fname, const std::function<void(Args...)>& f) {
	auto args = Ctx().begin_function_declaration<void,Args...>(fname);
	call_from_tuple(f, args);
	Ctx().end_function_declaration();
}

template <typename ReturnType, typename... Args>
void call(const std::string & fname, const std::function<ReturnType(Args...)>& f) {
	auto args = Ctx().begin_function_declaration<ReturnType,Args...>(fname);
	ReturnType out = call_from_tuple(f, args);
	Ctx().end_function_declaration(out);
}

template <typename String, typename... Args, typename... Strings >
void call(const std::string & fname, const std::function<void(Args...)>& f, const std::tuple<String, Strings...>& argnames) {
	auto args = Ctx().begin_function_declaration<void,Args...>(fname, argnames);
	call_from_tuple(f, args);
	Ctx().end_function_declaration();
}

template < typename ReturnType, typename String , typename... Args, typename... Strings >
void call(const std::string & fname, const std::function<ReturnType(Args...)>& f , const std::tuple<String, Strings...>& argnames) {
	auto args = Ctx().begin_function_declaration<ReturnType,Args...>(fname, argnames);
	ReturnType out = call_from_tuple(f, args);
	Ctx().end_function_declaration(out);
}

template<typename F_Type> struct Fun : public NamedObject<Fun<F_Type>> {
	
	using FunctionType = F_Type;
	using NamedObject<Fun<F_Type>>::name;

	Fun(const std::string & _name, const F_Type  & _f) : NamedObject<Fun<F_Type>>(_name), f(_f) {
		call(name, functionFromLambda(f));
	}

	template<typename String, typename ... Strings>
	Fun(const std::string & _name, const F_Type  & _f, String && _arg, Strings && ... _argnames ) : NamedObject<Fun<F_Type>>(_name), f(_f) {
		std::tuple<String,Strings...> tup(_arg,_argnames...);
		call(name, functionFromLambda(f), tup);
	}

	template<typename ...Args> const typename std::result_of_t<F_Type(Args...)> operator()(const Args &  ... args) {
		using ReturnType = typename std::result_of_t<F_Type(Args...)>;
		std::string s = name + "(" + strFromObj(args...) + ")";
		return createDummy<ReturnType>(s);
	}

	F_Type f;

};

template<typename F_Type> Fun<F_Type> makeF(const std::string & name , const F_Type & f) {
	return Fun<F_Type>(name, f);
}

template<typename F_Type, typename ... Strings > Fun<F_Type> makeF(const std::string & name, const F_Type & f, const Strings & ...argnames ) {
	return Fun<F_Type>(name, f, argnames...);
}

//////////////////////////////

struct EndFor {
	~EndFor() {
		Ctx().end_for_body();
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

#define GL_FOR(...) Ctx().enable() = false; for( __VA_ARGS__ ){break;} Ctx().enable() = true; \
Ctx().begin_for_declaration(); __VA_ARGS__; Ctx().end_for_declaration();  \
for(EndFor dummy_loop; dummy_loop; )

void Break() { Ctx().declare_break(); }
void Continue() { Ctx().declare_continue(); }

#define GL_BREAK Break()
#define GL_CONTINUE Break()
#define GL_RETURN return

void begin_if(const Bool& b) {
	release(b);
	Ctx().begin_if(b);
}

void begin_if(bool) = delete;

void begin_else_if(const Bool& b) {
	release(b);
	Ctx().begin_else_if(b);
}
void begin_else_if(bool) = delete;

struct BeginIF {
	operator bool() const {
		return true;
	}
	~BeginIF() { Ctx().end_if(); }
};

struct BeginELSE {
	operator bool() const {
		return false;
	}
	~BeginELSE() { Ctx().end_if(); }

};
#define GL_IF(condition) begin_if(condition); if(BeginIF obj = BeginIF())

#define GL_ELSE else {} Ctx().begin_else(); if(BeginELSE obj = BeginELSE()) {} else 

#define GL_ELSE_IF(condition) else if(true){} begin_else_if(condition); if(false) {} else if(BeginIF obj = BeginIF())

template<typename A, typename B, typename = std::enable_if_t<EqualDim<A,B>> > 
MatrixType<A> ternary(const Bool &b, const A& expa, const B& expb) {
	return createDummy<MatrixType<A>>("( " + getName(b) + " ? " + getName(expa) + " : " + getName(expb) + " )");
}

#define GL_TERNARY(a,b,c) ternary(a,b,c)

//void begin_switch() { std::cout << " switch( ) {" << std::endl; }
//void begin_switch(int i) = delete;
//void end_switch() { std::cout << " } " << std::endl; }
//void begin_case(const std::string & s) {
//
//}
//struct BeginSWITCH {
//	int counter = 0;
//	operator int() const {
//		return 0;
//	}
//	~BeginSWITCH() { end_switch(); }
//};


//#define GL_SWITCH(condition) begin_switch(); switch(BeginSWITCH obj = BeginSWITCH() )
//#define GL_CASE(c) case c : std::cout << " case " << BOOST_PP_STRINGIZE(c) << " : ";
//#define GL_DEFAULT default : std::cout << " default : "; 

//# define GL_SWITCH switch
//# define GL_CASE case
//# define GL_DEFAULT default

#define REM(...) __VA_ARGS__
#define EAT(...)
// Strip off the type
#define STRIP(x) EAT x		
// Show the type without parenthesis
#define PAIR(x) REM x

#define GL_STRUCT(StructTypename,...)  \
struct StructTypename : public NamedObject<StructTypename> {	\
	BOOST_PP_SEQ_FOR_EACH_I(DECLARE_MEMBER, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	using NamedObject<StructTypename>::name;\
	\
	StructTypename(const std::string & _name = "") : NamedObject<StructTypename>(_name)  {  \
		if (_name != "") { released = true; } \
		Ctx().removeCmd(numMembers()); \
		Ctx().addCmd(typeStr() + " " + name + "; ");  \
		BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	} \
	static const int numMembers() { return BOOST_PP_VARIADIC_SIZE(__VA_ARGS__); } \
	static const std::string typeStr() { return std::string(#StructTypename); } \
	static const std::string structDeclaration() { return "struct " + typeStr() + \
		" { \n" + BOOST_PP_SEQ_FOR_EACH_I(DECLARE_MEMBER_STR, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) + "} \n"; } \
}; \
Ctx().addStruct( StructTypename::structDeclaration())	\

#define DECLARE_MEMBER(r, data, i, elem) PAIR(elem); //static_assert(std::is_base_of_v<NamedObject<decltype(STRIP(elem))>, decltype(STRIP(elem))>, "non GL type"); 
#define DECLARE_MEMBER_STR(r, data, i, elem) "   " + strFromType<BOOST_PP_SEQ_HEAD(elem)>() + " " + std::string(BOOST_PP_STRINGIZE(STRIP(elem))) + "; \n" +  

#define INIT_MEMBER_PARENT(r, data, i, elem) getParent(STRIP(elem)) = this;  getBaseName(STRIP(elem)) = BOOST_PP_STRINGIZE(STRIP(elem)); 

template<typename ... Args> struct GG {};

#define GL_STRUCT_T(StructTypename,...)  \
\
listen().add_struct<true BOOST_PP_SEQ_FOR_EACH_I(MEMBER_TYPE_T, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) >(std::string(#StructTypename) \
			BOOST_PP_SEQ_FOR_EACH_I(MEMBER_STR_T, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))); \
\
struct StructTypename : public NamedObjectT<StructTypename> {	\
	BOOST_PP_SEQ_FOR_EACH_I(DECLARE_MEMBER, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	\
	StructTypename(const std::string & _name = "", NamedObjectBaseT * _parent = nullptr ) : NamedObjectT<StructTypename>(_name, _parent) \
		 BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT_T, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	{  \
		if(!_parent) { exp = createDeclaration<StructTypename>(myNamePtr()); }\
	} \
	static const std::string typeStr() { return std::string(#StructTypename); } \
} \

#define INIT_MEMBER_PARENT_T(r, data, i, elem) , STRIP(elem)(BOOST_PP_STRINGIZE(STRIP(elem)), this) 
#define MEMBER_TYPE_T(r, data, i, elem) , BOOST_PP_SEQ_HEAD(elem)
#define MEMBER_STR_T(r, data, i, elem) , std::string(BOOST_PP_STRINGIZE(STRIP(elem)))

//static const int numMembers() { return BOOST_PP_VARIADIC_SIZE(__VA_ARGS__); } \
//static const std::string typeStr() { return std::string(#StructTypename); } \
//static const std::string structDeclaration() {
//	return "struct " + typeStr() + \
//		" { \n" + BOOST_PP_SEQ_FOR_EACH_I(DECLARE_MEMBER_STR, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) + "} \n";
//} \
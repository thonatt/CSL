#pragma once

#include <string>
#include <tuple>

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include "ExpressionsTest.h"

#define REM(...) __VA_ARGS__
#define EAT(...)
// Strip off the type
#define STRIP(x) EAT x		
// Show the type without parenthesis
#define PAIR(x) REM x

#define DECLARE_MEMBER(r, data, i, elem) PAIR(elem);
#define INIT_MEMBER_PARENT(r, data, i, elem) , STRIP(elem)(BOOST_PP_STRINGIZE(STRIP(elem)), IS_TRACKED, this) 
#define INIT_MEMBER_PARENT_FROM_EXP(r, data, i, elem) , STRIP(elem)(BOOST_PP_STRINGIZE(STRIP(elem)), TRACKED, this, true) 
#define INIT_MEMBER_PARENT_NEW(r, data, i, elem) , STRIP(elem)( \
	createExp<MemberAccessor>(getExRef(), std::make_shared<std::string>(BOOST_PP_STRINGIZE(STRIP(elem)))), 0, ALWAYS_EXP) 
#define MEMBER_TYPE(r, data, i, elem) , BOOST_PP_SEQ_HEAD(elem)
#define MEMBER_STR(r, data, i, elem) , std::string(BOOST_PP_STRINGIZE(STRIP(elem)))

#define GL_STRUCT(StructTypename,...)  \
\
listen().add_struct<true BOOST_PP_SEQ_FOR_EACH_I(MEMBER_TYPE, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) >(std::string(#StructTypename) \
			BOOST_PP_SEQ_FOR_EACH_I(MEMBER_STR, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))); \
\
struct StructTypename : public NamedObject<StructTypename> { \
	using UnderlyingType = StructTypename;	\
	\
	BOOST_PP_SEQ_FOR_EACH_I(DECLARE_MEMBER, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	\
	StructTypename(const std::string & _name = "", uint _flags = IS_TRACKED) \
		: NamedObject<StructTypename>(_name, _flags) \
		BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT_NEW, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	{ \
	} \
	\
	StructTypename(const Ex & _ex, uint ctor_flags = 0,	uint obj_flags = IS_TRACKED, const std::string & s = "") \
		: NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
		BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT_NEW, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	{ \
	} \
	\
	static const std::string typeStr() { return std::string(#StructTypename); } \
}

//if (!_parent) { isUsed = true;  if (_track) { exp = createDeclaration<StructTypename>(myNamePtr()); } } \
//Matrix(const Ex & _exp) : NamedObject<Matrix>() {
//	//std::cout << " from exp " << std::endl;
//	exp = createInit<Matrix, HIDE, NO_PARENTHESIS>(namePtr, _exp);
//	//exp = _exp;
//	isUsed = false;
//}
#pragma once

#include <string>
#include <tuple>

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>

#include "ExpressionsTest.h"

// http://pfultz2.com/blog/2012/07/31/reflection-in-under-100-lines/

#define REM(...) __VA_ARGS__
#define EAT(...)
// Strip off the type
#define STRIP(x) EAT x		
// Show the type without parenthesis
#define PAIR(x) REM x

#define MEMBER_TYPE(elem) BOOST_PP_SEQ_HEAD(elem)
#define MEMBER_NAME(elem) STRIP(elem)
#define MEMBER_STR(elem) BOOST_PP_STRINGIZE(MEMBER_NAME(elem))

#define MEMBER_TYPE_IT(r, data, i, elem) , MEMBER_TYPE(elem)
#define MEMBER_STR_IT(r, data, i, elem) , MEMBER_STR(elem)
#define MEMBER_ARG_IT(r, data, i, elem) MEMBER_TYPE(elem) && MEMBER_NAME(elem),
#define MEMBER_ARG_EX_IT(r, data, i, elem) , EX( MEMBER_TYPE(elem), MEMBER_NAME(elem))

#define DECLARE_MEMBER_IT(r, data, i, elem) PAIR(elem);
#define INIT_MEMBER_PARENT_IT(r, data, i, elem) , MEMBER_NAME(elem)( \
	createExp<MemberAccessor>(getExRef(), std::make_shared<std::string>(MEMBER_STR(elem))), 0, ALWAYS_EXP) 

#define MEMBER_ITERATE(macro, ...) BOOST_PP_SEQ_FOR_EACH_I(macro, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define GL_STRUCT(StructTypename,...)  \
\
listen().add_struct<true MEMBER_ITERATE(MEMBER_TYPE_IT, __VA_ARGS__) >(BOOST_PP_STRINGIZE(StructTypename) \
			MEMBER_ITERATE(MEMBER_STR_IT, __VA_ARGS__) ); \
\
struct StructTypename : public NamedObject<StructTypename> { \
	using UnderlyingType = StructTypename;	\
	\
	MEMBER_ITERATE(DECLARE_MEMBER_IT, __VA_ARGS__) \
	\
	StructTypename(const std::string & _name = "", uint _flags = IS_TRACKED) \
		: NamedObject<StructTypename>(_name, _flags) \
		MEMBER_ITERATE(INIT_MEMBER_PARENT_IT, __VA_ARGS__) \
	{ \
	} \
	\
	StructTypename(const Ex & _ex, uint ctor_flags = 0,	uint obj_flags = IS_TRACKED, const std::string & s = "") \
		: NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
		MEMBER_ITERATE(INIT_MEMBER_PARENT_IT, __VA_ARGS__) \
	{ \
	} \
    \
	StructTypename(MEMBER_ITERATE(MEMBER_ARG_IT,__VA_ARGS__) bool dummy = false) \
		: NamedObject<StructTypename>(DISPLAY_TYPE | PARENTHESIS, IS_TRACKED, "" MEMBER_ITERATE(MEMBER_ARG_EX_IT, __VA_ARGS__)) \
		MEMBER_ITERATE(INIT_MEMBER_PARENT_IT, __VA_ARGS__) \
	{ \
	} \
	\
	static std::string typeStr(int trailing = 0) { return std::string(#StructTypename); } \
}

#define GL_INTERFACE(Qualifier, Name, ...) \
struct Name : public NamedObject<Name> { \
	using UnderlyingType = Name;	\
	\
	MEMBER_ITERATE(DECLARE_MEMBER_IT, __VA_ARGS__) \
	\
	Name(const std::string & _name = "", uint _flags = IS_TRACKED) \
		: NamedObject<Name>(_name, _flags) \
		MEMBER_ITERATE(INIT_MEMBER_PARENT_IT, __VA_ARGS__) \
	{ \
	} \
	\
	static std::string typeStr(int trailing = 0) { \
		return InterfaceDeclaration<GetTemplateQualifierT<Qualifier>::value MEMBER_ITERATE(MEMBER_TYPE_IT, __VA_ARGS__)>::str( \
			BOOST_PP_STRINGIZE(Name), trailing  MEMBER_ITERATE(MEMBER_STR_IT, __VA_ARGS__)  \
		); \
	} \
}

#define GL_DECLARE(name, ...) __VA_ARGS__  name(BOOST_PP_STRINGIZE(name))

//#define GL_INTERFACE(Qualifier, name, Name, ...) \
//			GL_STRUCT(Name, __VA_ARGS__ ); \
//			Qualifier<Name> name(BOOST_PP_STRINGIZE(name));


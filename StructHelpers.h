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
#define INIT_MEMBER_PARENT(r, data, i, elem) , STRIP(elem)(BOOST_PP_STRINGIZE(STRIP(elem)), TRACKED, this, true) 
#define INIT_MEMBER_PARENT_FROM_EXP(r, data, i, elem) , STRIP(elem)(BOOST_PP_STRINGIZE(STRIP(elem)), TRACKED, this, true) 
#define INIT_MEMBER_PARENT_NEW(r, data, i, elem) , STRIP(elem)(createExp<FieldSelector>(getExpForced<false>(*this), std::make_shared<std::string>(BOOST_PP_STRINGIZE(STRIP(elem)))),NOT_TRACKED) 
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
	StructTypename(const std::string & _name = "", NamedObjectTracking _track = TRACKED, NamedObjectBase * _parent = nullptr, bool _isUsed = false ) \
		: NamedObject<StructTypename>(_name, _track, _parent, _isUsed) \
		 BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	{  \
		if(!_parent) {  isUsed = true;  if(_track) { exp = createDeclaration<StructTypename>(myNamePtr()); } } \
	} \
	\
	StructTypename(const Ex & _exp, NamedObjectTracking _track = TRACKED, NamedObjectInit _init = INIT) : NamedObject<StructTypename>()	\
	  BOOST_PP_SEQ_FOR_EACH_I(INIT_MEMBER_PARENT, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)) \
	{ \
		if(!parent) { \
			if( _init ) { exp = createInit<StructTypename, HIDE, NO_PARENTHESIS>(myNamePtr(), _exp); } else { exp = _exp; }\
			if (!_track) { areNotInit(*this); } \
			isUsed = false; \
		} \
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
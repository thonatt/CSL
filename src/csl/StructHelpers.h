#pragma once

#include <string>
#include <tuple>

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/variadic/elem.hpp>
#include <boost/preprocessor/seq/pop_front.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/facilities/is_empty.hpp>

#include "ExpressionsTest.h"

//helpers for C++ reflexion from http://pfultz2.com/blog/2012/07/31/reflection-in-under-100-lines/
#define CSL_PP_REM(...) __VA_ARGS__
#define CSL_PP_EAT(...)
#define CSL_PP_STRIP(x) CSL_PP_EAT x		
#define CSL_PP_PAIR(x) CSL_PP_REM x

//helpers wrapping up BOOST_PP
#define CSL_PP_STR(arg) BOOST_PP_STRINGIZE(arg)
#define CSL_PP_IS_EMPTY(arg) BOOST_PP_IS_EMPTY(arg)
#define CSL_PP_HEAD(elem) BOOST_PP_SEQ_HEAD(elem)
#define CSL_PP_IF_EMPTY(arg,t,f) BOOST_PP_IIF(BOOST_PP_IS_EMPTY(arg),t,f)
#define CSL_PP_NOT_EMPTY(arg) BOOST_PP_COMPL(BOOST_PP_IS_EMPTY(arg))
#define CSL_PP_COMMA_IF_NOT_EMPTY(arg) BOOST_PP_COMMA_IF(CSL_PP_NOT_EMPTY(arg))

#define CSL_PP_ITERATE(macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, , BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define CSL_PP_ITERATE_1(data, macro, ...) \
	BOOST_PP_SEQ_FOR_EACH_I(macro, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

//helpers for member infos acces
#define CSL_PP_MEMBER_TYPE(elem) CSL_PP_HEAD(elem)
#define CSL_PP_MEMBER_NAME(elem) CSL_PP_STRIP(elem)
#define CSL_PP_MEMBER_STR(elem) CSL_PP_STR(CSL_PP_MEMBER_NAME(elem))
#define CSL_PP_MEMBER_TYPE_QUALI(quali, elem) GetQualifierType<quali, CSL_PP_MEMBER_TYPE(elem) >

//macros used when iterating over members
#define CSL_PP_MEMBER_TYPE_IT(r, data, i, elem) , CSL_PP_MEMBER_TYPE(elem)
#define CSL_PP_MEMBER_STR_IT(r, data, i, elem) , CSL_PP_MEMBER_STR(elem)
#define CSL_PP_MEMBER_ARG_IT(r, data, i, elem) CSL_PP_MEMBER_TYPE(elem) && CSL_PP_MEMBER_NAME(elem),
#define CSL_PP_MEMBER_ARG_EX_IT(r, data, i, elem) , EX( CSL_PP_MEMBER_TYPE(elem), CSL_PP_MEMBER_NAME(elem))

#define CSL_PP_DECLARE_MEMBER_IT(r, data, i, elem) CSL_PP_PAIR(elem);
#define CSL_PP_INIT_MEMBER_PARENT_IT(r, data, i, elem) , CSL_PP_MEMBER_NAME(elem)( \
	createExp<MemberAccessor>(exp, CSL_PP_MEMBER_STR(elem)), 0, ALWAYS_EXP) 

//helpers for declaring members
#define CSL_PP_QUALI_TYPENAME(Qualifier, Typename) \
	typename GetQualifier<Qualifier CSL_PP_COMMA_IF_NOT_EMPTY(Qualifier) Typename>::Type

#define CSL_PP_TYPENAME_ARRAY(Quali, Typename, ArraySize) \
	Array<CSL_PP_QUALI_TYPENAME(Quali, Typename), ArraySize>

#define CSL_PP_TYPENAME_NO_ARRAY(Quali, Typename, ArraySize) \
	CSL_PP_QUALI_TYPENAME(Quali, Typename)

#define CSL_PP_TYPENAME_FULL(Quali, Typename, ArraySize) \
	CSL_PP_IF_EMPTY(ArraySize, \
		CSL_PP_TYPENAME_NO_ARRAY, \
		CSL_PP_TYPENAME_ARRAY \
	) (Quali, Typename, ArraySize)

#define CSL_PP_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	CSL_PP_QUALI_TYPENAME(quali, CSL_PP_MEMBER_TYPE(elem)) \
	CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), DISABLED);

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	CSL_PP_QUALI_TYPENAME(quali, CSL_PP_MEMBER_TYPE(elem)) \
	static CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), DISABLED);

//internal macros for named/unnamed interface blocks

#define CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, ... ) \
	struct Typename : NamedObject<Typename> { \
		static_assert(CSL_PP_IS_EMPTY(ArraySize), "unnamed interface block cant be array"); \
		Typename() = delete; \
		static std::string typeStr(int trailing) { return CSL_PP_STR(Typename); } \
	}; 

#define CSL_PP_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	listen().add_unnamed_interface_block<CSL_PP_QUALI_TYPENAME(Qualifier, Typename) CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >( \
			"" CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP_ITERATE_1(Qualifier, CSL_PP_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ ) 

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	CSL_PP_ITERATE_1(Qualifier, CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ )

#define CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, ...) \
	struct Typename : public NamedObject<Typename> { \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__ ) \
		\
		Typename(const std::string & _name = "", uint _flags = IS_TRACKED) \
			: NamedObject<Typename>(_name, _flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		Typename(const Ex & _ex, uint ctor_flags = 0, uint obj_flags = IS_TRACKED, const std::string & s = "" ) \
			 : NamedObject<Typename>(_ex,ctor_flags,obj_flags,s) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing) { \
			return CSL_PP_STR(Typename) + InterfaceDeclarationStr<Typename CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__ )>::str( \
				trailing CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__ ) \
			); \
		} \
		\
		static std::string typeNamingStr(int trailing) { \
			return CSL_PP_STR(Typename); \
		} \
	}; 

#define CSL_PP_NAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	CSL_PP_TYPENAME_FULL(Qualifier,Typename,ArraySize) Name(CSL_PP_STR(Name)); 
	//listen().add_blank_line(1);

#define CSL_PP_BUILT_IN_NAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ...) \
	CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	static CSL_PP_TYPENAME_FULL(Qualifier,Typename,ArraySize) Name(CSL_PP_STR(Name), DISABLED); 

//actual macros

#define GL_INTERFACE_BLOCK(Qualifier, Typename, Name, ArraySize, ... ) \
	static_assert(CSL_PP_NOT_EMPTY(Qualifier), "interface block must have In, Out or Uniform qualifier"); \
	CSL_PP_IF_EMPTY(Name, \
		CSL_PP_UNNAMED_INTERFACE, \
		CSL_PP_NAMED_INTERFACE\
	) ( \
		Qualifier, Typename, Name, ArraySize, __VA_ARGS__ \
	)

#define GL_STRUCT(StructTypename,...)  \
	listen().add_struct<true CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >(CSL_PP_STR(StructTypename) \
				CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	\
	struct StructTypename : public NamedObject<StructTypename> { \
		using NamedObject<StructTypename>::exp; \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		StructTypename(const std::string & _name = "", uint _flags = IS_TRACKED) \
			: NamedObject<StructTypename>(_name, _flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const Ex & _ex, uint ctor_flags = 0,	uint obj_flags = IS_TRACKED, const std::string & s = "") \
			: NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_IT,__VA_ARGS__) bool dummy = false) \
			: NamedObject<StructTypename>(DISPLAY_TYPE | PARENTHESIS, IS_TRACKED, "" CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_EX_IT, __VA_ARGS__)) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing = 0) { return CSL_PP_STR(StructTypename); } \
		static std::string typeNamingStr(int trailing = 0) { return typeStr(); } \
	}

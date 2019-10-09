#pragma once

#include <string>
#include <tuple>

#include "Preprocessor.hpp"
#include "NamedObjects.hpp"

//helpers for member infos acces
#define CSL_PP_MEMBER_TYPE(elem) CSL_PP_HEAD(elem)
#define CSL_PP_MEMBER_NAME(elem) CSL_PP_STRIP(elem)
#define CSL_PP_MEMBER_STR(elem) CSL_PP_STR(CSL_PP_MEMBER_NAME(elem))
#define CSL_PP_MEMBER_TYPE_QUALI(quali, elem) GetQualifierType<quali, CSL_PP_MEMBER_TYPE(elem) >

//macros used when iterating over members
#define CSL_PP_MEMBER_TYPE_IT(r, data, i, elem) , CSL_PP_MEMBER_TYPE(elem)
#define CSL_PP_MEMBER_STR_IT(r, data, i, elem) , CSL_PP_MEMBER_STR(elem)
#define CSL_PP_MEMBER_ARG_IT(r, data, i, elem) CSL_PP_MEMBER_TYPE(elem) && CSL_PP_MEMBER_NAME(elem),
#define CSL_PP_MEMBER_ARG_EX_IT(r, data, i, elem) , core::getExp(std::forward<CSL_PP_MEMBER_TYPE(elem)>(CSL_PP_MEMBER_NAME(elem)))

#define CSL_PP_DECLARE_MEMBER_IT(r, data, i, elem) CSL_PP_PAIR(elem);
#define CSL_PP_INIT_MEMBER_PARENT_IT(r, data, i, elem) , CSL_PP_MEMBER_NAME(elem)( \
	core::createExp<core::MemberAccessor>(exp, CSL_PP_MEMBER_STR(elem)), core::MEMBER_DECLARATION, core::ALWAYS_EXP) 

//helpers for declaring members
#define CSL_PP_QUALI_TYPENAME(Qualifier, Typename) \
	typename core::GetQualifier<Qualifier CSL_PP_COMMA_IF_NOT_EMPTY(Qualifier) Typename>::Type

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
	CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), core::DISABLED);

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	CSL_PP_QUALI_TYPENAME(quali, CSL_PP_MEMBER_TYPE(elem)) \
	static CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), core::DISABLED);

//internal macros for named/unnamed interface blocks

#define CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, ... ) \
	struct Typename : core::NamedObject<Typename> { \
		static_assert(CSL_PP_IS_EMPTY(ArraySize), "unnamed interface block cant be array"); \
		Typename() = delete; \
		static std::string typeStr(int trailing) { return CSL_PP_STR(Typename); } \
	}; 

#define CSL_PP_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	core::listen().add_unnamed_interface_block<CSL_PP_QUALI_TYPENAME(Qualifier, Typename) CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >( \
			"" CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP_ITERATE_DATA(Qualifier, CSL_PP_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ ) 

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, __VA_ARGS__ ); \
	CSL_PP_ITERATE_DATA(Qualifier, CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ )

#define CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, Name, ArraySize, ...) \
	struct Typename : public core::NamedObject<Typename> { \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__ ) \
		\
		Typename(const std::string & _name = "", core::uint _flags = core::IS_TRACKED) \
			: core::NamedObject<Typename>(_name, _flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		Typename(const core::Ex & _ex, core::uint ctor_flags = 0, core::uint obj_flags = core::IS_TRACKED, const std::string & s = "" ) \
			 : core::NamedObject<Typename>(_ex, ctor_flags, obj_flags, s) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing) { \
			return CSL_PP_STR(Typename) + core::InterfaceDeclarationStr<Typename CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__ )>::str( \
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
	static CSL_PP_TYPENAME_FULL(Qualifier,Typename,ArraySize) Name(CSL_PP_STR(Name), core::DISABLED); 

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
	core::listen().add_struct<true CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >(CSL_PP_STR(StructTypename) \
				CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	\
	struct StructTypename : public core::NamedObject<StructTypename> { \
		using core::NamedObject<StructTypename>::exp; \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		StructTypename(const std::string & _name = "", core::uint _flags = core::IS_TRACKED) \
			: core::NamedObject<StructTypename>(_name, _flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::Ex & _ex, core::uint ctor_flags = 0,	core::uint obj_flags = core::IS_TRACKED, const std::string & s = "") \
			: core::NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::NamedObjectInit<StructTypename> & obj) \
			: core::NamedObject<StructTypename>(obj)	\
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_IT,__VA_ARGS__) bool dummy = false) \
			: core::NamedObject<StructTypename>(core::DISPLAY_TYPE | core::PARENTHESIS, core::IS_TRACKED, "" CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_EX_IT, __VA_ARGS__)) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing = 0) { return CSL_PP_STR(StructTypename); } \
		static std::string typeNamingStr(int trailing = 0) { return typeStr(); } \
	}




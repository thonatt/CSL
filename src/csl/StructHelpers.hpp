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
	core::createExp<core::MemberAccessor>(exp, CSL_PP_MEMBER_STR(elem)), \
	core::OpFlags::MEMBER_DECLARATION, core::ObjFlags::ALWAYS_EXP) 

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
	CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), core::OpFlags::DISABLED);

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	static CSL_PP_QUALI_TYPENAME(quali, CSL_PP_MEMBER_TYPE(elem)) \
	CSL_PP_MEMBER_NAME(elem)(CSL_PP_MEMBER_STR(elem), core::OpFlags::DISABLED);

//internal macros for named/unnamed interface blocks

#define CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	struct TypenameId : core::NamedObject<TypenameId> { \
		static_assert(CSL_PP_IS_EMPTY(ArraySize), "unnamed interface block cant be array"); \
		TypenameId() = delete; \
		static std::string typeStr(int trailing) { return CSL_PP_STR(Typename); } \
	}; 

#define CSL_PP_UNNAMED_INTERFACE(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	core::listen().add_unnamed_interface_block<CSL_PP_QUALI_TYPENAME(Qualifier, TypenameId) CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >( \
			"" CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP_ITERATE_DATA(Qualifier, CSL_PP_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ ) 

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE_IMPL(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	CSL_PP_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	CSL_PP_ITERATE_DATA(Qualifier, CSL_PP_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ );

#define CSL_PP_BUILT_IN_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	CSL_PP_BUILT_IN_UNNAMED_INTERFACE_IMPL(Qualifier, Typename, CSL_PP_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__ ) 

#define CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, ...) \
	struct TypenameId : public core::NamedObject<TypenameId> { \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__ ) \
		\
		TypenameId(const std::string & _name = "", core::ObjFlags _flags = core::ObjFlags::IS_TRACKED) \
			: core::NamedObject<TypenameId>(_name, _flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		TypenameId(const core::Ex & _ex, core::OpFlags ctor_flags = core::OpFlags::NONE, core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED, const std::string & s = "" ) \
			 : core::NamedObject<TypenameId>(_ex, ctor_flags, obj_flags, s) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing) { \
			return CSL_PP_STR(Typename) + core::InterfaceDeclarationStr<TypenameId CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__ )>::str( \
				trailing CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__ ) \
			); \
		} \
		\
		static std::string typeNamingStr(int trailing) { \
			return CSL_PP_STR(Typename); \
		} \
	}; 

#define CSL_PP_DECLARATION(Qualifier, Typename, Name, ArraySize, Flags) \
	CSL_PP_TYPENAME_FULL(Qualifier,Typename,ArraySize) Name(CSL_PP_STR(Name), Flags);

#define CSL_PP_NAMED_INTERFACE(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	CSL_PP_DECLARATION(Qualifier, TypenameId, Name, ArraySize, core::ObjFlags::IS_TRACKED);

#define CSL_PP_BUILT_IN_NAMED_INTERFACE_IMPL(Qualifier, Typename, TypenameId, Name, ArraySize, ...) \
	CSL_PP_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	static CSL_PP_DECLARATION(Qualifier, TypenameId, Name, ArraySize, core::OpFlags::BUILT_IN); 

#define CSL_PP_BUILT_IN_NAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ...) \
	CSL_PP_BUILT_IN_NAMED_INTERFACE_IMPL(Qualifier, Typename, CSL_PP_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__);

//actual macros

#define CSL_INTERFACE_BLOCK(Qualifier, Typename, Name, ArraySize, ... ) \
	static_assert(CSL_PP_NOT_EMPTY(Qualifier), "interface block must have In, Out or Uniform qualifier"); \
	CSL_PP_IF_EMPTY(Name, \
		CSL_PP_UNNAMED_INTERFACE, \
		CSL_PP_NAMED_INTERFACE\
	) ( \
		Qualifier, Typename, CSL_PP_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__ \
	)

#define CSL_STRUCT(StructTypename,...)  \
	core::listen().add_struct<true CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >(CSL_PP_STR(StructTypename) \
				CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	\
	struct StructTypename : public core::NamedObject<StructTypename> { \
		using core::NamedObject<StructTypename>::exp; \
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		StructTypename(const std::string & _name = "", core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED) \
			: core::NamedObject<StructTypename>(_name, obj_flags) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::Ex & _ex, core::OpFlags ctor_flags = core::OpFlags::NONE, core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED, const std::string & s = "") \
			: core::NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::NamedObjectInit<StructTypename> & obj) \
			: core::NamedObject<StructTypename>(obj)	\
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_IT,__VA_ARGS__) bool dummy = false) \
			: core::NamedObject<StructTypename>(core::OpFlags::DISPLAY_TYPE | core::OpFlags::PARENTHESIS, core::ObjFlags::IS_TRACKED, "" CSL_PP_ITERATE(CSL_PP_MEMBER_ARG_EX_IT, __VA_ARGS__)) \
			CSL_PP_ITERATE(CSL_PP_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing = 0) { return CSL_PP_STR(StructTypename); } \
		static std::string typeNamingStr(int trailing = 0) { return typeStr(); } \
	}




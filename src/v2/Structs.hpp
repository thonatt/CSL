#pragma once

#include <string>
#include <tuple>

#include "Preprocessor.hpp"
#include "NamedObjects.hpp"

//helpers for member infos acces
#define PP_CSL_MEMBER_TYPE(elem) PP_CSL_HEAD(elem)
#define PP_CSL_MEMBER_NAME(elem) PP_CSL_STRIP(elem)
#define PP_CSL_MEMBER_STR(elem) PP_CSL_STR(PP_CSL_MEMBER_NAME(elem))
#define PP_CSL_MEMBER_TYPE_QUALI(quali, elem) GetQualifierType<quali, PP_CSL_MEMBER_TYPE(elem) >

//macros used when iterating over members
#define PP_CSL_MEMBER_TYPE_IT(r, data, i, elem) , PP_CSL_MEMBER_TYPE(elem)
#define PP_CSL_MEMBER_STR_IT(r, data, i, elem) , PP_CSL_MEMBER_STR(elem)
#define PP_CSL_MEMBER_ARG_IT(r, data, i, elem) PP_CSL_MEMBER_TYPE(elem) && PP_CSL_MEMBER_NAME(elem),
#define PP_CSL_MEMBER_ARG_EX_IT(r, data, i, elem) , get_expr(std::forward<PP_CSL_MEMBER_TYPE(elem)>(PP_CSL_MEMBER_NAME(elem)))

#define PP_CSL_DECLARE_MEMBER_IT(r, data, i, elem) PP_CSL_PAIR(elem);
#define PP_CSL_INIT_MEMBER_PARENT_IT(r, data, i, elem) , PP_CSL_MEMBER_NAME(elem)( \
	core::createExp<core::MemberAccessor>(exp, PP_CSL_MEMBER_STR(elem)), \
	core::OpFlags::MEMBER_DECLARATION, core::ObjFlags::ALWAYS_EXP) 

//helpers for declaring members
#define PP_CSL_QUALI_TYPENAME(Qualifier, Typename) \
	typename core::GetQualifier<Qualifier PP_CSL_COMMA_IF_NOT_EMPTY(Qualifier) Typename>::Type

#define PP_CSL_TYPENAME_ARRAY(Quali, Typename, ArraySize) \
	Array<PP_CSL_QUALI_TYPENAME(Quali, Typename), ArraySize>

#define PP_CSL_TYPENAME_NO_ARRAY(Quali, Typename, ArraySize) \
	PP_CSL_QUALI_TYPENAME(Quali, Typename)

#define PP_CSL_TYPENAME_FULL(Quali, Typename, ArraySize) \
	PP_CSL_IF_EMPTY(ArraySize, \
		PP_CSL_TYPENAME_NO_ARRAY, \
		PP_CSL_TYPENAME_ARRAY \
	) (Quali, Typename, ArraySize)

#define PP_CSL_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	PP_CSL_QUALI_TYPENAME(quali, PP_CSL_MEMBER_TYPE(elem)) \
	PP_CSL_MEMBER_NAME(elem)(PP_CSL_MEMBER_STR(elem), core::OpFlags::DISABLED);

#define PP_CSL_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	static PP_CSL_QUALI_TYPENAME(quali, PP_CSL_MEMBER_TYPE(elem)) \
	PP_CSL_MEMBER_NAME(elem)(PP_CSL_MEMBER_STR(elem), core::OpFlags::DISABLED);

//internal macros for named/unnamed interface blocks

#define PP_CSL_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	struct TypenameId : core::NamedObject<TypenameId> { \
		static_assert(PP_CSL_IS_EMPTY(ArraySize), "unnamed interface block cant be array"); \
		TypenameId() = delete; \
		static std::string typeStr(int trailing = 0) { return PP_CSL_STR(Typename); } \
	}; 

#define PP_CSL_UNNAMED_INTERFACE(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	PP_CSL_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	core::listen().add_unnamed_interface_block<PP_CSL_QUALI_TYPENAME(Qualifier, TypenameId) PP_CSL_ITERATE(PP_CSL_MEMBER_TYPE_IT, __VA_ARGS__) >( \
			"" PP_CSL_ITERATE(PP_CSL_MEMBER_STR_IT, __VA_ARGS__) ); \
	PP_CSL_ITERATE_DATA(Qualifier, PP_CSL_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ ) 

#define PP_CSL_BUILT_IN_UNNAMED_INTERFACE_IMPL(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	PP_CSL_UNNAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	PP_CSL_ITERATE_DATA(Qualifier, PP_CSL_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION, __VA_ARGS__ );

#define PP_CSL_BUILT_IN_UNNAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ... ) \
	PP_CSL_BUILT_IN_UNNAMED_INTERFACE_IMPL(Qualifier, Typename, PP_CSL_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__ ) 

#define PP_CSL_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, ...) \
	struct TypenameId : public core::NamedObject<TypenameId> { \
		\
		PP_CSL_ITERATE(PP_CSL_DECLARE_MEMBER_IT, __VA_ARGS__ ) \
		\
		TypenameId(const std::string & _name = "", core::ObjFlags _flags = core::ObjFlags::IS_TRACKED) \
			: core::NamedObject<TypenameId>(_name, _flags) \
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		TypenameId(const core::Ex & _ex, core::OpFlags ctor_flags = core::OpFlags::NONE, core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED, const std::string & s = "" ) \
			 : core::NamedObject<TypenameId>(_ex, ctor_flags, obj_flags, s) \
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing = 0) { \
			return PP_CSL_STR(Typename) + core::InterfaceDeclarationStr<TypenameId PP_CSL_ITERATE(PP_CSL_MEMBER_TYPE_IT, __VA_ARGS__ )>::str( \
				trailing PP_CSL_ITERATE(PP_CSL_MEMBER_STR_IT, __VA_ARGS__ ) \
			); \
		} \
		\
		static std::string typeNamingStr(int trailing = 0) { \
			return PP_CSL_STR(Typename); \
		} \
	}; 

#define PP_CSL_DECLARATION(Qualifier, Typename, Name, ArraySize, Flags) \
	PP_CSL_TYPENAME_FULL(Qualifier,Typename,ArraySize) Name(PP_CSL_STR(Name), Flags);

#define PP_CSL_NAMED_INTERFACE(Qualifier, Typename, TypenameId, Name, ArraySize, ... ) \
	PP_CSL_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	PP_CSL_DECLARATION(Qualifier, TypenameId, Name, ArraySize, core::ObjFlags::IS_TRACKED);

#define PP_CSL_BUILT_IN_NAMED_INTERFACE_IMPL(Qualifier, Typename, TypenameId, Name, ArraySize, ...) \
	PP_CSL_NAMED_INTERFACE_INTERNAL(Qualifier, Typename, TypenameId, Name, ArraySize, __VA_ARGS__ ); \
	static PP_CSL_DECLARATION(Qualifier, TypenameId, Name, ArraySize, core::OpFlags::BUILT_IN); 

#define PP_CSL_BUILT_IN_NAMED_INTERFACE(Qualifier, Typename, Name, ArraySize, ...) \
	PP_CSL_BUILT_IN_NAMED_INTERFACE_IMPL(Qualifier, Typename, PP_CSL_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__);

//actual macros

#define CSL_INTERFACE_BLOCK(Qualifier, Typename, Name, ArraySize, ... ) \
	static_assert(PP_CSL_NOT_EMPTY(Qualifier), "interface block must have In, Out or Uniform qualifier"); \
	PP_CSL_IF_EMPTY(Name, \
		PP_CSL_UNNAMED_INTERFACE, \
		PP_CSL_NAMED_INTERFACE\
	) ( \
		Qualifier, Typename, PP_CSL_CONCAT(Typename, __COUNTER__), Name, ArraySize, __VA_ARGS__ \
	)

#define CSL_STRUCT(StructTypename,...)  \
	core::listen().add_struct<true PP_CSL_ITERATE(PP_CSL_MEMBER_TYPE_IT, __VA_ARGS__) >(PP_CSL_STR(StructTypename) \
				PP_CSL_ITERATE(PP_CSL_MEMBER_STR_IT, __VA_ARGS__) ); \
	\
	struct StructTypename : public core::NamedObject<StructTypename> { \
		using core::NamedObject<StructTypename>::exp; \
		\
		PP_CSL_ITERATE(PP_CSL_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		StructTypename(const std::string & _name = "", core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED) \
			: core::NamedObject<StructTypename>(_name, obj_flags) \
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::Ex & _ex, core::OpFlags ctor_flags = core::OpFlags::NONE, core::ObjFlags obj_flags = core::ObjFlags::IS_TRACKED, const std::string & s = "") \
			: core::NamedObject<StructTypename>(_ex, ctor_flags, obj_flags, s)	\
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(const core::NamedObjectInit<StructTypename> & obj) \
			: core::NamedObject<StructTypename>(obj)	\
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		StructTypename(PP_CSL_ITERATE(PP_CSL_MEMBER_ARG_IT,__VA_ARGS__) bool dummy = false) \
			: core::NamedObject<StructTypename>(core::OpFlags::DISPLAY_TYPE | core::OpFlags::PARENTHESIS, core::ObjFlags::IS_TRACKED, "" PP_CSL_ITERATE(PP_CSL_MEMBER_ARG_EX_IT, __VA_ARGS__)) \
			PP_CSL_ITERATE(PP_CSL_INIT_MEMBER_PARENT_IT, __VA_ARGS__) { } \
		\
		static std::string typeStr(int trailing = 0) { return PP_CSL_STR(StructTypename); } \
		static std::string typeNamingStr(int trailing = 0) { return typeStr(); } \
	}


#define CSL_BLOCK(QualifiedTypename, Name, ...) \
	PP_CSL_GET_TYPE_LIST(QualifiedTypename) "+" PP_CSL_STRIP(QualifiedTypename)

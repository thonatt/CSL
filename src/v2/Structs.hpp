#pragma once

#include <string>
#include <tuple>

#include "Preprocessor.hpp"
#include "NamedObjects.hpp"

#include "AlgebraTypes.hpp"

#include "Listeners.hpp"

//helpers for member infos acces
#define CSL_PP2_MEMBER_TYPE(elem) FIRST(elem)
#define CSL_PP2_MEMBER_NAME(elem) SECOND(elem)
#define CSL_PP2_MEMBER_STR(elem) CSL_PP2_STR(CSL_PP2_MEMBER_NAME(elem))

//macros used when iterating over members
#define CSL_PP2_MEMBER_TYPE_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_TYPE(elem)
#define CSL_PP2_MEMBER_STR_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_STR(elem)
#define CSL_PP2_MEMBER_ARG_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) && CSL_PP2_MEMBER_NAME(elem),
#define CSL_PP2_MEMBER_ARG_EX_IT(r, data, i, elem) , v2::get_expr(std::forward<CSL_PP2_MEMBER_TYPE(elem)>(CSL_PP2_MEMBER_NAME(elem)))

#define CSL_PP2_DECLARE_MEMBER_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) CSL_PP2_MEMBER_NAME(elem);

#define CSL_PP2_INIT_MEMBER_IT(r, data, i, elem) , CSL_PP2_MEMBER_NAME(elem) ( \
	 v2::make_expr<v2::MemberAccessor<data,i>>(Base::m_expr), v2::ObjFlags::StructMember | v2::ObjFlags::AlwaysExp )

#define CSL_PP2_ARRAY_INFOS_FROM_QUALIFIER2(Qualifiers) typename v2::ArrayInfos< CSL_PP2_REMOVE_PARENTHESIS(Qualifiers) >::Dimensions
#define CSL_PP2_QUALIFIERS_LIST(Qualifiers) v2::RemoveArrayFromQualifiers< CSL_PP2_REMOVE_PARENTHESIS(Qualifiers) > 

#define CSL_PP2_STRUCT( _Qualifiers, StructTypename, StructTypenameStr,  ...) \
	struct StructTypename : public v2::NamedObject<StructTypename> { \
		using Base = v2::NamedObject<StructTypename>;\
		\
		CSL_PP2_ITERATE(CSL_PP2_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		using MemberTList = v2::TList< CSL_PP2_ITERATE(CSL_PP2_MEMBER_TYPE_IT, __VA_ARGS__) >; \
		using ArrayDimensions = CSL_PP2_ARRAY_INFOS_FROM_QUALIFIER2( _Qualifiers ); \
		using Qualifiers = CSL_PP2_QUALIFIERS_LIST( _Qualifiers ); \
		using QualifierFree = StructTypename; \
		\
		StructTypename(StructTypename && other) : Base(other) \
			CSL_PP2_ITERATE_DATA(StructTypename, CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const std::string & name = "", const v2::ObjFlags obj_flags = v2::ObjFlags::Default) \
			: NamedObjectBase(obj_flags), Base(name, obj_flags) \
			CSL_PP2_ITERATE_DATA(StructTypename, CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const v2::Expr& expr, const v2::ObjFlags obj_flags = v2::ObjFlags::Default) \
			: NamedObjectBase(obj_flags), Base(expr, obj_flags) \
			CSL_PP2_ITERATE_DATA(StructTypename, CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename operator=(const StructTypename& other) & { \
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_ref(), other.get_expr_as_ref()) }; \
		} \
		\
		StructTypename operator=(const StructTypename& other) && { \
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_temp(), other.get_expr_as_ref()) }; \
		} \
		\
		StructTypename operator=(StructTypename&& other) & { \
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_ref(), other.get_expr_as_temp()) }; \
		} \
		\
		StructTypename operator=(StructTypename&& other) && { \
			return { make_expr<BinaryOperator>(Op::Assignment, NamedObjectBase::get_expr_as_temp(), other.get_expr_as_temp()) }; \
		} \
		\
		static const std::string& get_member_name(const std::size_t member_id) { \
			static const std::vector<std::string> member_names = { CSL_PP2_ITERATE(CSL_PP2_MEMBER_STR_IT, __VA_ARGS__) }; \
			return member_names[member_id]; \
		} \
		static const std::string& get_type_str() { \
			static const std::string type_str = CSL_PP2_STR(StructTypenameStr); \
			return type_str; \
		} \
	}

#define CSL_PP2_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, Name, ...) \
	struct UniqueTypename; \
	v2::listen().add_named_interface_block<UniqueTypename>(CSL_PP2_STR(Name)); \
	CSL_PP2_STRUCT(Qualifiers, UniqueTypename, Typename, __VA_ARGS__);

#define CSL_PP2_UNNAMED_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, ...) \
	v2::listen().add_unnamed_interface_block< \
		v2::TList< CSL_PP2_REMOVE_PARENTHESIS(Qualifiers) >, v2::TList< CSL_PP2_ITERATE(CSL_PP2_MEMBER_TYPE_IT, __VA_ARGS__) > > ( \
			CSL_PP2_STR(Typename), CSL_PP2_ITERATE(CSL_PP2_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP2_ITERATE(CSL_PP2_DECLARE_MEMBER_IT, __VA_ARGS__ );

#define CSL2_STRUCT(StructTypename, ...)  \
	struct StructTypename; \
	v2::listen().add_struct<StructTypename>(); \
	CSL_PP2_STRUCT((),StructTypename, StructTypename, __VA_ARGS__ )

#define CSL2_INTERFACE_BLOCK(Qualifiers, StructTypename, Name, ...) \
	CSL_PP2_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP2_CONCAT(StructTypename, __COUNTER__), Name, __VA_ARGS__ )

#define CSL2_UNNANMED_INTERFACE_BLOCK(Qualifiers, StructTypename, ...) \
	CSL_PP2_UNNAMED_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP2_CONCAT(StructTypename, __COUNTER__), __VA_ARGS__) 
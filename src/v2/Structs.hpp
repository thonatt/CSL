#pragma once

#include <string>
#include <tuple>

#include "Preprocessor.hpp"
#include "NamedObjects.hpp"

#include "AlgebraTypes.hpp"

#include "Listeners.hpp"

//helpers for member infos acces
#define CSL_PP2_MEMBER_TYPE(elem) CSL_PP_DEPARENTHESIS(CSL_PP_FIRST(elem))
#define CSL_PP2_MEMBER_NAME(elem) CSL_PP_SECOND(elem)
#define CSL_PP2_MEMBER_STR(elem) CSL_PP2_STR(CSL_PP2_MEMBER_NAME(elem))

//macros used when iterating over members
#define CSL_PP2_MEMBER_TYPE_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_TYPE(elem)
#define CSL_PP2_MEMBER_STR_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_STR(elem)
#define CSL_PP2_MEMBER_ARG_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) && CSL_PP2_MEMBER_NAME(elem),
#define CSL_PP2_MEMBER_ARG_EX_IT(r, data, i, elem) , v2::get_expr(std::forward<CSL_PP2_MEMBER_TYPE(elem)>(CSL_PP2_MEMBER_NAME(elem)))

#define CSL_PP2_DECLARE_MEMBER_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) CSL_PP2_MEMBER_NAME(elem);

#define CSL_PP2_MEMBERWISE_CTOR_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_TYPE(elem) && CSL_PP2_MEMBER_NAME(elem)
#define CSL_PP2_MEMBERWISE_ARG_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) v2::get_expr(std::forward<CSL_PP2_MEMBER_TYPE(elem)>( CSL_PP2_MEMBER_NAME(elem) ) )

#define CSL_PP2_INIT_MEMBER_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_NAME(elem)( \
	 v2::make_expr<v2::MemberAccessor<CSL_PP_FIRST(data) ,i>>(Base::m_expr), CSL_PP_SECOND(data) )

#define CSL_PP2_SET_MEMBER_IT(r, data, i, elem) \
	CSL_PP2_MEMBER_NAME(elem).m_expr = v2::make_expr<v2::MemberAccessor<data,i>>(Base::m_expr); \
	CSL_PP2_MEMBER_NAME(elem).m_flags = v2::ObjFlags::StructMember | v2::ObjFlags::AlwaysExp;

#define CSL_PP2_EMPTY_INIT_MEMBER_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_NAME(elem) ( v2::Dummy{} )

#define CSL_PP2_DECLARE_UNNAMED_INTERFACE_MEMBER_IT(r, data, i, elem) \
	CSL_PP2_MEMBER_TYPE(elem) CSL_PP2_MEMBER_NAME(elem) { CSL_PP2_MEMBER_STR(elem), v2::ObjFlags::Constructor };

#define CSL_PP2_DECLARE_BUILTIN_UNNAMED_INTERFACE_MEMBER_IT(r, data, i, elem) \
	inline CSL_PP2_MEMBER_TYPE(elem) CSL_PP2_MEMBER_NAME(elem) { CSL_PP2_MEMBER_STR(elem), data };

#define CSL_PP2_ARRAY_INFOS_FROM_QUALIFIER2(Qualifiers) typename v2::ArrayInfos< CSL_PP_DEPARENTHESIS(Qualifiers) >::Dimensions
#define CSL_PP2_QUALIFIERS_LIST(Qualifiers) v2::RemoveArrayFromQualifiers< CSL_PP_DEPARENTHESIS(Qualifiers) > 

#define CSL_PP2_STRUCT( _Qualifiers, StructTypename, StructTypenameStr, DefaultObjFlags, IsBuiltIn, ...) \
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
		StructTypename( CSL_PP2_ITERATE( CSL_PP2_MEMBERWISE_CTOR_IT, __VA_ARGS__ ) ) \
			 : Base("", v2::ObjFlags::Default, v2::CtorFlags::Initialisation, CSL_PP2_ITERATE(CSL_PP2_MEMBERWISE_ARG_IT, __VA_ARGS__) ), CSL_PP2_ITERATE_DATA((StructTypename, DefaultObjFlags), CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(StructTypename && other) : Base(other), \
			CSL_PP2_ITERATE_DATA((StructTypename, DefaultObjFlags), CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(v2::Dummy) : Base(), CSL_PP2_ITERATE(CSL_PP2_EMPTY_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const std::string& name = "", const v2::ObjFlags obj_flags = v2::ObjFlags::Default) \
			: Base(name, obj_flags), \
			CSL_PP2_ITERATE_DATA((StructTypename, obj_flags | DefaultObjFlags), CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const v2::Expr expr, const v2::ObjFlags obj_flags = v2::ObjFlags::Default) \
			: Base(expr, obj_flags), \
			CSL_PP2_ITERATE_DATA((StructTypename, obj_flags), CSL_PP2_INIT_MEMBER_IT, __VA_ARGS__) { } \
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
		void set_members() { \
			CSL_PP2_ITERATE_DATA(StructTypename, CSL_PP2_SET_MEMBER_IT, __VA_ARGS__); \
		}\
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

#define CSL_PP2_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, Name, DefaultObjFlags, ...) \
	struct UniqueTypename; \
	v2::listen().add_named_interface_block<UniqueTypename>(CSL_PP2_STR(Name)); \
	CSL_PP2_STRUCT(Qualifiers, UniqueTypename, Typename, DefaultObjFlags, 0, __VA_ARGS__); \
	Qualify<UniqueTypename, CSL_PP_DEPARENTHESIS(Qualifiers)> Name( CSL_PP2_STR(Name), DefaultObjFlags | v2::ObjFlags::UsedAsRef );

#define CSL_PP2_BUILTIN_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, Name, ...) \
	CSL_PP2_STRUCT((),UniqueTypename, Typename,  v2::ObjFlags::BuiltInConstructor | v2::ObjFlags::UsedAsRef, 1, __VA_ARGS__); \
	inline Qualify<UniqueTypename, CSL_PP_DEPARENTHESIS(Qualifiers)> Name( CSL_PP2_STR(Name) );

#define CSL_PP2_UNNAMED_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, DefaultObjFlags,  ...) \
	v2::listen().add_unnamed_interface_block< \
		v2::TList< CSL_PP_DEPARENTHESIS(Qualifiers) >, v2::TList< CSL_PP2_ITERATE(CSL_PP2_MEMBER_TYPE_IT, __VA_ARGS__) > > ( \
			CSL_PP2_STR(Typename), CSL_PP2_ITERATE(CSL_PP2_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP2_ITERATE(CSL_PP2_DECLARE_UNNAMED_INTERFACE_MEMBER_IT, __VA_ARGS__ );

#define CSL_PP2_BUILTIN_UNNAMED_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, DefaultObjFlags,  ...) \
	CSL_PP2_ITERATE_DATA(v2::ObjFlags::BuiltInConstructor, CSL_PP2_DECLARE_BUILTIN_UNNAMED_INTERFACE_MEMBER_IT, __VA_ARGS__ );

#define CSL2_STRUCT(StructTypename, ...)  \
	struct StructTypename; \
	v2::listen().add_struct<StructTypename>(); \
	CSL_PP2_STRUCT((),StructTypename, StructTypename, v2::ObjFlags::StructMember | v2::ObjFlags::AlwaysExp, 0, __VA_ARGS__ )

#define CSL2_INTERFACE_BLOCK(Qualifiers, StructTypename, Name, ...) \
	CSL_PP2_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP2_CONCAT(StructTypename, __COUNTER__), Name, v2::ObjFlags::Constructor,  __VA_ARGS__ )

#define CSL2_UNNANMED_INTERFACE_BLOCK(Qualifiers, StructTypename, ...) \
	CSL_PP2_UNNAMED_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP2_CONCAT(StructTypename, __COUNTER__), v2::ObjFlags::Default, __VA_ARGS__ ) 
#pragma once

#include <csl/Listeners.hpp>
#include <csl/NamedObjects.hpp>
#include <csl/Preprocessor.hpp>

//#include "AlgebraTypes.hpp"

#include <array>
#include <string>

//helpers for member infos acces
#define CSL_PP_MEMBER_TYPE(elem) CSL_PP_DEPARENTHESIS(CSL_PP_FIRST(elem))
#define CSL_PP_MEMBER_NAME(elem) CSL_PP_SECOND(elem)
#define CSL_PP_MEMBER_STR(elem) CSL_PP_STR(CSL_PP_MEMBER_NAME(elem))

//macros used when iterating over members
#define CSL_PP_MEMBER_TYPE_IT(data, i, elem) CSL_PP_COMMA_IF(i) CSL_PP_MEMBER_TYPE(elem)
#define CSL_PP_MEMBER_STR_IT(data, i, elem) CSL_PP_COMMA_IF(i) CSL_PP_MEMBER_STR(elem)

#define CSL_PP_DECLARE_MEMBER_IT(data, i, elem) CSL_PP_MEMBER_TYPE(elem) CSL_PP_MEMBER_NAME(elem);

#define CSL_PP_MEMBERWISE_CTOR_IT(data, i, elem) CSL_PP_COMMA_IF(i) CSL_PP_MEMBER_TYPE(elem) && CSL_PP_MEMBER_NAME(elem)
#define CSL_PP_MEMBERWISE_ARG_IT(data, i, elem) CSL_PP_COMMA_IF(i) csl::get_expr(std::forward<CSL_PP_MEMBER_TYPE(elem)>( CSL_PP_MEMBER_NAME(elem) ) )

#define CSL_PP_INIT_MEMBER_IT(data, i, elem) CSL_PP_COMMA_IF(i) CSL_PP_MEMBER_NAME(elem)( \
	 csl::make_expr<csl::MemberAccessor<CSL_PP_FIRST(data) ,i>>(Base::m_expr), CSL_PP_SECOND(data) )

#define CSL_PP_SET_MEMBER_IT(data, i, elem) \
	CSL_PP_MEMBER_NAME(elem).m_expr = csl::make_expr<csl::MemberAccessor<data,i>>(Base::m_expr); \
	CSL_PP_MEMBER_NAME(elem).m_flags = csl::ObjFlags::StructMember;

#define CSL_PP_EMPTY_INIT_MEMBER_IT(data, i, elem) CSL_PP_COMMA_IF(i) CSL_PP_MEMBER_NAME(elem) ( csl::Dummy{} )

#define CSL_PP_DECLARE_UNNAMED_INTERFACE_MEMBER_IT(data, i, elem) \
	CSL_PP_MEMBER_TYPE(elem) CSL_PP_MEMBER_NAME(elem) { CSL_PP_MEMBER_STR(elem), csl::ObjFlags::Constructor };

#define CSL_PP_DECLARE_BUILTIN_UNNAMED_INTERFACE_MEMBER_IT(data, i, elem) \
	inline CSL_PP_MEMBER_TYPE(elem) CSL_PP_MEMBER_NAME(elem) { CSL_PP_MEMBER_STR(elem), data };

#define CSL_PP_DIMENSIONS_FROM_QUALIFIERS(Qualifiers) typename csl::ArrayInfos< CSL_PP_DEPARENTHESIS(Qualifiers) >::Dimensions
#define CSL_PP_QUALIFIERS_LIST(Qualifiers) csl::RemoveArrayFromQualifiers< CSL_PP_DEPARENTHESIS(Qualifiers) > 

#define CSL_PP_STRUCT(StructTypename, StructTypenameStr, DefaultObjFlags, ...) \
	struct StructTypename : public csl::NamedObject<StructTypename> { \
		using Base = csl::NamedObject<StructTypename>;\
		\
		CSL_PP_ITERATE(CSL_PP_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		using MemberTList = csl::TList< CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) >; \
		\
		StructTypename( CSL_PP_ITERATE( CSL_PP_MEMBERWISE_CTOR_IT, __VA_ARGS__ ) ) \
			 : Base("", csl::ObjFlags::Default, csl::CtorFlags::Initialisation, SizeList<>{}, TList<>{}, CSL_PP_ITERATE(CSL_PP_MEMBERWISE_ARG_IT, __VA_ARGS__) ), CSL_PP_ITERATE_DATA((StructTypename, DefaultObjFlags), CSL_PP_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(StructTypename && other) : Base(other), \
			CSL_PP_ITERATE_DATA((StructTypename, DefaultObjFlags), CSL_PP_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(csl::Dummy) : Base(), CSL_PP_ITERATE(CSL_PP_EMPTY_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const csl::NamedObjectInit<StructTypename>& init) : Base(init), \
			CSL_PP_ITERATE_DATA((StructTypename, DefaultObjFlags), CSL_PP_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const std::string& name = "", const csl::ObjFlags obj_flags = csl::ObjFlags::Default) \
			: Base(name, obj_flags, SizeList<>{}, TList<>{}), \
			CSL_PP_ITERATE_DATA((StructTypename, obj_flags | DefaultObjFlags), CSL_PP_INIT_MEMBER_IT, __VA_ARGS__) { } \
		\
		StructTypename(const csl::Expr expr, const csl::ObjFlags obj_flags = csl::ObjFlags::Default) \
			: Base(expr, obj_flags), \
			CSL_PP_ITERATE_DATA((StructTypename, obj_flags), CSL_PP_INIT_MEMBER_IT, __VA_ARGS__) { } \
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
			CSL_PP_ITERATE_DATA(StructTypename, CSL_PP_SET_MEMBER_IT, __VA_ARGS__); \
		}\
		\
		static const std::string& get_member_name(const std::size_t member_id) { \
			static const std::array<std::string, CSL_PP_VARIADIC_SIZE(__VA_ARGS__)> member_names = { CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) }; \
			return member_names[member_id]; \
		} \
		static const std::string& get_type_str() { \
			static const std::string type_str = CSL_PP_STR(StructTypenameStr); \
			return type_str; \
		} \
		\
		static constexpr bool IsValid() { return true; } \
	}

#define CSL_PP_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, Name, DefaultObjFlags, ...) \
	struct UniqueTypename; \
	csl::context::get().add_named_interface_block< \
		UniqueTypename, CSL_PP_DIMENSIONS_FROM_QUALIFIERS(Qualifiers), CSL_PP_QUALIFIERS_LIST(Qualifiers)>(CSL_PP_STR(Name)); \
	CSL_PP_STRUCT(UniqueTypename, Typename, DefaultObjFlags, __VA_ARGS__); \
	Qualify<UniqueTypename, CSL_PP_DEPARENTHESIS(Qualifiers)> Name( CSL_PP_STR(Name), DefaultObjFlags | csl::ObjFlags::UsedAsRef );

#define CSL_PP_BUILTIN_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, Name, ...) \
	CSL_PP_STRUCT(UniqueTypename, Typename, csl::ObjFlags::BuiltInConstructor | csl::ObjFlags::UsedAsRef, __VA_ARGS__); \
	inline Qualify<UniqueTypename, CSL_PP_DEPARENTHESIS(Qualifiers)> Name( CSL_PP_STR(Name) );

#define CSL_PP_UNNAMED_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, DefaultObjFlags,  ...) \
	csl::context::get().add_unnamed_interface_block< \
		csl::TList< CSL_PP_DEPARENTHESIS(Qualifiers) >, csl::TList< CSL_PP_ITERATE(CSL_PP_MEMBER_TYPE_IT, __VA_ARGS__) > > ( \
			CSL_PP_STR(Typename), CSL_PP_ITERATE(CSL_PP_MEMBER_STR_IT, __VA_ARGS__) ); \
	CSL_PP_ITERATE(CSL_PP_DECLARE_UNNAMED_INTERFACE_MEMBER_IT, __VA_ARGS__ );

#define CSL_PP_BUILTIN_UNNAMED_INTERFACE_BLOCK(Qualifiers, Typename, UniqueTypename, DefaultObjFlags,  ...) \
	CSL_PP_ITERATE_DATA(csl::ObjFlags::BuiltInConstructor, CSL_PP_DECLARE_BUILTIN_UNNAMED_INTERFACE_MEMBER_IT, __VA_ARGS__ );

#define CSL_STRUCT(StructTypename, ...)  \
	struct StructTypename; \
	csl::context::get().add_struct<StructTypename>(); \
	CSL_PP_STRUCT(StructTypename, StructTypename, csl::ObjFlags::StructMember, __VA_ARGS__ )

#define CSL_INTERFACE_BLOCK(Qualifiers, StructTypename, Name, ...) \
	CSL_PP_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP_CONCAT(StructTypename, __COUNTER__), Name, csl::ObjFlags::Constructor,  __VA_ARGS__ )

#define CSL_UNNAMED_INTERFACE_BLOCK(Qualifiers, StructTypename, ...) \
	CSL_PP_UNNAMED_INTERFACE_BLOCK(Qualifiers, StructTypename, CSL_PP_CONCAT(StructTypename, __COUNTER__), csl::ObjFlags::Default, __VA_ARGS__ ) 

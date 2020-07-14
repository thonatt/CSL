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
#define CSL_PP2_MEMBER_STR(elem) CSL_PP2_STR(FIRST(elem))

//macros used when iterating over members
#define CSL_PP2_MEMBER_TYPE_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_TYPE(elem)
#define CSL_PP2_MEMBER_STR_IT(r, data, i, elem) CSL_PP2_COMMA_IF(i) CSL_PP2_MEMBER_STR(elem)
#define CSL_PP2_MEMBER_ARG_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) && CSL_PP2_MEMBER_NAME(elem),
#define CSL_PP2_MEMBER_ARG_EX_IT(r, data, i, elem) , v2::get_expr(std::forward<CSL_PP2_MEMBER_TYPE(elem)>(CSL_PP2_MEMBER_NAME(elem)))

#define CSL_PP2_DECLARE_MEMBER_IT(r, data, i, elem) CSL_PP2_MEMBER_TYPE(elem) CSL_PP2_MEMBER_NAME(elem);

#define CSL_PP2_INIT_MEMBER_IT(r, data, i, elem) , CSL_PP2_MEMBER_NAME(elem) ( \
	 v2::make_expr<v2::MemberAccessor<data,i>>(Base::m_expr), v2::ObjFlags::StructMember | v2::ObjFlags::AlwaysExp )

//helpers for declaring members
#define CSL_PP2_QUALI_TYPENAME(Qualifier, Typename) \
	typename core::GetQualifier<Qualifier CSL_PP2_COMMA_IF_NOT_EMPTY(Qualifier) Typename>::Type

#define CSL_PP2_TYPENAME_ARRAY(Quali, Typename, ArraySize) \
	Array<CSL_PP2_QUALI_TYPENAME(Quali, Typename), ArraySize>

#define CSL_PP2_TYPENAME_NO_ARRAY(Quali, Typename, ArraySize) \
	CSL_PP2_QUALI_TYPENAME(Quali, Typename)

#define CSL_PP2_TYPENAME_FULL(Quali, Typename, ArraySize) \
	CSL_PP2_IF_EMPTY(ArraySize, \
		CSL_PP2_TYPENAME_NO_ARRAY, \
		CSL_PP2_TYPENAME_ARRAY \
	) (Quali, Typename, ArraySize)

#define CSL_PP2_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	CSL_PP2_QUALI_TYPENAME(quali, CSL_PP2_MEMBER_TYPE(elem)) \
	CSL_PP2_MEMBER_NAME(elem)(CSL_PP2_MEMBER_STR(elem), core::OpFlags::DISABLED);

#define CSL_PP2_BUILT_IN_UNNAMED_INTERFACE_MEMBER_DECLARATION(r, quali, i, elem) \
	static CSL_PP2_QUALI_TYPENAME(quali, CSL_PP2_MEMBER_TYPE(elem)) \
	CSL_PP2_MEMBER_NAME(elem)(CSL_PP2_MEMBER_STR(elem), core::OpFlags::DISABLED);


//namespace v2 {

//	struct TestStruct;
//
//
//	struct TestStruct : NamedObject<TestStruct> {
//		using Base = NamedObject<TestStruct>;
//
//		vec3 m_v;
//		Float m_f;
//
//		using MemberTList = TList<vec3, Float>;
//		using ArrayDimensions = SizeList<>;
//		using Qualifiers = TList<>;
//		using QualifierFree = TestStruct;
//
//		TestStruct(const std::string& name = "", const ObjFlags obj_flags = ObjFlags::Default)
//			: Base(name, obj_flags),
//			m_v(make_expr<MemberAccessorWrapper>(MemberAccessorWrapper::create<TestStruct, 0>(Base::m_expr)), ObjFlags::StructMember | ObjFlags::AlwaysExp),
//			m_f(make_expr<MemberAccessorWrapper>(MemberAccessorWrapper::create<TestStruct, 1>(Base::m_expr)), ObjFlags::StructMember | ObjFlags::AlwaysExp)
//		{
//		}
//
//		static const std::string& get_member_name(const std::size_t member_id) {
//			static const std::vector<std::string> member_names = { "m_v", "m_f" };
//			return member_names[member_id];
//		}
//		static const std::string& get_type_str() {
//			static const std::string type_str = CSL_PP2_STR(TestStruct);
//			return type_str;
//		}
//	};
//}

#define CSL_PP2_STRUCT(StructTypename, ...) \
	struct StructTypename : public v2::NamedObject<StructTypename> { \
		using Base = v2::NamedObject<StructTypename>;\
		\
		CSL_PP2_ITERATE(CSL_PP2_DECLARE_MEMBER_IT, __VA_ARGS__) \
		\
		using MemberTList = v2::TList< CSL_PP2_ITERATE(CSL_PP2_MEMBER_TYPE_IT, __VA_ARGS__) >; \
		using ArrayDimensions = v2::SizeList<>; \
		using Qualifiers = v2::TList<>; \
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
			static const std::string type_str = CSL_PP2_STR(StructTypename); \
			return type_str; \
		} \
	}

#define CSL2_STRUCT(StructTypename, ...)  \
	struct StructTypename; \
	v2::listen().add_struct<StructTypename>(); \
	CSL_PP2_STRUCT(StructTypename, __VA_ARGS__ )

#define CSL2_PP_UNNAMED_INTERFACE()
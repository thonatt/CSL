#pragma once

#include "Operators.hpp"
#include "Types.hpp"

#include <cassert>
#include <type_traits>
#include <string>

namespace csl {

	struct MainListener;
	MainListener& listen();

	enum class ObjFlags : std::size_t {
		None = 0,
		Tracked = 1 << 1,
		Constructor = 1 << 2,
		UsedAsRef = 1 << 3,
		StructMember = 1 << 4,
		BuiltIn = 1 << 5,
		Const = 1 << 6,
		BuiltInConstructor = BuiltIn | UsedAsRef | Constructor,
		Default = Tracked | Constructor
	};

	constexpr ObjFlags operator|(const ObjFlags a, const ObjFlags b) {
		return static_cast<ObjFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr bool operator&(const ObjFlags a, const ObjFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a) & static_cast<std::size_t>(b));
	}
	constexpr ObjFlags operator~(const ObjFlags a) {
		return static_cast<ObjFlags>(~static_cast<std::size_t>(a));
	}
	constexpr ObjFlags& operator|=(ObjFlags& a, const ObjFlags b) {
		a = a | b;
		return a;
	}

	class NamedObjectBase
	{
	public:
		~NamedObjectBase();

		NamedObjectBase(const ObjFlags flags = ObjFlags::Default) : id(counter++), m_flags(flags)
		{
		}

		void set_as_temp() const 
		{
			if (m_flags & ObjFlags::Constructor) {
				auto ctor = safe_static_cast<ConstructorBase*>(retrieve_expr(m_expr));
				ctor->set_as_temp();

				if (ctor->arg_count() == 1)
					m_expr = ctor->first_arg();
			}

			if (m_flags & ObjFlags::StructMember)
				safe_static_cast<MemberAccessorBase*>(retrieve_expr(m_expr))->set_as_temp();
		}

		void set_as_const() const
		{
			if (!(m_flags & ObjFlags::Const)) {
				m_flags |= ObjFlags::Const;
				if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(m_expr))) {
					ctor->set_as_const();
				}
			}
		}

		Expr get_expr_as_ref_internal() const
		{
			m_flags |= ObjFlags::UsedAsRef;
			if (m_flags & ObjFlags::StructMember)
				return m_expr;
			return make_expr<Reference>(id);
		}

		Expr get_expr_as_ref() const
		{
			set_as_const();
			return get_expr_as_ref_internal();
		}

		Expr get_expr_as_ref()
		{
			return static_cast<const NamedObjectBase&>(*this).get_expr_as_ref_internal();
		}

		Expr get_expr_as_temp() const
		{
			if (!(m_flags & ObjFlags::UsedAsRef)) {
				m_flags |= ObjFlags::UsedAsRef;
				set_as_temp();
			}
			return m_expr;
		}

		Expr get_expr_as_temp()
		{
			return static_cast<const NamedObjectBase*>(this)->get_expr_as_temp();
		}

		Expr get_this_expr()& { return get_expr_as_ref(); }
		Expr get_this_expr() const& { return get_expr_as_ref(); }
		Expr get_this_expr()&& { return get_expr_as_temp(); }
		Expr get_this_expr() const&& { return get_expr_as_temp(); }

		Expr get_plain_expr() const 
		{
			return m_expr;
		}

		void set_members() { }

	public:
		std::size_t id;
		mutable Expr m_expr;
		mutable ObjFlags m_flags;

		static std::size_t counter;
	};

	inline std::size_t NamedObjectBase::counter = 0;

	template<typename T, typename Dimensions = SizeList<>, typename Qualifiers = TList<>, typename ... Args>
	Expr create_variable_expr(const std::string& name, const ObjFlags obj_flags, CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args);

	template<typename T, typename Enabler = void>
	struct ExprGetter {
		static Expr get_expr(T&& t) {
			return std::forward<T>(t).get_this_expr();
		}
	};

	template<typename T>
	Expr get_expr(T&& t) {
		return ExprGetter<T>::get_expr(std::forward<T>(t));
	}

	template<typename T>
	struct NamedObjectInit
	{
		NamedObjectInit(const Expr& expr, const std::string& name) : m_expr(expr), m_name(name) {}
		Expr m_expr;
		std::string m_name;
	};

	template<typename T>
	class NamedObject : public NamedObjectBase 
	{
	public:

		NamedObjectInit<T> operator<<(const std::string& name) const&& 
		{
			return { get_expr_as_temp(), name };
		}

		NamedObject() = default;

		template<typename ArrayDimensions, typename Qualifiers, typename ...Args>
		NamedObject(const std::string& name, const ObjFlags obj_flags, const CtorFlags ctor_flags, ArrayDimensions, Qualifiers, Args&& ...args)
			: NamedObjectBase(obj_flags)
		{
			if (obj_flags & ObjFlags::Constructor)
				m_expr = create_variable_expr<T, ArrayDimensions, Qualifiers>(name, obj_flags, ctor_flags, NamedObjectBase::id, std::forward<Args>(args)...);
		}

		template<typename ArrayDimensions = SizeList<>, typename Qualifiers = TList<>>
		NamedObject(const std::string& name, const ObjFlags obj_flags, ArrayDimensions, Qualifiers)
			: NamedObject(name, obj_flags, CtorFlags::Declaration, ArrayDimensions{}, Qualifiers{})
		{
		}

		NamedObject(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: NamedObjectBase(obj_flags) {
			if (obj_flags & ObjFlags::StructMember)
				m_expr = expr;
			else if (obj_flags & ObjFlags::Constructor)
				m_expr = create_variable_expr<T>("", obj_flags, CtorFlags::Initialisation, NamedObjectBase::id, expr);
		}

		template<typename ArrayDimensions = SizeList<>, typename Qualifiers = TList<>>
		NamedObject(const NamedObjectInit<T>& init, ArrayDimensions = {}, Qualifiers = {})
			: NamedObjectBase(ObjFlags::Default | ObjFlags::UsedAsRef)
		{
			m_expr = create_variable_expr<T, ArrayDimensions, Qualifiers>(init.m_name, ObjFlags::Default | ObjFlags::UsedAsRef, CtorFlags::Initialisation, NamedObjectBase::id, init.m_expr);
		}
	};


	template<typename T, typename ...Qs>
	struct TypeInterface : T
	{
		using Qualifiers = TList<Qs...>;

		using T::operator=;

		template<typename U, typename = std::enable_if_t<SameType<T, U>>>
		TypeInterface(U&& rhs)
			: TypeInterface(make_expr<ConvertorOperator<U, T>>(get_expr(std::forward<U>(rhs))))
		{
		}

		template<std::size_t N>
		TypeInterface(const char(&name)[N], const ObjFlags obj_flags = ObjFlags::Default)
			: T(Dummy{})
		{
			if (obj_flags & ObjFlags::Constructor)
				T::m_expr = create_variable_expr<T, SizeList<>, Qualifiers>(name, obj_flags, CtorFlags::Declaration, NamedObjectBase::id);

			T::set_members();
		}

		TypeInterface(const std::string& name = "", const ObjFlags obj_flags = ObjFlags::Default)
			: T(Dummy{})
		{
			if (obj_flags & ObjFlags::Constructor)
				T::m_expr = create_variable_expr<T, SizeList<>, Qualifiers>(name, obj_flags, CtorFlags::Declaration, NamedObjectBase::id);

			T::set_members();
		}

		TypeInterface(const Expr expr, const ObjFlags obj_flags = ObjFlags::Default)
			: T(Dummy{})
		{
			if (obj_flags & ObjFlags::StructMember)
				T::m_expr = expr;
			else if (obj_flags & ObjFlags::Constructor)
				T::m_expr = create_variable_expr<T, SizeList<>, Qualifiers>("", obj_flags, CtorFlags::Initialisation, NamedObjectBase::id, expr);

			T::set_members();
		}

		TypeInterface(const NamedObjectInit<T>& init)
			: T(Dummy{})
		{
			T::m_expr = create_variable_expr<T, SizeList<>, Qualifiers>(init.m_name, ObjFlags::Default, CtorFlags::Initialisation, NamedObjectBase::id, init.m_expr);
			T::set_members();
		}
	};

	template<typename T, typename ArrayDimensions, typename ... Qualifiers>
	struct ArrayInterface : NamedObject<ArrayInterface<T, ArrayDimensions>>
	{
		using Base = NamedObject<ArrayInterface<T, ArrayDimensions>>;
		using QualifierList = TList<Qualifiers...>;

		static constexpr std::size_t ComponentCount = ArrayDimensions::Front;

		using ArrayComponent = std::conditional_t<
			(ArrayDimensions::Tail::Size > 0),
			Qualify<T, typename GetArrayFromList<typename ArrayDimensions::Tail>::Type, Qualifiers...>,
			Qualify<T, Qualifiers...>
		>;

		static constexpr bool IsArray = true;

		ArrayInterface(csl::Dummy) : Base()
		{
		}

		ArrayInterface() : Base("")
		{
		}

		template<std::size_t N>
		explicit ArrayInterface(const char(&name)[N], const ObjFlags obj_flags = ObjFlags::Default)
			: Base(name, obj_flags, ArrayDimensions{}, QualifierList{})
		{
		}

		ArrayInterface(const Expr expr, const ObjFlags obj_flags = ObjFlags::Default)
			: Base(expr, obj_flags)
		{
		}

		template<typename U, typename = std::enable_if_t<SameType<ArrayInterface, U>>>
		ArrayInterface(const NamedObjectInit<U>& init) 
			: Base(init, ArrayDimensions{}, QualifierList{})
		{
		}

		template<typename ... Us, typename = std::enable_if_t<
			(SameType<Us, ArrayComponent>&& ...) && ((ComponentCount == 0 && sizeof...(Us) > 0) || (sizeof...(Us) == ComponentCount))
			>>
			explicit ArrayInterface(Us&& ... us)
			: Base("", ObjFlags::Default, CtorFlags::Initialisation, ArrayDimensions{}, QualifierList{}, get_expr(std::forward<Us>(us))...)
		{
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index)& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), get_expr(std::forward<Index>(index))) };
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index)&& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_temp(), get_expr(std::forward<Index>(index))) };
		}
	};

	template<typename T>
	struct ExprGetter<T, std::enable_if_t<std::is_fundamental_v<T>>> 
	{
		static Expr get_expr(T&& t) {
			return make_expr<Litteral<T>>(std::forward<T>(t));
		}
	};

	template<typename T>
	struct ExprGetter<T&, std::enable_if_t<std::is_fundamental_v<T>>> 
	{
		static Expr get_expr(const T& t) {
			return make_expr<Litteral<T>>(t);
		}
	};
}
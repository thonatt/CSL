#pragma once

#include "Operators.hpp"
#include "Types.hpp"

#include <type_traits>
#include <string>

namespace v2 {

	enum class ObjFlags : std::size_t {
		None = 0,
		Tracked = 1 << 1,
		Constructor = 1 << 2,
		UsedAsRef = 1 << 3,
		AlwaysExp = 1 << 4,
		StructMember = 1 << 5,
		BuiltIn = UsedAsRef | Constructor,
		Default = Tracked | Constructor
	};

	constexpr ObjFlags operator|(const ObjFlags a, const ObjFlags b) {
		return static_cast<ObjFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr bool operator&(const ObjFlags a, const ObjFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}
	constexpr ObjFlags operator~(const ObjFlags a) {
		return static_cast<ObjFlags>(~static_cast<std::size_t>(a));
	}
	inline ObjFlags& operator|=(ObjFlags& a, const ObjFlags b) {
		a = a | b;
		return a;
	}

	class NamedObjectBase {
	public:
		virtual ~NamedObjectBase();

		NamedObjectBase(const ObjFlags flags = ObjFlags::Default) : id(counter++), m_flags(flags)
		{
		}

		void set_as_temp() const {
			if (m_flags & ObjFlags::Constructor) {
				auto ctor = std::dynamic_pointer_cast<ConstructorBase>(m_expr);
				ctor->set_as_temp();

				//auto tmp = ctor->first_arg();

				if (ctor->arg_count() == 1) {
					m_expr = ctor->first_arg();
				}
				
				//m_expr = std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(ctor->first_arg());
				//m_expr.swap(ctor->first_arg());
			}

			if (m_flags & ObjFlags::StructMember) {
				std::dynamic_pointer_cast<MemberAccessorBase>(m_expr)->set_as_temp();
			}
		}

		Expr get_expr_as_ref() const
		{
			if (!(m_flags & ObjFlags::UsedAsRef)) {
				m_flags |= ObjFlags::UsedAsRef;
			}
			if (m_flags & ObjFlags::AlwaysExp) {
				return m_expr;
				//return std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(m_expr)->m_operator.m_ctor->first_arg();
			}
			return make_expr<Reference<Dummy>>(id);
		}

		Expr get_expr_as_ref()
		{
			return static_cast<const NamedObjectBase*>(this)->get_expr_as_ref();
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

		Expr get_plain_expr() const {
			return m_expr;
		}

	protected:
		std::size_t id;
		mutable Expr m_expr;
		mutable ObjFlags m_flags;

		static std::size_t counter;
	};

	inline std::size_t NamedObjectBase::counter = 0;

	template<typename T, typename ... Args>
	Expr create_variable_expr(const std::string& name, const ObjFlags obj_flags, const CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args);

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
	struct NamedObjectInit {
		NamedObjectInit(const Expr& expr, const std::string& name) : m_expr(expr), m_name(name) {}
		Expr m_expr;
		std::string m_name;
	};

	template<typename T>
	class NamedObject : virtual public NamedObjectBase {
	public:

		virtual ~NamedObject() = default;

		NamedObjectInit<T> operator<<(const std::string& name) const&& {
			return { get_expr_as_temp(), name };
		}

		NamedObject(const std::string& name = "", const ObjFlags obj_flags = ObjFlags::Default)
			: NamedObjectBase(obj_flags) {
			if (obj_flags & ObjFlags::Constructor) {
				m_expr = create_variable_expr<T>(name, obj_flags, CtorFlags::Declaration, NamedObjectBase::id);
			}
		}

		template<typename ... Args>
		NamedObject(const std::string& name, const ObjFlags obj_flags, const CtorFlags ctor_flags, Args&&... args)
			: NamedObjectBase(obj_flags) {
			if (obj_flags & ObjFlags::Constructor) {
				m_expr = create_variable_expr<T>(name, obj_flags, ctor_flags, NamedObjectBase::id, std::forward<Args>(args)...);
			}
		}

		NamedObject(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: NamedObjectBase(obj_flags) {
			if (obj_flags & ObjFlags::StructMember) {
				m_expr = expr;
			} else if (obj_flags & ObjFlags::Constructor) {
				m_expr = create_variable_expr<T>("", obj_flags, CtorFlags::Initialisation, NamedObjectBase::id, expr);
			}
		}

		NamedObject(const NamedObjectInit<T>& init)
			: NamedObjectBase(ObjFlags::Default | ObjFlags::UsedAsRef)
		{
			m_expr = create_variable_expr<T>(init.m_name, ObjFlags::Default | ObjFlags::UsedAsRef, CtorFlags::Initialisation, NamedObjectBase::id, init.m_expr);
		}


	private:
	};


	template<typename T, typename ...Qs>
	struct TypeInterface : NamedObject<TypeInterface<T, Qs... >>, T {
		using Qualifiers = TList<Qs...>;
		using ArrayDimensions = SizeList<>;
		using QualifierFree = T;

		using Base = NamedObject<TypeInterface<T, Qs... >>;
		using T::operator=;

		TypeInterface(const std::string& name = "", const ObjFlags obj_flags = ObjFlags::Default)
			: NamedObjectBase(obj_flags), Base(name, obj_flags), T(NamedObjectBase::m_expr, ObjFlags::None) { }

		TypeInterface(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default)
			: NamedObjectBase(obj_flags), Base(expr, obj_flags), T(NamedObjectBase::m_expr, ObjFlags::None) {

		}

		//TypeInterface(T&& t) : Base(get_expr(std::move(t))), T(Base::m_expr, ObjFlags::None) { }

		//Expr get_this_expr()& { return Base::get_expr_as_ref(); }
		//Expr get_this_expr() const& { return Base::get_expr_as_ref(); }
		//Expr get_this_expr()&& { return Base::get_expr_as_temp(); }
		//Expr get_this_expr() const&& { return Base::get_expr_as_temp(); }
	};

	template<typename T, typename Ds, typename ... Qs>
	struct ArrayInterface : virtual NamedObject<ArrayInterface<T, Ds, Qs...>> {

		virtual ~ArrayInterface() = default;

		using Base = NamedObject<ArrayInterface<T, Ds, Qs...>>;

		using ArrayDimensions = Ds;
		using Qualifiers = TList<Qs...>;
		using QualifierFree = T;

		static constexpr std::size_t ComponentCount = Ds::Front;

		using ArrayComponent = Qualify<T, typename GetArrayFromList<typename Ds::Tail>::Type, Qs...>;
		static constexpr bool IsArray = true;

		ArrayInterface() : Base() {}

		template<std::size_t N>
		explicit ArrayInterface(const char(&name)[N], const ObjFlags obj_flags = ObjFlags::Default) 
			: Base(name, obj_flags) {}

		ArrayInterface(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default) 
			: NamedObjectBase(obj_flags), Base(expr, obj_flags) { }

		template<typename ... Us, typename = std::enable_if_t<
			!(std::is_same_v<Expr, Us> || ...) && ((ComponentCount == 0 && sizeof...(Us) > 0) || (sizeof...(Us) == ComponentCount)) && (SameType<Us, ArrayComponent>&& ...)
			>>
			explicit ArrayInterface(Us&& ... us) : NamedObjectBase("", ObjFlags::Default), Base("", ObjFlags::Default, CtorFlags::Initialisation, get_expr(std::forward<Us>(us))...)
		{
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index) const& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_ref(), get_expr(std::forward<Index>(index))) };
		}

		template<typename Index>
		ArrayComponent operator [](Index&& index) const&& {
			return { make_expr<ArraySubscript>(NamedObjectBase::get_expr_as_temp(), get_expr(std::forward<Index>(index))) };
		}

	};

	template<typename T>
	struct ExprGetter<T, std::enable_if_t<std::is_fundamental_v<T>>> {
		static Expr get_expr(T&& t) {
			return make_expr<Litteral<T>>(std::forward<T>(t));
		}
	};

	template<typename T>
	struct ExprGetter<T&, std::enable_if_t<std::is_fundamental_v<T>>> {
		static Expr get_expr(const T& t) {
			return make_expr<Litteral<T>>(t);
		}
	};
}
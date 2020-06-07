#pragma once

#include "Operators.hpp"

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
		Default = Tracked | Constructor
	};

	constexpr ObjFlags operator|(ObjFlags a, ObjFlags b) {
		return static_cast<ObjFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr bool operator&(ObjFlags a, ObjFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}
	constexpr ObjFlags operator~(ObjFlags a) {
		return static_cast<ObjFlags>(~static_cast<std::size_t>(a));
	}
	inline ObjFlags& operator|=(ObjFlags& a, ObjFlags b) {
		a = a | b;
		return a;
	}

	class NamedObjectBase {
	public:

		NamedObjectBase(const std::string& name = "", const ObjFlags flags = ObjFlags::Default) : m_name(name), id(counter++), m_flags(flags)
		{
		}

		void set_as_temp() const {
			if (m_flags & ObjFlags::Constructor) {
				auto ctor = std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(m_expr)->m_operator.m_ctor;
				ctor->set_as_temp();

				//auto tmp = ctor->first_arg();

				//m_expr = ctor->first_arg();

				//m_expr = std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(ctor->first_arg());


				//m_expr.swap(ctor->first_arg());
			}

			if (m_flags & ObjFlags::StructMember) {
				std::dynamic_pointer_cast<OperatorWrapper<MemberAccessorWrapper>>(m_expr)->m_operator.m_member_accessor->set_as_temp();
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
			return make_expr<Reference>(id);
		}

		Expr get_expr_as_ref()
		{
			return static_cast<const NamedObjectBase*>(this)->get_expr_as_ref();
		}

		Expr get_expr_as_temp() const
		{
			if (!(m_flags & ObjFlags::UsedAsRef)) {
				set_as_temp();
				m_flags |= ObjFlags::UsedAsRef;
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

	protected:
		std::string m_name;
		std::size_t id;
		mutable Expr m_expr;
		mutable ObjFlags m_flags;

		static std::size_t counter;
	};

	inline std::size_t NamedObjectBase::counter = 0;

	template<typename T, typename ... Args>
	Expr create_variable_expr(const ObjFlags obj_flags, const CtorFlags ctor_flags, const std::size_t variable_id, Args&& ... args);

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
	class NamedObject : public NamedObjectBase {
	public:

		NamedObject(const std::string& name = "", const ObjFlags obj_flags = ObjFlags::Default) : NamedObjectBase(name, obj_flags) {
			m_expr = create_variable_expr<T>(obj_flags, CtorFlags::Declaration, NamedObjectBase::id);
		}

		template<typename ... Args>
		NamedObject(const std::string& name, const ObjFlags obj_flags, const CtorFlags ctor_flags, Args&&... args) : NamedObjectBase(name, obj_flags) {
			m_expr = create_variable_expr<T>(obj_flags, ctor_flags, NamedObjectBase::id, std::forward<Args>(args)...);
		}

		NamedObject(const Expr& expr, const ObjFlags obj_flags = ObjFlags::Default) : NamedObjectBase("", obj_flags) {
			if (obj_flags & ObjFlags::StructMember) {
				m_expr = expr;
			} else {
				m_expr = create_variable_expr<T>(obj_flags, CtorFlags::Initialisation, NamedObjectBase::id, expr);
			}
		}

	private:
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
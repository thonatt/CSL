#pragma once

#include "Operators.hpp"

#include <type_traits>
#include <string>

namespace v2 {

	enum class ObjFlags {
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
	constexpr ObjFlags operator&(ObjFlags a, ObjFlags b) {
		return static_cast<ObjFlags>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}
	constexpr ObjFlags operator~(ObjFlags a) {
		return static_cast<ObjFlags>(~static_cast<std::size_t>(a));
	}
	inline ObjFlags& operator&=(ObjFlags& a, ObjFlags b) {
		a = a & b;
		return a;
	}
	inline ObjFlags& operator|=(ObjFlags& a, ObjFlags b) {
		a = a | b;
		return a;
	}

	class NamedObjectBase {
	public:

		NamedObjectBase(const std::string& name = "", const ObjFlags flags = ObjFlags::Default) : m_name(name), m_flags(flags) {
		}

		Expr get_expr_as_ref() const
		{
			m_flags |= ObjFlags::UsedAsRef;
			if (static_cast<bool>(m_flags & ObjFlags::AlwaysExp)) {
				return m_expr;
			}
			return {};
		}

		Expr get_expr_as_ref()
		{
			return static_cast<const NamedObjectBase*>(this)->get_expr_as_ref();
		}

		Expr get_expr_as_temp() const
		{
			m_flags |= ObjFlags::UsedAsRef;
			return m_expr;
		}

		Expr get_expr_as_tmp()
		{
			return static_cast<const NamedObjectBase*>(this)->get_expr_as_temp();
		}

		Expr get_expr() & { return get_expr_as_ref(); }
		Expr get_expr() const& { return get_expr_as_ref(); }
		Expr get_expr() && { return get_expr_as_temp(); }
		Expr get_expr() const&& { return get_expr_as_temp(); }

	protected:
		std::string m_name;
		Expr m_expr;
		mutable ObjFlags m_flags;
	};

	template<typename T, typename ... Args>
	Expr create_initialization(ObjFlags objFlags, Args&& ... args);

	template<typename T, typename Enabler = void>
	struct ExprGetter {
		static Expr get_expr(T&& t) {
			return std::forward<T>(t).get_expr();
		}
	};

	template<typename T>
	Expr get_expr(T&& t) {
		return ExprGetter<T>::get_expr(std::forward<T>(t));
	}

	template<typename T>
	class NamedObject : public NamedObjectBase {
	public:

		NamedObject(const std::string& name = "", const ObjFlags flags = ObjFlags::Default) : NamedObjectBase(name, flags) {
			m_expr = make_expr<Constructor<T>>(CtorFlags::Declaration);
		}

	private:
	};

	template<typename T>
	struct ExprGetter<T, std::enable_if_t<std::is_fundamental_v<T>>> {
		static Expr get_expr(T&& t) {
			return make_expr<Litteral<T>>(std::forward<T>(t));
		}
	};
}
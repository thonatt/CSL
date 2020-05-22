#pragma once

#include "Operators.hpp"

#include <string>

namespace v2 {

	enum class ObjFlags {
		IsTracked = 1 << 1,
		IsConstructor = 1 << 2,
		IsUsed = 1 << 3,
		AlwaysExp = 1 << 4,
		IsStructMember = 1 << 5,
		Default = IsTracked | IsConstructor
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

	protected:
		std::string m_name;
		std::size_t m_id = 0;
		Expr m_expr;
		mutable ObjFlags m_flags;
	};

	template<typename T>
	struct Counter;

	template<typename T>
	class NamedObject : public NamedObjectBase {
	public:
	private:
		void checkName()
		{
			using AutoName = typename Counter<T>::Type;
			if (m_name.empty()) {
				m_name = AutoName::getNextName();
			}

		}
	}
}
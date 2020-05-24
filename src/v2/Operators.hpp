#pragma once

#include "SpirvOperators.hpp"

#include <array>
#include <cstddef>
#include <memory>

namespace v2 {

	template<typename T>
	struct GLSLstr;

	template<typename T>
	struct SPIRVstr;

	template<typename T>
	struct OperatorDebug;

	struct DebugData;

	struct OperatorBase;
	using Expr = std::shared_ptr<OperatorBase>;

	enum class CtorFlags : std::size_t {
		Declaration,
		Initialisation,
		Temporary,
		Forward
	};

	constexpr CtorFlags operator|(CtorFlags a, CtorFlags b) {
		return static_cast<CtorFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr bool operator&(CtorFlags a, CtorFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}
	constexpr CtorFlags operator~(CtorFlags a) {
		return static_cast<CtorFlags>(~static_cast<std::size_t>(a));
	}
	inline CtorFlags& operator|=(CtorFlags& a, CtorFlags b) {
		a = a | b;
		return a;
	}

	struct OperatorBase {
		virtual ~OperatorBase() = default;

		virtual void print_glsl() const {}
		virtual void print_spirv() const {}
		virtual void print_debug(DebugData& data) const {}

		Op op = Op::OpNop;
	};

	template<typename Operator>
	struct OperatorWrapper : OperatorBase {
		virtual void print_glsl() const override {
			//GLSLstr<Operator>::call(op);
		}

		virtual void print_spirv() const override {
			//SPIRVstr<Operator>::call(op);
		}

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Operator>::call(m_operator, data);
		}

		template<typename ...Args>
		OperatorWrapper(Args&& ...args) : m_operator{ std::forward<Args>(args)... } { }

		OperatorWrapper(Operator&& op) : m_operator(std::move(op)) { }

		Operator m_operator;
	};

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args) {
		return std::static_pointer_cast<OperatorBase>(std::make_shared<OperatorWrapper<Operator>>(std::forward<Args>(args)...));
	}

	struct Reference {
		Reference(const std::size_t id) : m_id(id) {}
		std::size_t m_id;
	};

	template<std::size_t N>
	struct ArgSeq {
		virtual ~ArgSeq() = default;

		template<typename ...Args>
		ArgSeq(Args&& ... args) : arguments{ std::forward<Args>(args)... } { }

		const std::array<Expr, N> arguments;
	};


	template<std::size_t N>
	struct FunCall : ArgSeq<N> {

		template<typename ...Args>
		FunCall(Args&& ...args) : ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	struct ConstructorBase {
		virtual ~ConstructorBase() = default;

		ConstructorBase(const CtorFlags flags) : m_flags(flags) {}

		void set_as_temp() {
			if (m_flags & CtorFlags::Initialisation) {
				m_flags |= CtorFlags::Temporary;
			}
		}

		CtorFlags m_flags;
	};

	template<typename T, std::size_t N>
	struct Constructor : ConstructorBase, ArgSeq<N> {

		using ArgSeq<N>::arguments;

		template<typename ...Args>
		Constructor(const CtorFlags flags, Args&& ...args) : ConstructorBase(flags), ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	struct ConstructorWrapper {
		
		template<typename T, typename ...Args>
		static ConstructorWrapper create(const CtorFlags flags, Args&& ...args) {
			ConstructorWrapper out;
			out.m_ctor = std::static_pointer_cast<ConstructorBase>(std::make_shared<Constructor<T, sizeof...(Args)>>(flags, std::forward<Args>(args)...));
			return out;
		}

		std::shared_ptr<ConstructorBase> m_ctor;
	};

	template<typename F, std::size_t N = F::arg_count>
	struct CustomFunCall : ArgSeq<N> {

		template<typename ... Args>
		CustomFunCall(Args&& ...args) : ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	template<typename T>
	struct Litteral {

		Litteral(const T t) : value(t) { }
		const T value;
	};

}

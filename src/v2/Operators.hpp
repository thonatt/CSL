#pragma once

#include "SpirvOperators.hpp"

#include <array>
#include <cstddef>
#include <memory>

namespace v2 {

	enum class Op {
		CWiseMul,
		MatrixTimesScalar,
		MatrixTimesMatrix,
		CWiseAdd,
		MatrixAddScalar,
		CWiseSub,
		MatrixSubScalar,
		UnaryNegation,
		Assignment
	};

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
		ArgSeq(Args&& ... args) : m_args{ std::forward<Args>(args)... } { }

		const std::array<Expr, N> m_args;
	};


	template<std::size_t N>
	struct FunCall : ArgSeq<N> {

		template<typename ...Args>
		FunCall(Args&& ...args) : ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	struct ConstructorBase {
		virtual ~ConstructorBase() = default;

		ConstructorBase(const CtorFlags flags, const std::size_t variable_id) : m_flags(flags), m_variable_id(variable_id) {}

		void set_as_temp() {
			if (m_flags & CtorFlags::Initialisation) {
				m_flags = CtorFlags::Temporary;
			}
		}

		virtual void print_debug(DebugData& data) const {}

		std::size_t m_variable_id;
		CtorFlags m_flags;
	};

	template<typename T, std::size_t N>
	struct Constructor : ConstructorBase, ArgSeq<N> {

		using ArgSeq<N>::m_args;

		template<typename ...Args>
		Constructor(const CtorFlags flags, const std::size_t variable_id, Args&& ...args) : ConstructorBase(flags, variable_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Constructor>::call(*this, data);
		}
	};

	struct ConstructorWrapper {

		template<typename T, typename ...Args>
		static ConstructorWrapper create(const CtorFlags flags, const std::size_t variable_id, Args&& ...args) {
			return ConstructorWrapper{ std::static_pointer_cast<ConstructorBase>(std::make_shared<Constructor<T, sizeof...(Args)>>(flags, variable_id, std::forward<Args>(args)...)) };
		}

		std::shared_ptr<ConstructorBase> m_ctor;
	};

	struct ArraySubscript {
		Expr m_obj, m_index;
	};

	struct SwizzlingBase {
		virtual ~SwizzlingBase() = default;

		SwizzlingBase(const Expr& expr) : m_obj(expr) { }

		virtual void print_debug(DebugData& data) const {}

		Expr m_obj;
	};

	template<typename S>
	struct Swizzling : SwizzlingBase {
		Swizzling(const Expr& expr) : SwizzlingBase(expr) { }

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Swizzling<S>>::call(*this, data);
		}
	};

	struct SwizzlingWrapper {

		template<typename S>
		static SwizzlingWrapper create(const Expr& expr) {
			return SwizzlingWrapper{ std::static_pointer_cast<SwizzlingBase>(std::make_shared<Swizzling<S>>(expr)) };
		}

		std::shared_ptr<SwizzlingBase> m_swizzle;
	};

	struct UnaryOperator {

		UnaryOperator(const Op op, const Expr& arg) : m_arg(arg), m_op(op) { }

		Expr m_arg;
		Op m_op;

	};

	struct BinaryOperator {

		BinaryOperator(const Op op, const Expr& lhs, const Expr& rhs) : m_op(op), m_lhs(lhs), m_rhs(rhs) { }

		Expr m_lhs, m_rhs;
		Op m_op;
	};

	template<typename F, std::size_t N>
	struct CustomFunCall : Reference, ArgSeq<N> {

		template<typename ... Args>
		CustomFunCall(const std::size_t fun_id, Args&& ...args) : Reference(fun_id), ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	template<typename T>
	struct Litteral {

		Litteral(const T t) : value(t) { }
		const T value;
	};

}

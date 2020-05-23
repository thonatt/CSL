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

	struct OperatorBase;
	using Expr = std::shared_ptr<OperatorBase>;

	enum class CtorFlags {
		Declaration,
		Initialisation,
		Temporary,
		Forward
	};

	struct OperatorBase {
		virtual ~OperatorBase() = default;

		virtual void print_glsl() const {}
		virtual void print_spirv() const {}

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

		template<typename ...Args>
		OperatorWrapper(Args&& ...args) : m_op{ std::forward<Args>(args)... } { }

		Operator m_op;
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

	struct ConstructorBase : OperatorBase {
		virtual ~ConstructorBase() = default;

		ConstructorBase(const CtorFlags flags) : m_flags(flags) {}

		CtorFlags m_flags;
	};

	template<typename T, typename ...Args>
	struct Constructor : ConstructorBase, ArgSeq<sizeof...(Args)> {

		template<typename ...Args>
		Constructor(const CtorFlags flags, Args&& ...args) : ConstructorBase(flags), ArgSeq<sizeof...(Args)>(std::forward<Args>(args)...) { }
	};

	template<typename F, std::size_t N = F::arg_count>
	struct CustomFunCall : ArgSeq<N> {

		template<typename ... Args>
		CustomFunCall(Args&& ...args) : ArgSeq<N>(std::forward<Args>(args)...) { }
	};

	template<typename T>
	struct Litteral : OperatorBase {

		Litteral(const T t) : value(t) { }
		const T value;
	};

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args) {
		return std::static_pointer_cast<OperatorBase>(std::make_shared<OperatorWrapper<Operator>>(std::forward<Args>(args)...));
	}


	//// language implementations

	//template<int N>
	//struct GLSLstr<FunCall<N>> {
	//	static void call(const FunCall<N>& fcall) {
	//		std::cout << fcall.arg_count() << std::endl;
	//	}
	//};


	//template<int N>
	//struct SPIRVstr<FunCall<N>> {
	//	static void call(const FunCall<N>& fcall) {
	//		std::cout << fcall.arg_count() + "_1" << std::endl;
	//	}
	//};

}

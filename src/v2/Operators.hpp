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

	struct OperatorBase {
		virtual ~OperatorBase() = default;

		virtual void print_glsl() const {}
		virtual void print_spirv() const {}

		Op op = OpNop;
	};


	template<typename Operator>
	struct OperatorWrapper : OperatorBase {
		virtual void print_glsl() const override {
			GLSLstr<Operator>::call(op);
		}

		virtual void print_spirv() const override {
			SPIRVstr<Operator>::call(op);
		}

		Operator op;
	};


	template<std::size_t N>
	struct ArgsCall : OperatorBase {
		virtual ~ArgsCall() = default;

		template<typename ...Args>
		ArgsCall(Args&& ... args) : arguments{ std::forward<Args>(args)... } { }

		const std::array<Expr, N> arguments;
	};


	template<std::size_t N>
	struct FunCall : ArgsCall<N> {
		template<typename ...Args>
		FunCall(Args&& ...args) : ArgsCall<N>(std::forward<Args>(args)...) { }
	};

	template<typename F, std::size_t N = F::arg_count>
	struct CustomFunCall : ArgsCall<N> {
		template<typename ... Args>
		CustomFunCall(Args&& ...args) : ArgsCall<N>(std::forward<Args>(args)...) { }
	};

	template<typename T>
	struct Litteral : OperatorBase {
		Litteral(const T t) : value(t) { }
		const T value;
	};

	template <typename T, typename ... Args>
	Expr make_operator(Args&& ...args) {
		return std::static_pointer_cast<OperatorBase>(std::make_shared<OperatorWrapper<T>>(std::forward<Args>(args)...));
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

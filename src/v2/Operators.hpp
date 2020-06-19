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

	enum class OperatorPrecedence : std::size_t
	{
		Alias = 10,

		ArraySubscript = 20,
		FunctionCall = 20,
		FieldAccessor = 20,
		Swizzle = 20,
		Postfix = 20,

		Prefix = 30,
		Unary = 30,
		Negation = 30,
		OnesComplement = 30,

		Division = 40,
		Modulo = 41,
		Multiply = 42,

		Addition = 50,
		Substraction = 50,

		BitwiseShift = 60,

		Relational = 70,

		Equality = 80,

		BitwiseAnd = 90,

		BitwiseXor = 100,

		BitwiseOr = 110,

		LogicalAnd = 120,

		LogicalXor = 130,

		LogicalOr = 140,

		Ternary = 150,

		Assignment = 160,

		Sequence = 170
	};

	//////////////////////////////////////////////////////////////

	//template<typename ...Ts>
	//struct VisitableBase {
	//	virtual ~VisitableBase() = default;
	//};

	//template<typename T>
	//struct VisitableBase<T> {
	//	virtual void visit(T& visitor) const = 0;
	//	virtual ~VisitableBase() = default;
	//};

	//template<typename T, typename ...Ts>
	//struct VisitableBase<T, Ts...> : virtual  VisitableBase<T>, virtual VisitableBase<Ts...>
	//{
	//	using VisitableBase<T>::visit;
	//	using VisitableBase<Ts...>::visit;
	//	virtual ~VisitableBase() = default;
	//};

	//template<typename Visited, typename Visitor>
	//struct Visiting {
	//	static void call(const Visited& visited, Visitor& visitor) {
	//		std::cout << "visiting " << typeid(Visited).name() << " with " << typeid(Visitor).name() << std::endl;
	//	}
	//};

	//template<typename Visited, typename ...Ts>
	//struct VisitableDerived {
	//	virtual ~VisitableDerived() = default;
	//};

	//template<typename Visited, typename T>
	//struct VisitableDerived<Visited, T> : virtual VisitableBase<T> {
	//	virtual ~VisitableDerived() = default;
	//	virtual void visit(T& visitor) const override {
	//		Visiting<Visited, T>::call(static_cast<const Visited&>(*this), visitor);
	//	}
	//};

	//template<typename Visited, typename T, typename ...Ts>
	//struct VisitableDerived<Visited, T, Ts...> : VisitableDerived<Visited, T>, VisitableDerived<Visited, Ts...> {
	//	virtual ~VisitableDerived() = default;
	//};

	////////////////////////////////////////////////////////

	template<typename T>
	struct OperatorDebug;

	struct DebugData;

	/////////////////////////////////////////////////////////

	template<typename T>
	struct OperatorImGui;

	struct ImGuiData;

	//////////////////////////////////////////////////////////

	template<typename T>
	struct OperatorGLSL;

	struct GLSLData;

	//////////////////////////////////////////////////////////

	struct OperatorBase;
	using Expr = std::shared_ptr<OperatorBase>;

	enum class CtorFlags : std::size_t {
		Declaration = 1 << 1,
		Initialisation = 1 << 2,
		Temporary = 1 << 3,
		Unused = 1 << 4,
		FunctionArgument = 1 << 5
	};

	//constexpr CtorFlags operator|(CtorFlags a, CtorFlags b) {
	//	return static_cast<CtorFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	//}
	constexpr bool operator&(CtorFlags a, CtorFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}
	//constexpr CtorFlags operator~(CtorFlags a) {
	//	return static_cast<CtorFlags>(~static_cast<std::size_t>(a));
	//}
	//inline CtorFlags& operator|=(CtorFlags& a, CtorFlags b) {
	//	a = a | b;
	//	return a;
	//}

	struct OperatorBase {
		virtual ~OperatorBase() = default;

		virtual void print_spirv() const {}
		virtual void print_debug(DebugData& data) const {}
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

	};

	template<typename Operator>
	struct OperatorWrapper : OperatorBase {
		virtual void print_spirv() const override {
			//SPIRVstr<Operator>::call(op);
		}

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Operator>::call(m_operator, data);
		}
		virtual void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Operator>::call(m_operator, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			OperatorGLSL<Operator>::call(m_operator, data);
		}

		template<typename ...Args>
		OperatorWrapper(Args&& ...args) : m_operator{ std::forward<Args>(args)... } { }

		OperatorWrapper(Operator&& op) : m_operator(std::move(op)) { }

		Operator m_operator;
	};

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args) {
		auto wrapper = std::make_shared<OperatorWrapper<Operator>>(std::forward<Args>(args)...);
		auto expr = std::static_pointer_cast<OperatorBase>(wrapper);
		return expr;
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
		virtual void print_debug(DebugData& data) const {}
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

		ConstructorBase(const std::string& name, const CtorFlags flags, const std::size_t variable_id) 
			: m_name(std::move(name)), m_variable_id(variable_id), m_flags(flags) {}

		void set_as_temp() {
			m_flags = CtorFlags::Temporary;
		}

		void set_as_unused() {
			if (m_flags & CtorFlags::Initialisation) {
				m_flags = CtorFlags::Unused;
			}
		}

		virtual Expr first_arg() const {
			return {};
		}

		std::string m_name;
		std::size_t m_variable_id;
		CtorFlags m_flags;
	};

	template<typename T, std::size_t N>
	struct Constructor : ConstructorBase, ArgSeq<N> {

		using ArgSeq<N>::m_args;

		template<typename ...Args>
		Constructor(const std::string& name, const CtorFlags flags, const std::size_t variable_id, Args&& ...args)
			: ConstructorBase(name, flags, variable_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		virtual Expr first_arg() const override {
			if constexpr (N == 0) {
				return {};
			} else {
				return ArgSeq<N>::m_args[0];
			}
		}

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Constructor>::call(*this, data);
		}
		virtual void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Constructor>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			OperatorGLSL<Constructor>::call(*this, data);
		}
	};

	struct ConstructorWrapper {

		template<typename T, typename ...Args>
		static ConstructorWrapper create(const std::string& name, const CtorFlags flags, const std::size_t variable_id, Args&& ...args) {
			return ConstructorWrapper{ std::static_pointer_cast<ConstructorBase>(std::make_shared<Constructor<T, sizeof...(Args)>>(name, flags, variable_id, std::forward<Args>(args)...)) };
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
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

		Expr m_obj;
	};

	template<typename S>
	struct Swizzling : SwizzlingBase {
		Swizzling(const Expr& expr) : SwizzlingBase(expr) { }

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<Swizzling<S>>::call(*this, data);
		}
		virtual void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Swizzling<S>>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			OperatorGLSL<Swizzling<S>>::call(*this, data);
		}
	};

	struct SwizzlingWrapper {

		template<typename S>
		static SwizzlingWrapper create(const Expr& expr) {
			return SwizzlingWrapper{ std::static_pointer_cast<SwizzlingBase>(std::make_shared<Swizzling<S>>(expr)) };
		}

		std::shared_ptr<SwizzlingBase> m_swizzle;
	};


	struct MemberAccessorBase {
		virtual ~MemberAccessorBase() = default;

		virtual void print_debug(DebugData& data) const {}
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

		MemberAccessorBase(const Expr& expr) : m_obj(expr) { }

		void set_as_temp();

		Expr m_obj;
	};

	template<typename S, std::size_t MemberId>
	struct MemberAccessor : MemberAccessorBase {

		MemberAccessor(const Expr& expr) : MemberAccessorBase(expr) { }

		virtual void print_debug(DebugData& data) const override {
			OperatorDebug<MemberAccessor<S, MemberId>>::call(*this, data);
		}
		virtual void print_imgui(ImGuiData& data) const override {
			OperatorImGui<MemberAccessor<S, MemberId>>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			OperatorGLSL<MemberAccessor<S, MemberId>>::call(*this, data);
		}
	};

	struct MemberAccessorWrapper {
		template<typename S, std::size_t MemberId>
		static MemberAccessorWrapper create(const Expr& expr) {
			return MemberAccessorWrapper{ std::static_pointer_cast<MemberAccessorBase>(std::make_shared<MemberAccessor<S, MemberId>>(expr)) };
		}

		std::shared_ptr<MemberAccessorBase> m_member_accessor;
	};

	inline void MemberAccessorBase::set_as_temp() {
		if (auto parent_ctor = std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(m_obj)) {
			parent_ctor->m_operator.m_ctor->set_as_temp();
		} else {
			std::dynamic_pointer_cast<OperatorWrapper<MemberAccessorWrapper>>(m_obj)->m_operator.m_member_accessor->set_as_temp();
		}
	}

	struct UnaryOperator {

		UnaryOperator(const Op op, const Expr& arg) : m_arg(arg), m_op(op) { }

		Expr m_arg;
		Op m_op;

	};

	struct BinaryOperator {

		BinaryOperator(const Op op, const Expr& lhs, const Expr& rhs) : m_lhs(lhs), m_rhs(rhs), m_op(op) { }

		Expr m_lhs, m_rhs;
		Op m_op;
	};

	template<typename F, typename ReturnType, std::size_t N>
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

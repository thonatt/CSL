#pragma once

#include "SpirvOperators.hpp"

#include <array>
#include <cstddef>
#include <memory>

#include <type_traits>
#include <utility>
#include <vector>

namespace v2 {

	enum class Op : std::size_t {
		CWiseMul,
		MatrixTimesScalar,
		ScalarTimesMatrix,
		MatrixTimesMatrix,
		CWiseAdd,
		MatrixAddScalar,
		ScalarAddMatrix,
		CWiseSub,
		MatrixSubScalar,
		ScalarSubMatrix,
		CWiseDiv,
		MatrixDivScalar,
		ScalarDivMatrix,
		UnaryNegation,
		UnarySub,
		Assignment,
		AddAssignment,
		MulAssignment,
		SubAssignment,
		ScalarLessThanScalar,
		ScalarGreaterThanScalar,
		LogicalOr,
		LogicalAnd,
		PostfixUnary,
		PrefixUnary,
		// glsl 1.1
		dFdx,
		dFdy,
		abs,
		sin,
		cos,
		tan,
		exp,
		log,
		sqrt,
		floor,
		ceil,
		fract,
		exp2,
		log2,
		normalize,
		atan,
		acos,
		asin,
		radians,
		degrees,
		greaterThan,
		lessThan,
		greaterThanEqual,
		lessThenEqual,
		equal,
		notEqual,
		max,
		min,
		mod,
		mix,
		dot,
		smoothstep,
		length,
		clamp,
		distance,
		pow,
		cross,

		//glsl 1.2
		transpose,

		// glsl 1.3
		texture,
		//glsl 1.4
		inverse,
	};

	std::ostream& operator<<(std::ostream& t, const Op op) {
		return t << static_cast<std::size_t>(op);
	}

	enum class Precedence : std::size_t
	{
		Alias = 10,

		ArraySubscript = 20,
		FunctionCall = 20,
		MemberAccessor = 20,
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

		Sequence = 170,

		NoExtraParenthesis = 900
	};

	inline bool operator<(const Precedence a, const Precedence b) {
		return static_cast<std::size_t>(a) < static_cast<std::size_t>(b);
	}

	////////////////////////////////////////////////////////

	struct DebugData;

	template<typename T>
	struct OperatorDebug;
	//{
	//	static void call(const T& t, DebugData& data) {}
	//};

	/////////////////////////////////////////////////////////

	struct ImGuiData;

	template<typename T>
	struct OperatorImGui;
	//{
	//	static void call(const T& t, ImGuiData& data) {}
	//};

	//////////////////////////////////////////////////////////

	struct GLSLData;

	template<typename T>
	struct OperatorGLSL;
	//{
	//	static void call(const T& t, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) {}
	//};

	//////////////////////////////////////////////////////////

	struct OperatorBase;

	//using Expr = std::shared_ptr<OperatorBase>;

	struct ExprOld
	{
		enum Status : std::uint8_t { Empty, Static, Default };

		ExprOld() = default;
		ExprOld(const std::size_t id, const std::size_t band) : m_id(static_cast<std::uint32_t>(id)), m_band(static_cast<uint8_t>(band)), m_status(Default) { }

		operator bool() const {
			return m_status != Empty;
		}

		std::uint32_t m_id = 0;
		std::uint8_t m_band = 0;
		Status m_status = Empty;
	};

	struct Expr
	{
		enum Status : std::uint8_t { Empty, Static, Default };

		Expr() = default;
		Expr(const std::size_t id) : m_id(static_cast<std::uint32_t>(id)), m_status(Default) { }

		operator bool() const {
			return m_status != Empty;
		}

		std::uint32_t m_id = 0;
		Status m_status = Empty;
	};

	OperatorBase* retrieve_expr(const Expr index);

	enum class CtorFlags : std::size_t {
		Declaration = 1 << 1,
		Initialisation = 1 << 2,
		Temporary = 1 << 3,
		Unused = 1 << 4,
		FunctionArgument = 1 << 5
	};

	constexpr bool operator&(CtorFlags a, CtorFlags b) {
		return static_cast<bool>(static_cast<std::size_t>(a)& static_cast<std::size_t>(b));
	}

	std::ostream& operator<<(std::ostream& t, const CtorFlags op) {
		return t << static_cast<std::size_t>(op);
	}

	struct OperatorBase {
		virtual ~OperatorBase() = default;

		virtual void print_spirv() const {}
		virtual void print_debug(DebugData& data) const = 0;
		virtual void print_imgui(ImGuiData& data) const = 0;
		virtual void print_glsl(GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) const = 0;

	};

	struct Dummy { };

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args);

	template<typename Delayed>
	struct ReferenceDelayed : OperatorBase
	{
		virtual ~ReferenceDelayed() = default;

		ReferenceDelayed(const std::size_t id) : m_id(id) {}

		void print_debug(DebugData& data) const override {
			OperatorDebug<Reference>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Reference>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<Reference>::call(*this, data, precedence);
		}

		std::size_t m_id;
	};
	using Reference = ReferenceDelayed<Dummy>;

	template<std::size_t N>
	struct ArgSeq
	{
		template<typename ...Args>
		ArgSeq(Args&& ... args) : m_args{ std::forward<Args>(args)... } { }

		const std::array<Expr, N> m_args;
	};


	template<std::size_t N>
	struct FunCall final : OperatorBase, ArgSeq<N>
	{
		template<typename ...Args>
		FunCall(const Op op, Args&& ...args) : ArgSeq<N>(std::forward<Args>(args)...), m_op(op) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<FunCall>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<FunCall>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<FunCall>::call(*this, data, precedence);
		}

		Op m_op;
	};

	template <typename ... Args>
	Expr make_funcall(const Op op, Args&& ...args)
	{
		return listen().current_shader->m_memory_pool->emplace_back<FunCall<sizeof...(Args)>>(op, std::forward<Args>(args)...);
	}

	struct ConstructorBase : OperatorBase
	{
		ConstructorBase(const std::string& name, const CtorFlags flags, const std::size_t variable_id)
			: m_name(name), m_variable_id(variable_id), m_flags(flags) {}

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
		virtual std::size_t arg_count() const {
			return 0;
		}

		std::string m_name;
		std::size_t m_variable_id;
		CtorFlags m_flags;
	};

	template<typename T, std::size_t N>
	struct Constructor final : ConstructorBase, ArgSeq<N>
	{

		using ArgSeq<N>::m_args;

		template<typename ...Args>
		Constructor(const std::string& name, const CtorFlags flags, const std::size_t variable_id, Args&& ...args)
			: ConstructorBase(name, flags, variable_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		Expr first_arg() const override {
			if constexpr (N == 0) {
				return {};
			} else {
				return ArgSeq<N>::m_args[0];
			}
		}

		std::size_t arg_count() const override {
			return N;
		}

		void print_debug(DebugData& data) const override {
			OperatorDebug<Constructor>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Constructor>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<Constructor>::call(*this, data, precedence);
		}
	};

	template<typename Delayed>
	struct ArraySubscriptDelayed final : OperatorBase
	{
		void print_debug(DebugData& data) const override {
			OperatorDebug<ArraySubscript>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<ArraySubscript>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<ArraySubscript>::call(*this, data, precedence);
		}

		Expr m_obj, m_index;
	};
	using ArraySubscript = ArraySubscriptDelayed<Dummy>;

	struct SwizzlingBase : OperatorBase
	{
		virtual ~SwizzlingBase() = default;

		SwizzlingBase(const Expr& expr) : m_obj(expr) { }

		virtual void print_debug(DebugData& data) const {}
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data, const Precedence precedence) const {}

		Expr m_obj;
	};

	template<typename S>
	struct Swizzling final : SwizzlingBase
	{
		Swizzling(const Expr& expr) : SwizzlingBase(expr) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<Swizzling<S>>::call(*this, data);
		}
		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Swizzling<S>>::call(*this, data);
		}
		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<Swizzling<S>>::call(*this, data, precedence);
		}
	};

	struct MemberAccessorBase : OperatorBase
	{
		virtual ~MemberAccessorBase() = default;

		virtual void print_debug(DebugData& data) const {}
		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data, const Precedence precedence) const {}

		MemberAccessorBase(const Expr& expr) : m_obj(expr) { }

		void set_as_temp();

		Expr m_obj;
	};

	template<typename S, std::size_t MemberId>
	struct MemberAccessor final : MemberAccessorBase {

		MemberAccessor(const Expr& expr) : MemberAccessorBase(expr) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<MemberAccessor<S, MemberId>>::call(*this, data);
		}
		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<MemberAccessor<S, MemberId>>::call(*this, data);
		}
		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<MemberAccessor<S, MemberId>>::call(*this, data, precedence);
		}
	};

	inline void MemberAccessorBase::set_as_temp() {
		if (auto parent_ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(m_obj))) {
			parent_ctor->set_as_temp();
		} else {
			if (auto obj = dynamic_cast<MemberAccessorBase*>(retrieve_expr(m_obj))) {
				obj->set_as_temp();
			}
		}
	}

	template<typename Delayed>
	struct UnaryOperatorDelayed final : OperatorBase
	{
		UnaryOperatorDelayed(const Op op, const Expr& arg) : m_arg(arg), m_op(op) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<UnaryOperator>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<UnaryOperator>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<UnaryOperator>::call(*this, data, precedence);
		}

		Expr m_arg;
		Op m_op;

	};
	using UnaryOperator = UnaryOperatorDelayed<Dummy>;

	template<typename Delayed>
	struct BinaryOperatorDelayed final : OperatorBase
	{
		BinaryOperatorDelayed(const Op op, const Expr& lhs, const Expr& rhs) : m_lhs(lhs), m_rhs(rhs), m_op(op) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<BinaryOperator>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<BinaryOperator>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<BinaryOperator>::call(*this, data, precedence);
		}

		Expr m_lhs, m_rhs;
		Op m_op;
	};
	using BinaryOperator = BinaryOperatorDelayed<Dummy>;

	template<typename From, typename To>
	struct ConvertorOperator final : OperatorBase, ArgSeq<1>
	{
		ConvertorOperator(const Expr& obj) : ArgSeq<1>(obj) {}

		void print_debug(DebugData& data) const override {
			OperatorDebug<ConvertorOperator>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<ConvertorOperator>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<ConvertorOperator>::call(*this, data, precedence);
		}
	};

	template<typename F, typename ReturnType, std::size_t N>
	struct CustomFunCall final : Reference, ArgSeq<N>
	{
		template<typename ... Args>
		CustomFunCall(const std::size_t fun_id, Args&& ...args) : Reference(fun_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<CustomFunCall>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<CustomFunCall>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<CustomFunCall>::call(*this, data, precedence);
		}
	};

	template<typename T>
	struct Litteral final : OperatorBase
	{
		Litteral(const T t) : value(t) { }

		void print_debug(DebugData& data) const override {
			OperatorDebug<Litteral>::call(*this, data);
		}

		void print_imgui(ImGuiData& data) const override {
			OperatorImGui<Litteral>::call(*this, data);
		}

		void print_glsl(GLSLData& data, const Precedence precedence) const override {
			OperatorGLSL<Litteral>::call(*this, data, precedence);
		}

		const T value;
	};


	template<typename Base, std::size_t MaxSizeof, std::size_t MaxAlignment>
	struct PolymorphicVector
	{

	public:
		PolymorphicVector() {
			m_buffer.reserve(2000);
		}

		template<typename Derived, typename ... Args>
		Base* emplace_back(Args&& ...args) {
			static_assert(std::is_base_of_v<Base, Derived>, "Derived should inherit from Base");
			static_assert(sizeof(Derived) <= MaxSizeof, "Derived sizeof is too big");
			static_assert(alignof(Derived) <= MaxAlignment, "Derived alignement is too big");

			m_buffer.push_back({});
			new (&m_buffer.back()) Derived(std::forward<Args>(args)...);
			return std::launder(reinterpret_cast<Base*>(&m_buffer.back()));
		}

		Base& operator[](const std::size_t index) {
			return *std::launder(reinterpret_cast<Base*>(&m_buffer[index]));
		}

		const Base& operator[](const std::size_t index) const {
			return *std::launder(reinterpret_cast<const Base*>(&m_buffer[index]));
		}

		~PolymorphicVector() {
			for (std::size_t i = m_buffer.size(); i > 0; --i) {
				operator[](i - 1).~Base();
			}
		}

		std::size_t size() const {
			return m_buffer.size();
		}

	private:
		std::vector<std::aligned_storage_t<MaxSizeof, MaxAlignment>> m_buffer;
	};

	template<typename Base, std::size_t BandSize, std::size_t MaxSizeof, std::size_t MaxAlignment>
	struct PolymorphicMemoryManager {

		using IndexElt = std::size_t;
		using Index = ExprOld; // std::pair<IndexElt, IndexElt>;

		constexpr std::size_t get_element_size(const std::size_t band_id) {
			return (1 + band_id) * BandSize;
		}

		template<typename Derived, typename ... Args>
		Index emplace_back(Args&& ...args) {
			static_assert(std::is_base_of_v<Base, Derived>, "Derived should inherit from Base");
			static_assert(sizeof(Derived) <= MaxSizeof, "Derived sizeof is too big");
			static_assert(alignof(Derived) <= MaxAlignment, "Derived alignement is too big");

			constexpr IndexElt band = (sizeof(Derived) - 1) / BandSize;
			constexpr std::size_t derived_max_size = (1 + band) * BandSize;

			const std::size_t current_size = m_buffers[band].size();
			m_buffers[band].resize(current_size + derived_max_size);
			new (&m_buffers[band][current_size]) Derived(std::forward<Args>(args)...);
			return Index{ current_size, band };
		}

		Base& operator[](const Index pair) {
			return *std::launder(reinterpret_cast<Base*>(&m_buffers[pair.m_band][pair.m_id]));
		}

		const Base& operator[](const Index pair) const {
			return *std::launder(reinterpret_cast<const Base*>(&m_buffers[pair.m_band][pair.m_id]));
		}

		~PolymorphicMemoryManager() {
			for (std::size_t i = 0; i < m_buffers.size(); ++i) {
				const std::size_t item_size = (i + 1) * BandSize;
				for (std::size_t j = 0; j < m_buffers[i].size(); j += item_size) {
					operator[](Index{ j, i }).~Base();
				}
			}
		}

	public:
		static constexpr std::size_t BandCount = 1 + (std::max(MaxSizeof, static_cast<std::size_t>(1)) - 1) / BandSize;
		std::array<std::vector<char>, BandCount> m_buffers;
	};

	template<std::size_t N>
	struct CanFit {
		using Type = std::conditional_t<
			(N <= 8),
			uint8_t,
			std::conditional_t<
			(N <= 16),
			uint16_t,
			std::conditional_t<
			(N <= 32),
			uint32_t,
			uint64_t
			>
			>
		>;
	};

	template<typename Base>
	struct PolymorphicMemoryPool {

		using Index = Expr;
		//using DeltaOffsetType = typename CanFit<MaxSizeof>::Type;

		template<typename Derived, typename ... Args>
		Index emplace_back(Args&& ...args) {
			static_assert(std::is_base_of_v<Base, Derived>, "Derived should inherit from Base");
			//static_assert(sizeof(Derived) <= MaxSizeof, "Derived sizeof is too big");

			constexpr std::size_t derived_size = sizeof(Derived);
			constexpr std::size_t derived_alignment = alignof(Derived);

			std::size_t current_size = m_buffer.size();
			const char* data = m_buffer.data() + current_size;
			current_size += reinterpret_cast<std::uintptr_t>(data) % derived_alignment;
			m_buffer.resize(current_size + derived_size);
			m_objects_ids.push_back(current_size);
			new (&m_buffer[current_size]) Derived(std::forward<Args>(args)...);
			return Index{ current_size };
		}

		Base& operator[](const Index index) {
			return *std::launder(reinterpret_cast<Base*>(&m_buffer[index.m_id]));
		}

		const Base& operator[](const Index index) const {
			return *std::launder(reinterpret_cast<const Base*>(&m_buffer[index.m_id]));
		}

		~PolymorphicMemoryPool() {
			for (const std::size_t id : m_objects_ids) {
				operator[](id).~Base();
			}
		}

	public:
		std::vector<char> m_buffer;
		std::vector<std::size_t> m_objects_ids;
	};
}

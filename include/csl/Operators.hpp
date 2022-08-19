#pragma once

#include <csl/TemplateHelpers.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#if __cplusplus >= 201703L
#define CSL_LAUNDER(...) __VA_ARGS__ // std::launder(__VA_ARGS__)
#else 
#define CSL_LAUNDER(...) __VA_ARGS__
#endif

namespace csl
{
	enum class Op : std::size_t
	{
		// algebra operators
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
		DivAssignment,
		MulAssignment,
		SubAssignment,
		BitwiseAndAssignment,
		ScalarLessThanScalar,
		ScalarLessThanEqualScalar,
		ScalarGreaterThanScalar,
		ScalarGreaterThanEqualScalar,
		LogicalOr,
		LogicalAnd,
		BitwiseAnd,
		BitwiseOr,
		BitwiseLeftShift,
		BitwiseRightShift,
		PostfixIncrement,
		PrefixIncrement,
		PostfixDecrement,
		PrefixDecrement,
		Equality,
		NotEquality,

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
		reflect,

		// glsl 1.2
		transpose,

		// glsl 1.3
		round,
		sign,
		texture,

		// glsl 1.4
		inverse,

		// glsl 4.0
		bitfieldExtract,

		// glsl 4.2
		imageStore,

		// glsl 4.3
		imageSize,
	};

	enum class Precedence : std::size_t
	{
		Alias = 10,

		ArraySubscript = 19,
		FunctionCall = 19,
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

		NoExtraParenthesis = Sequence + 1,
	};

	constexpr bool operator<(const Precedence a, const Precedence b) {
		return static_cast<std::size_t>(a) < static_cast<std::size_t>(b);
	}

	struct ExpressionHandle
	{
		static constexpr std::uint32_t Defined = 1 << 31;
		static constexpr std::uint32_t Static = 1 << 30;
		static constexpr std::uint32_t IndexMask = ~(Defined | Static);

		ExpressionHandle() = default;

		ExpressionHandle(const std::uint32_t index)
			: m_id{ index | Defined }
		{
			// assert(index == get_index());
		}

		explicit operator bool() const {
			return m_id & Defined;
		}

		std::uint32_t get_index() const {
			return m_id & IndexMask;
		}

		bool is_static() const {
			return m_id & Static;
		}

		std::uint32_t m_id = 0;
	};
	using Expr = ExpressionHandle;

	struct OperatorBase;
	OperatorBase* retrieve_expr(const Expr index);

	enum class CtorFlags : std::size_t
	{
		Declaration = 1 << 0,
		Initialisation = 1 << 1,
		Temporary = 1 << 2,
		Unused = 1 << 3,
		FunctionArgument = 1 << 4,
		Untracked = 1 << 5,
		Const = 1 << 6,
		SwitchMask = Declaration | Initialisation | Temporary | Unused | FunctionArgument
	};

	constexpr CtorFlags operator&(const CtorFlags a, const CtorFlags b) {
		return static_cast<CtorFlags>(static_cast<std::size_t>(a) & static_cast<std::size_t>(b));
	}
	constexpr CtorFlags operator|(const CtorFlags a, const CtorFlags b) {
		return static_cast<CtorFlags>(static_cast<std::size_t>(a) | static_cast<std::size_t>(b));
	}
	constexpr CtorFlags operator~(const CtorFlags a) {
		return static_cast<CtorFlags>(~static_cast<std::size_t>(a));
	}
	constexpr CtorFlags& operator|=(CtorFlags& a, const CtorFlags b) {
		a = a | b;
		return a;
	}

	//////////////////////////////////////////////////////////

	struct GLSLData;

	template<typename T>
	void to_glsl(const T& t, GLSLData& data);

	/////////////////////////////////////////////////////////

	struct ImGuiData;

	template<typename T>
	void to_imgui(const T& t, ImGuiData& data);

	inline GLSLData& get_glsl_data(ImGuiData& data);

	//////////////////////////////////////////////////////////

	struct OperatorBase
	{
		virtual ~OperatorBase() = default;

		virtual void print_spirv() const {}
		virtual void print_imgui(ImGuiData& data) const = 0;
		virtual void print_glsl(GLSLData& data) const = 0;
	};

	template <typename Operator, typename ... Args>
	Expr make_expr(Args&& ...args);

	struct Reference : OperatorBase
	{
		virtual ~Reference() = default;

		Reference(const std::size_t id) : m_id(id) {}

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		const std::size_t m_id;
	};

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

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		const Op m_op;
	};

	template <typename ... Args>
	Expr make_funcall(const Op op, Args&& ...args);

	struct ConstructorBase : OperatorBase
	{
		virtual ~ConstructorBase() = default;

		ConstructorBase(const std::string& name, const CtorFlags flags, const std::size_t variable_id)
			: m_name(name), m_variable_id(variable_id), m_flags(flags) {}

		void set_as_temp() {
			m_flags = CtorFlags::Temporary;
		}

		void set_as_unused() {
			if (bool(m_flags & CtorFlags::Initialisation))
				m_flags = CtorFlags::Unused;
		}

		void set_as_const() {
			m_flags |= CtorFlags::Const;
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

	template<typename T, std::size_t N, typename Dimensions, typename Qualifiers>
	struct Constructor final : ConstructorBase, ArgSeq<N>
	{
		template<typename ...Args>
		Constructor(const std::string& name, const CtorFlags flags, const std::size_t variable_id, Args&& ...args)
			: ConstructorBase(name, flags, variable_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		Expr first_arg() const override {
			if constexpr (N == 0)
				return {};
			else
				return ArgSeq<N>::m_args[0];
		}

		std::size_t arg_count() const override {
			return N;
		}

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	struct ArraySubscript final : OperatorBase
	{
		ArraySubscript(const Expr obj, const Expr index) 
			: m_obj(obj), m_index(index) {}

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_obj, m_index;
	};

	struct SwizzlingBase : OperatorBase
	{
		virtual ~SwizzlingBase() = default;

		SwizzlingBase(const Expr& expr) : m_obj(expr) { }

		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

		Expr m_obj;
	};

	template<char ...chars>
	struct Swizzling final : SwizzlingBase
	{
		Swizzling(const Expr expr) : SwizzlingBase(expr) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}
		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	struct MemberAccessorBase : OperatorBase
	{
		virtual ~MemberAccessorBase() = default;

		virtual void print_imgui(ImGuiData& data) const {}
		virtual void print_glsl(GLSLData& data) const {}

		MemberAccessorBase(const Expr expr) : m_obj(expr) { }

		void set_as_temp()
		{
			OperatorBase* op_base = retrieve_expr(m_obj);

			if (auto parent_ctor = dynamic_cast<ConstructorBase*>(op_base))
				parent_ctor->set_as_temp();
			else
				safe_static_cast<MemberAccessorBase*>(op_base)->set_as_temp();
		}

		Expr m_obj;
	};

	template<typename S, std::size_t MemberId>
	struct MemberAccessor final : MemberAccessorBase
	{
		MemberAccessor(const Expr expr) : MemberAccessorBase(expr) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}
		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	struct UnaryOperator final : OperatorBase
	{
		UnaryOperator(const Op op, const Expr& arg) : m_arg(arg), m_op(op) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_arg;
		Op m_op;
	};

	struct BinaryOperator final : ArgSeq<2>, OperatorBase
	{
		BinaryOperator(const Op op, const Expr& lhs, const Expr& rhs) : m_lhs(lhs), m_rhs(rhs), m_op(op) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_lhs, m_rhs;
		Op m_op;
	};

	struct TernaryOperator final : OperatorBase
	{
		TernaryOperator(const Expr condition, const Expr first, const Expr second)
			: m_condition(condition), m_first(first), m_second(second) {
		}

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_condition, m_first, m_second;
	};

	template<typename From, typename To>
	struct ConvertorOperator final : OperatorBase, ArgSeq<1>
	{
		ConvertorOperator(const Expr& obj) : ArgSeq<1>(obj) {}

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	template<typename F, typename ReturnType, std::size_t N>
	struct CustomFunCall final : Reference, ArgSeq<N>
	{
		template<typename ... Args>
		CustomFunCall(const std::size_t fun_id, Args&& ...args) : Reference(fun_id), ArgSeq<N>(std::forward<Args>(args)...) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	template<typename T>
	struct Litteral final : OperatorBase
	{
		Litteral(const T t) : value(t) { }

		void print_imgui(ImGuiData& data) const override {
			to_imgui(*this, data);
		}

		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		const T value;
	};

	template<typename Base>
	struct PolymorphicMemoryPool
	{
		PolymorphicMemoryPool()
		{
			m_objects_offsets.reserve(124);
			m_buffer.reserve(10000);
		}

		PolymorphicMemoryPool(PolymorphicMemoryPool&& other) noexcept
			: m_buffer(std::move(other.m_buffer)), m_objects_offsets(std::move(other.m_objects_offsets))
		{
		}

		PolymorphicMemoryPool& operator=(PolymorphicMemoryPool&& other) noexcept
		{
			m_buffer = std::move(other.m_buffer);
			m_objects_offsets = std::move(other.m_objects_offsets);
			return *this;
		}

		template<typename Derived, typename ... Args>
		Expr emplace_back(Args&& ...args)
		{
			static_assert(std::is_base_of_v<Base, Derived>, "Derived should inherit from Base");

			std::size_t current_size = m_buffer.size();
			const char* data = m_buffer.data() + current_size;
			current_size += reinterpret_cast<std::uintptr_t>(data) % alignof(Derived);
			m_buffer.resize(current_size + sizeof(Derived));
			m_objects_offsets.push_back(current_size);
			new (&m_buffer[current_size]) Derived(std::forward<Args>(args)...);
			return Expr(static_cast<std::uint32_t>(current_size));
		}

		Base& operator[](const Expr index) {
			return *CSL_LAUNDER(reinterpret_cast<Base*>(&m_buffer[index.get_index()]));
		}

		const Base& operator[](const Expr index) const {
			return *CSL_LAUNDER(reinterpret_cast<const Base*>(&m_buffer[index.get_index()]));
		}

		~PolymorphicMemoryPool() {
			for (const std::size_t index : m_objects_offsets) {
				Base& obj = reinterpret_cast<Base&>(m_buffer[index]);
				obj.~Base();
			}
		}

		std::size_t get_data_size() const {
			return m_buffer.size() + sizeof(std::size_t) * m_objects_offsets.size();
		}

	public:
		std::vector<char> m_buffer;
		std::vector<std::size_t> m_objects_offsets;
	};

	template<typename Base>
	struct PolymorphicMemoryPoolDebug
	{
		template<typename Derived, typename ... Args>
		Expr emplace_back(Args&& ...args) {
			const std::size_t current_size = m_buffer.size();
			m_buffer.emplace_back(std::make_unique<Derived>(std::forward<Args>(args)...));
			m_objects_offsets.push_back(current_size);
			m_data_size += sizeof(std::unique_ptr<Base>) + sizeof(Derived);
			return Expr(static_cast<std::uint32_t>(current_size));
		}

		Base& operator[](const Expr index) {
			return *m_buffer[index.get_index()];
		}

		const Base& operator[](const Expr index) const {
			return *m_buffer[index.get_index()];
		}

		std::size_t get_data_size() const {
			return m_data_size;
		}

		std::vector<std::unique_ptr<Base>> m_buffer;
		std::vector<std::size_t> m_objects_offsets;

		std::size_t m_data_size = 0;
	};
}

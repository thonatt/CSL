#pragma once

#include "../InstructionTree.hpp"
#include "../Preprocessor.hpp"

#include "Shaders.hpp"

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

namespace v2 {

	const std::string& glsl_op_str(const Op op);

	struct GLSLData {
		std::stringstream stream;

		std::unordered_map<std::size_t, std::size_t> global_to_local;
		std::unordered_map<std::size_t, std::string> func_names;
		std::unordered_map<std::size_t, std::string> var_names;

		int trailing = 0;

		GLSLData& trail() {
			for (int t = 0; t < trailing; ++t) {
				stream << "    ";
			}
			return *this;
		}

		GLSLData& endl() {
			stream << "\n";
			return *this;
		}

		template<typename T>
		GLSLData& operator<<(T&& t) {
			stream << std::forward<T>(t);
			return *this;
		}

		void check_for_parenthesis(const Expr& expr, const Precedence parent, const Precedence current)
		{
			const bool inversion = (parent < current);
			if (inversion) {
				stream << "(";
			}
			retrieve_expr(expr)->print_glsl(*this, current);
			if (inversion) {
				stream << ")";
			}
		}

		const std::string& register_var_name(const std::string& name, const std::size_t id)
		{
			return var_names.emplace(id, name.empty() ? "x" + std::to_string(var_names.size()) : name).first->second;
		}

		template<typename ...Ts>
		void register_builtins(const Ts&... vars) {
			([this](const auto& var) {
				auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(var.get_plain_expr()));
				assert(ctor);
				register_var_name(ctor->m_name, ctor->m_variable_id);
			}(vars), ...);
		}
	};

	template<typename Delayed>
	struct ControllerGLSL<Delayed, ShaderController> {
		static void call(const ShaderController& controller, GLSLData& data) {

			for (const auto& s : controller.m_structs) {
				retrieve_instruction(s)->print_glsl(data);
				data.endl();
			}

			for (const auto& i : controller.m_named_interface_blocks) {
				retrieve_instruction(i)->print_glsl(data);
				data.endl();
			}

			for (const auto& i : controller.m_unnamed_interface_blocks) {
				retrieve_instruction(i)->print_glsl(data);
				data.endl();
			}

			for (const auto& i : controller.m_declarations->m_instructions) {
				retrieve_instruction(i)->print_glsl(data);
			}
			data.endl();

			for (const auto& f : controller.m_functions) {
				retrieve_instruction(f)->print_glsl(data);
			}
		}
	};

	// operators infos 

	struct OpInfos {
		Precedence m_precendence;
		std::string m_str;
	};

#define MAKE_OP_IT(r, data, i, elem) {  Op :: elem, { Precedence :: FunctionCall, CSL_PP2_STR(elem) }},

	inline const std::unordered_map<Op, OpInfos>& glsl_op_infos() {
		static const std::unordered_map<Op, OpInfos> op_infos = {
			{ Op::CWiseMul, { Precedence::Multiply, "*" } },
			{ Op::MatrixTimesScalar, { Precedence::Multiply, "*" }  },
			{ Op::MatrixTimesMatrix, { Precedence::Multiply, "*" } },
			{ Op::ScalarTimesMatrix, { Precedence::Multiply, "*" }  },
			{ Op::CWiseDiv, { Precedence::Division, "/" } },
			{ Op::MatrixDivScalar, { Precedence::Division, "/" } },
			{ Op::ScalarDivMatrix, { Precedence::Division, "/" } },
			{ Op::CWiseAdd, { Precedence::Addition, " + "} },
			{ Op::MatrixAddScalar, { Precedence::Addition, " + "}},
			{ Op::ScalarAddMatrix, { Precedence::Addition, " + "} },
			{ Op::CWiseSub, { Precedence::Substraction, " - " } },
			{ Op::MatrixSubScalar, { Precedence::Substraction, " - " }},
			{ Op::ScalarSubMatrix, { Precedence::Substraction, " - " } },
			{ Op::UnaryNegation, { Precedence::Unary, "!" }},
			{ Op::UnarySub, { Precedence::Unary, "-" }},
			{ Op::Assignment, { Precedence::Assignment, " = "} },
			{ Op::AddAssignment, { Precedence::Assignment, " += "} },
			{ Op::SubAssignment, { Precedence::Assignment, " -= "} },
			{ Op::MulAssignment, { Precedence::Assignment, " *= "} },
			{ Op::DivAssignment, { Precedence::Assignment, " /= "} },
			{ Op::ScalarLessThanScalar, { Precedence::Relational, " < "} },
			{ Op::ScalarGreaterThanScalar, { Precedence::Relational, " > "} },
			{ Op::LogicalOr, { Precedence::LogicalOr, " || "} },
			{ Op::LogicalAnd, { Precedence::LogicalAnd, " && "} },
			{ Op::PostfixUnary, { Precedence::Postfix, "++"} },
			{ Op::PrefixUnary, { Precedence::Prefix, "++"} },
			CSL_PP2_ITERATE(MAKE_OP_IT,
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
			texture,
			pow,
			cross,
			inverse,
			transpose)
		};

		return op_infos;
	}

#undef MAKE_OP_IT

	inline const std::string& glsl_op_str(const Op op) {
		auto it = glsl_op_infos().find(op);
		if (it == glsl_op_infos().end()) {
			static const std::string undefined = " undefined Op ";
			return undefined;
		} else {
			return it->second.m_str;
		}
	}

	inline Precedence glsl_op_precedence(const Op op) {
		auto it = glsl_op_infos().find(op);
		if (it == glsl_op_infos().end()) {
			return Precedence::Alias;
		} else {
			return it->second.m_precendence;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct GLSLQualifier {
		static std::string get() {
			return typeid(QList).name();
		}
	};

	template<typename ...Qs>
	struct GLSLQualifier<glsl::Layout<Qs...>> {
		static std::string get() {
			std::string s = "layout(";
			((s += GLSLQualifier<Qs>::get()), ...);
			s += ")";
			return s;
		}
	};

	template<std::size_t N>
	struct GLSLQualifier<glsl::Location<N>> {
		static std::string get() {
			return "location = " + std::to_string(N);
		}
	};

	template<std::size_t N>
	struct GLSLQualifier<glsl::Binding<N>> {
		static std::string get() {
			return "binding = " + std::to_string(N);
		}
	};

#define CSL_QUALIFIER_STR_IT(name, str) \
	template<> inline std::string GLSLQualifier< glsl :: name>::get() { return CSL_PP2_STR(str); }

	CSL_QUALIFIER_STR_IT(Uniform, uniform);
	CSL_QUALIFIER_STR_IT(In, in);
	CSL_QUALIFIER_STR_IT(Out, out);

	template<typename Q, typename ...Qs>
	struct GLSLQualifier<TList<Q, Qs...>> {
		static const std::string& get() {
			static const std::string str = [] {
				std::string s;
				s += GLSLQualifier<Q>::get();
				((s += " " + GLSLQualifier<Qs>::get()), ...);
				return s;
			}();
			return str;
		}
	};

	template<typename>
	struct ArraySizePrinterGLSL { };

	template<size_t ...Ns>
	struct ArraySizePrinterGLSL<SizeList<Ns...>> {
		static const std::string& get() {
			static const std::string str = [] {
				std::string s;
				((s += "[" + std::to_string(Ns) + "]"), ...);
				return s;
			}();
			return str;
		}
	};

	template<typename T> struct TypePrefixStr {
		static std::string get();
	};

	template<> inline std::string TypePrefixStr<bool>::get() { return "b"; }
	template<> inline std::string TypePrefixStr<int>::get() { return "i"; }
	template<> inline std::string TypePrefixStr<unsigned int>::get() { return "u"; }
	template<> inline std::string TypePrefixStr<float>::get() { return ""; }
	template<> inline std::string TypePrefixStr<double>::get() { return "d"; }

	template<typename T>
	struct GLSLTypeStr {
		static std::string get() { return T::get_type_str(); }
	};

	template<> inline std::string GLSLTypeStr<void>::get() { return "void"; }
	template<> inline std::string GLSLTypeStr<bool>::get() { return "bool"; }
	template<> inline std::string GLSLTypeStr<int>::get() { return "int"; }
	template<> inline std::string GLSLTypeStr<unsigned int>::get() { return "uint"; }
	template<> inline std::string GLSLTypeStr<float>::get() { return "float"; }
	template<> inline std::string GLSLTypeStr<double>::get() { return "double"; }

	template<typename T>
	struct GLSLTypeStr<Scalar<T>> {
		static std::string get() {
			return GLSLTypeStr<T>::get();
		}
	};

	template<typename T, std::size_t R>
	struct GLSLTypeStr<Vector<T, R>> {
		static const std::string& get() {
			static const std::string type_str = [] {
				return TypePrefixStr<T>::get() + "vec" + std::to_string(R);
			}();
			return type_str;
		}
	};

	template<typename T, std::size_t R, std::size_t C>
	struct GLSLTypeStr<Matrix<T, R, C>> {
		static const std::string& get() {
			static const std::string type_str = [] {
				return TypePrefixStr<T>::get() + "mat" + std::to_string(C) + (C == R ? "" : "x" + std::to_string(R));
			}();
			return type_str;
		}
	};

	template<SamplerAccessType Access, typename T, std::size_t N, SamplerType Type, SamplerFlags Flags>
	struct GLSLTypeStr<Sampler<Access, T, N, Type, Flags>> {
		static const std::string& get() {
			static const std::unordered_map<SamplerAccessType, std::string> sampler_access_strs = {
				{ SamplerAccessType::Sampler, "sampler" },
				{ SamplerAccessType::Image, "image"}
			};

			static const std::unordered_map<SamplerType, std::string> sampler_type_strs = {
				{ SamplerType::Basic, "" },
				{ SamplerType::Cube, "Cube"},
				{ SamplerType::Rectangle , "Rect"},
				{ SamplerType::MultiSample, "MS"},
				{ SamplerType::Buffer, "Buffer"},
			};

			static const std::string type_str = [] {
				return TypePrefixStr<T>::get() +
					sampler_access_strs.find(Access)->second +
					(N != 0 ? std::to_string(N) + 'D' : "") +
					sampler_type_strs.find(Type)->second +
					(Flags & SamplerFlags::Array ? "Array" : "") +
					(Flags & SamplerFlags::Shadow ? "Shadow" : "");
			}();
			return type_str;
		}
	};

	template<>
	struct GLSLTypeStr<glsl::atomic_uint>
	{
		static const std::string& get() {
			static const std::string type_str = "atomic_uint";
			return type_str;
		}
	};

	template<typename T, typename ...Qs>
	struct GLSLTypeStr<TypeInterface<T, Qs...>> {
		static std::string get() {
			return GLSLTypeStr<T>::get();
		}
	};

	template<typename T, typename Ds, typename ...Qs>
	struct GLSLTypeStr<ArrayInterface<T, Ds, Qs...>> {
		static std::string get() {
			return GLSLTypeStr<T>::get();
		}
	};

	template<typename T>
	struct GLSLDeclaration {
		static std::string get(const std::string& var_name) {
			using ArrayDimensions = typename T::ArrayDimensions;
			using Qualifiers = typename T::Qualifiers;
			using U = typename T::QualifierFree;

			std::string str;

			if constexpr (Qualifiers::Size > 0) {
				str += GLSLQualifier<Qualifiers>::get() + " ";
			}

			str += GLSLTypeStr<U>::get() + " " + var_name;

			if constexpr (ArrayDimensions::Size > 0) {
				str += " " + ArraySizePrinterGLSL<ArrayDimensions>::get();
			}

			return str;
		}

	};

	///////////////////////////////////////////////////////////////////////////////////
	// instructions

	template<>
	struct InstructionGLSL<IfInstruction> {
		static void call(const IfInstruction& i, GLSLData& data) {

			for (std::size_t k = 0; k < i.m_cases.size(); ++k) {
				data.endl().trail();
				if (k == 0) {
					data << "if (";
					retrieve_expr(i.m_cases[k].condition)->print_glsl(data);
					data << ")";
				} else if (k != i.m_cases.size() - 1) {
					data << "else if (";
					retrieve_expr(i.m_cases[k].condition)->print_glsl(data);
					data << ")";
				} else {
					data << "else ";
				}
				data.endl().trail() << "{";
				++data.trailing;
				for (const auto& j : i.m_cases[k].body->m_instructions) {
					retrieve_instruction(j)->print_glsl(data);
				}
				--data.trailing;
				data.endl().trail() << "}";
			}

		}
	};

	template<>
	struct InstructionGLSL<WhileInstruction> {
		static void call(const WhileInstruction& i, GLSLData& data) {
			data.endl().trail() << "while(";
			retrieve_expr(i.m_condition)->print_glsl(data);
			data << ") {";
			++data.trailing;
			for (const auto& i : i.m_body->m_instructions) {
				retrieve_instruction(i)->print_glsl(data);
			}
			--data.trailing;
			data.endl().trail() << "}";
		}
	};

	template<>
	struct InstructionGLSL<ForInstruction> {
		static void call(const ForInstruction& i, GLSLData& data) {
			data.endl().trail() << "for(";
			for (const auto& j : i.args->m_instructions) {
				retrieve_instruction(j)->print_glsl(data);
			}
			data << ") {";
			++data.trailing;
			for (const auto& j : i.body->m_instructions) {
				retrieve_instruction(j)->print_glsl(data);
			}
			--data.trailing;
			data.endl().trail() << "}";
		}
	};

	template<>
	struct InstructionGLSL<Statement> {
		static void call(const Statement& i, GLSLData& data) {
			if (!i.m_expr) {
				data << "empty expr";
				return;
			}

			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(i.m_expr))) {
				if (ctor->m_flags & CtorFlags::Temporary) {
					return;
				}
				if (ctor->m_flags & CtorFlags::FunctionArgument) {
					retrieve_expr(i.m_expr)->print_glsl(data, Precedence::Alias);
					return;
				}
			}

			data.endl().trail();
			retrieve_expr(i.m_expr)->print_glsl(data, Precedence::Alias);
			data << ";";
		}
	};

	template<>
	struct InstructionGLSL<ForArgStatement> {
		static void call(const ForArgStatement& i, GLSLData& data) {
			if (i.m_expr) {
				retrieve_expr(i.m_expr)->print_glsl(data);
				data << "; ";
				return;
			}
		}
	};

	template<>
	struct InstructionGLSL<ForIterationStatement> {
		static void call(const ForIterationStatement& i, GLSLData& data) {
			if (i.m_expr) {
				retrieve_expr(i.m_expr)->print_glsl(data);
			}
		}
	};

	template<>
	struct InstructionGLSL<SwitchInstruction> {
		static void call(const SwitchInstruction& i, GLSLData& data) {
			data.endl().trail() << "Switch ";
			retrieve_expr(i.m_condition)->print_glsl(data, Precedence::Alias);
			++data.trailing;
			for (const auto& c : i.m_body->m_instructions) {
				retrieve_instruction(c)->print_glsl(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct InstructionGLSL<SwitchCase> {
		static void call(const SwitchCase& i, GLSLData& data) {
			data.endl().trail();
			if (i.m_label) {
				data << "Case ";
				retrieve_expr(i.m_label)->print_glsl(data, Precedence::Alias);
			} else {
				data << "Default";
			}
			++data.trailing;
			for (const auto& j : i.m_body->m_instructions) {
				retrieve_instruction(j)->print_glsl(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct InstructionGLSL<ReturnStatement> {
		static void call(const ReturnStatement& i, GLSLData& data) {
			data.endl().trail() << "return";
			if (i.m_expr) {
				data << " ";
				retrieve_expr(i.m_expr)->print_glsl(data);
			}
			data << ";";
		}
	};

	template<typename T>
	std::string SpecialStatementStr();

	template<>
	inline std::string SpecialStatementStr<Discard>() { return "discard"; }
	template<>
	inline std::string SpecialStatementStr<Break>() { return "break"; }
	template<>
	inline std::string SpecialStatementStr<Continue>() { return "continue"; }

	template<typename T>
	struct InstructionGLSL<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, GLSLData& data) {
			data.endl().trail() << SpecialStatementStr<T>() << ";";
		}
	};

	template<>
	struct InstructionGLSL<FuncDeclarationBase> {
		static void call(const FuncDeclarationBase& f, GLSLData& data) { }
	};

	template<std::size_t NumOverloads>
	struct OverloadGLSL {
		template<typename T, std::size_t Id>
		struct Get {
			static void call(const std::array<OverloadData, NumOverloads>& overloads, GLSLData& data, const std::string& fname) {
				data.endl().endl().trail() << GLSLTypeStr<T>::get() << " " << fname << "(";
				const auto& args = overloads[Id].args->m_instructions;
				if (get_arg_evaluation_order() == ArgEvaluationOrder::LeftToRight) {
					if (!args.empty()) {
						retrieve_instruction(args.front())->print_glsl(data);
					}
					for (std::size_t i = 1; i < args.size(); ++i) {
						data << ", ";
						retrieve_instruction(args[i])->print_glsl(data);
					}
				} else {
					if (!args.empty()) {
						retrieve_instruction(args.back())->print_glsl(data);
					}
					for (std::size_t i = 1; i < args.size(); ++i) {
						data << ", ";
						retrieve_instruction(args[args.size() - i - 1])->print_glsl(data);
					}
				}

				data << ")";
				data.endl().trail() << "{";
				++data.trailing;
				for (const auto& i : overloads[Id].body->m_instructions) {
					retrieve_instruction(i)->print_glsl(data);
				}
				--data.trailing;
				data.endl().trail() << "}";
			}
		};

	};

	template<typename ReturnTList, typename ...Fs>
	struct InstructionGLSL<FuncDeclaration<ReturnTList, Fs...>> {
		static void call(const FuncDeclaration<ReturnTList, Fs...>& f, GLSLData& data) {
			const std::string& name = data.register_var_name(f.m_name, f.m_id);
			iterate_over_typelist<ReturnTList, OverloadGLSL<sizeof...(Fs)>::template Get>(f.m_overloads, data, name);
		}
	};

	template<>
	struct InstructionGLSL<StructDeclarationBase> {
		static void call(const StructDeclarationBase& f, GLSLData& data) { }
	};

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMemberGLSL {
		static void call(GLSLData& data) {
			data.endl().trail() << GLSLTypeStr<T>::get() << " " << S::get_member_name(Id) << ";";
		}
	};

	template<typename S>
	struct InstructionGLSL<StructDeclaration<S>> {

		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberGLSL<S, T, Id>;

		static void call(const StructDeclaration<S>& s, GLSLData& data) {
			data.endl().trail() << "struct " << GLSLTypeStr<S>::get();
			data.endl().trail() << "{";
			++data.trailing;
			iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
			--data.trailing;
			data.endl().trail() << "}";
		}
	};

	template<typename Interface>
	struct InstructionGLSL<NamedInterfaceDeclaration<Interface>> {

		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberGLSL<Interface, T, Id>;

		using ArrayDimensions = typename Interface::ArrayDimensions;
		using Qualifiers = typename Interface::Qualifiers;

		static void call(const NamedInterfaceDeclaration<Interface>& s, GLSLData& data) {
			data.endl().trail();
			if constexpr (Qualifiers::Size > 0) {
				data << GLSLQualifier<Qualifiers>::get() + " ";
			}
			data << GLSLTypeStr<Interface>::get() << " {";
			++data.trailing;
			iterate_over_typelist<typename Interface::MemberTList, StructMemberDeclaration>(data);
			--data.trailing;
			data.endl().trail() << "} " << s.m_name;
			if constexpr (ArrayDimensions::Size > 0) {
				data << ArraySizePrinterGLSL<ArrayDimensions>::get();
			}
			data << ";";
		}
	};

	template<typename Interface, typename T, std::size_t Id>
	struct UnnamedInterfaceDeclarationMemberGLSL {
		static void call(const Interface& i, GLSLData& data) {
			data.endl().trail() << GLSLTypeStr<T>::get() << " " << i.m_names[1 + Id] << ";";
		}
	};

	template<typename ...Qs, typename ...Ts>
	struct InstructionGLSL<UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>> {

		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;
		using Qualifiers = RemoveArrayFromQualifiers<Qs...>;
		using Interface = UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>;

		template<typename T, std::size_t Id>
		using MemberDeclaration = UnnamedInterfaceDeclarationMemberGLSL<Interface, T, Id>;

		static void call(const Interface& s, GLSLData& data) {
			data.endl().trail();
			if constexpr (Qualifiers::Size > 0) {
				data << GLSLQualifier<Qualifiers>::get() + " ";
			}
			data << s.m_names[0] << " {";
			++data.trailing;
			iterate_over_typelist<TList<Ts...>, MemberDeclaration>(s, data);
			--data.trailing;
			data.endl().trail() << "};";
		}
	};


	//////////////////////////////////////////////////////
	// operators

	template<std::size_t N>
	struct OperatorGLSL<ArgSeq<N>> {
		static void call(const ArgSeq<N>& seq, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) {
			data << "(";
			if constexpr (N > 0) {
				retrieve_expr(seq.m_args[0])->print_glsl(data, precedence);
			}
			for (std::size_t i = 1; i < N; ++i) {
				data << ", ";
				retrieve_expr(seq.m_args[i])->print_glsl(data, precedence);
			}
			data << ")";
		}
	};

	template<>
	struct OperatorGLSL<Reference> {
		static void call(const Reference& ref, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) {
			const auto it = data.var_names.find(ref.m_id);
			if (it == data.var_names.end()) {
				data.stream << "unregistered var";
				return;
			}
			data.stream << data.var_names.find(ref.m_id)->second;
		}
	};

	template<>
	struct OperatorGLSL<ConstructorBase> {
		static void call(const ConstructorBase& ctor, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) {
			data << " base ctor";
		}
	};

	template<typename T, std::size_t N>
	struct OperatorGLSL<Constructor<T, N>> {

		static void call(const Constructor<T, N>& ctor, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis) {

			if (ctor.m_flags & CtorFlags::Const) {
				data << "const ";
			}

			const CtorFlags without_const = ctor.m_flags && (~CtorFlags::Const);
			switch (without_const)
			{
			case CtorFlags::Declaration: {
				data << GLSLDeclaration<T>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
				break;
			}
			case CtorFlags::Initialisation: {
				data << GLSLDeclaration<T>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
				data << " = ";
				if (ctor.arg_count() == 1) {
					retrieve_expr(ctor.first_arg())->print_glsl(data);
				} else {
					data << GLSLTypeStr<T>::get();
					OperatorGLSL<ArgSeq<N>>::call(ctor, data);
				}
				break;
			}
			case CtorFlags::Temporary: {
				data << GLSLTypeStr<T>::get();
				OperatorGLSL<ArgSeq<N>>::call(ctor, data);
				break;
			}
			case CtorFlags::Unused: {
				retrieve_expr(ctor.first_arg())->print_glsl(data);
				break;
			}
			case CtorFlags::FunctionArgument: {
				data << GLSLDeclaration<T>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
				break;
			}
			default:
				break;
			}
		}
	};

	template<>
	struct OperatorGLSL<ArraySubscript> {
		static void call(const ArraySubscript& subscript, GLSLData& data, const Precedence precedence) {
			retrieve_expr(subscript.m_obj)->print_glsl(data, Precedence::ArraySubscript);
			data << "[";
			retrieve_expr(subscript.m_index)->print_glsl(data, Precedence::ArraySubscript);
			data << "]";
		}
	};

	template<>
	struct OperatorGLSL<SwizzlingBase> {
		static void call(const SwizzlingBase& swizzle, GLSLData& data, const Precedence precedence) { }
	};

	template<char c, char ... chars>
	struct OperatorGLSL<Swizzling<Swizzle<c, chars...>>> {

		static void call(const Swizzling<Swizzle<c, chars...>>& swizzle, GLSLData& data, const Precedence precedence) {
			retrieve_expr(swizzle.m_obj)->print_glsl(data, Precedence::Swizzle);
			data << ".";
			data << c;
			((data << chars), ...);
		}
	};

	template<typename T>
	struct OperatorGLSL<Litteral<T>> {
		static void call(const Litteral<T>& litteral, GLSLData& data, const Precedence precedence) {
			if constexpr (std::is_same_v<T, bool>) {
				data << std::boolalpha << litteral.value;
			} else if constexpr (std::is_integral_v<T>) {
				data << litteral.value;
			} else {
				std::stringstream ss;
				ss << std::fixed << litteral.value;
				std::string s = ss.str();
				s.erase(s.find_last_not_of('0') + 1, std::string::npos);
				if (s.back() == '.') {
					s += '0';
				}
				data << s;
			}
		}
	};

	template<>
	struct OperatorGLSL<BinaryOperator> {
		static void call(const BinaryOperator& bop, GLSLData& data, const Precedence precedence) {
			const Precedence bop_precendence = glsl_op_precedence(bop.m_op);
			const bool inversion = (precedence < bop_precendence);
			if (inversion) {
				data << "(";
			}
			retrieve_expr(bop.m_lhs)->print_glsl(data, bop_precendence);
			data << glsl_op_str(bop.m_op);
			retrieve_expr(bop.m_rhs)->print_glsl(data, bop_precendence);
			if (inversion) {
				data << ")";
			}
		}
	};

	template<>
	struct OperatorGLSL<UnaryOperator> {
		static void call(const UnaryOperator& uop, GLSLData& data, const Precedence precedence) {
			data << glsl_op_str(uop.m_op);
			retrieve_expr(uop.m_arg)->print_glsl(data, Precedence::Unary);
		}
	};

	template<typename From, typename To>
	struct OperatorGLSL<ConvertorOperator< From, To>> {
		static void call(const ConvertorOperator< From, To>& op, GLSLData& data, const Precedence precedence) {
			if constexpr (SameMat<From, To>) {
				retrieve_expr(op.m_args[0])->print_glsl(data);
			} else {
				data << GLSLTypeStr<To>::get();
				OperatorGLSL<ArgSeq<1>>::call(op, data);
			}

		}
	};

	template<std::size_t N>
	struct OperatorGLSL<FunCall<N>> {
		static void call(const FunCall<N>& fun_call, GLSLData& data, const Precedence precedence) {
			data << glsl_op_str(fun_call.m_op);
			OperatorGLSL<ArgSeq<N>>::call(fun_call, data);
		}
	};

	template<typename F, typename ReturnType, std::size_t N >
	struct OperatorGLSL<CustomFunCall< F, ReturnType, N>> {
		static void call(const CustomFunCall< F, ReturnType, N>& fun_call, GLSLData& data, const Precedence precedence) {
			OperatorGLSL<Reference>::call(fun_call, data);
			data << "(";
			if constexpr (N > 0) {
				retrieve_expr(fun_call.m_args[0])->print_glsl(data, precedence);
				for (std::size_t i = 1; i < N; ++i) {
					data << ", ";
					retrieve_expr(fun_call.m_args[i])->print_glsl(data, precedence);
				}
			}
			data << ")";
		}
	};

	template<typename S, std::size_t Id>
	struct OperatorGLSL<MemberAccessor<S, Id>> {
		static void call(const MemberAccessor<S, Id>& accessor, GLSLData& data, const Precedence precedence) {
			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(accessor.m_obj))) {
				if (ctor->m_flags & CtorFlags::Temporary) {
					ctor->print_glsl(data, precedence);
				} else {
					data << data.var_names.find(ctor->m_variable_id)->second;
				}
			} else {
				auto accessor_wrapper = dynamic_cast<MemberAccessorBase*>(retrieve_expr(accessor.m_obj));
				accessor_wrapper->print_glsl(data, precedence);
			}
			data << "." << S::get_member_name(Id);
		}
	};

}

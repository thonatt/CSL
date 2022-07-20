#pragma once

#include "../InstructionTree.hpp"
#include "../Preprocessor.hpp"
#include "../TemplateHelpers.hpp"

#include "Shaders.hpp"

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

namespace csl {

	const std::string& glsl_op_str(const Op op);

	struct GLSLData {
		std::stringstream stream;

		std::unordered_map<std::size_t, std::size_t> global_to_local;
		std::unordered_map<std::size_t, std::string> func_names;
		std::unordered_map<std::size_t, std::string> var_names;

		int trailing = 0;

		GLSLData& trail() {
			for (int t = 0; t < trailing; ++t)
				stream << "    ";
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
			if (inversion)
				stream << "(";
			retrieve_expr(expr)->print_glsl(*this, current);
			if (inversion)
				stream << ")";
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
		static void call(const ShaderController& controller, GLSLData& data)
		{
			for (const auto i : controller.m_scope->m_instructions)
				retrieve_instruction(i)->print_glsl(data);
		}
	};

	// operators infos 

	struct OpInfos {
		Precedence m_precendence;
		std::string m_str;
	};

#define MAKE_OP_IT(data, i, elem) { Op :: elem, { Precedence :: FunctionCall, CSL_PP_STR(elem) }},

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
			{ Op::BitwiseAndAssignment, { Precedence::Assignment, " &= "} },
			{ Op::ScalarLessThanScalar, { Precedence::Relational, " < "} },
			{ Op::ScalarGreaterThanScalar, { Precedence::Relational, " > "} },
			{ Op::ScalarLessThanEqualScalar, { Precedence::Relational, " <= "} },
			{ Op::ScalarGreaterThanEqualScalar, { Precedence::Relational, " >= "} },
			{ Op::LogicalOr, { Precedence::LogicalOr, " || "} },
			{ Op::LogicalAnd, { Precedence::LogicalAnd, " && "} },
			{ Op::BitwiseAnd, { Precedence::BitwiseAnd, " & "} },
			{ Op::BitwiseOr, { Precedence::BitwiseOr, " | "} },
			{ Op::BitwiseLeftShift, { Precedence::BitwiseShift, " << "} },
			{ Op::BitwiseRightShift, { Precedence::BitwiseShift, " >> "} },
			{ Op::PostfixIncrement, { Precedence::Postfix, "++"} },
			{ Op::PrefixIncrement, { Precedence::Prefix, "++"} },
			{ Op::PostfixDecrement, { Precedence::Postfix, "--"} },
			{ Op::PrefixDecrement, { Precedence::Prefix, "--"} },
			{ Op::Equality, { Precedence::Equality, " == "} },
			{ Op::NotEquality, { Precedence::Equality, " != "} },
			CSL_PP_ITERATE(MAKE_OP_IT,
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
			degrees)
			CSL_PP_ITERATE(MAKE_OP_IT,
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
			reflect,
			inverse,
			transpose,
			imageSize,
			imageStore,
			round,
			bitfieldExtract,
			sign)
		};

		return op_infos;
	}

#undef MAKE_OP_IT

	inline const std::string& glsl_op_str(const Op op) {
		const auto it = glsl_op_infos().find(op);
		if (it == glsl_op_infos().end()) {
			static const std::string undefined = " undefined Op ";
			return undefined;
		} else {
			return it->second.m_str;
		}
	}

	inline Precedence glsl_op_precedence(const Op op) {
		const auto it = glsl_op_infos().find(op);
		if (it == glsl_op_infos().end()) {
			return Precedence::NoExtraParenthesis;
		} else {
			return it->second.m_precendence;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct GLSLQualifier
	{
		static std::string get() { return ""; }
	};

	template<template<std::size_t N> class T>
	struct GLSLQualifierN {
		static std::string get();
	};

	template<typename Q, typename ...Qs>
	struct GLSLQualifier<glsl::Layout<Q, Qs...>> {
		static std::string get() {
			std::string s = "layout(";
			s += GLSLQualifier<Q>::get();
			((s += ", " + GLSLQualifier<Qs>::get()), ...);
			s += ")";
			return s;
		}
	};

	template<template<std::size_t N> class T, std::size_t M>
	struct GLSLQualifier<T<M>> {
		static std::string get() {
			return GLSLQualifierN<T>::get() + " = " + std::to_string(M);
		}
	};

	template<> inline std::string GLSLQualifierN<glsl::Binding>::get() { return "binding"; }
	template<> inline std::string GLSLQualifierN<glsl::Location>::get() { return "location"; }
	template<> inline std::string GLSLQualifierN<glsl::tcs_common::Vertices>::get() { return "vertices"; }
	template<> inline std::string GLSLQualifierN<glsl::geom_common::Max_vertices>::get() { return "max_vertices"; }
	template<> inline std::string GLSLQualifierN<glsl::compute_common::local_size_x>::get() { return "local_size_x"; }
	template<> inline std::string GLSLQualifierN<glsl::compute_common::local_size_y>::get() { return "local_size_y"; }
	template<> inline std::string GLSLQualifierN<glsl::compute_common::local_size_z>::get() { return "local_size_z"; }

#define CSL_QUALIFIER_STR_IT(data, i, elem) \
	template<> inline std::string GLSLQualifier<CSL_PP_FIRST(elem)>::get() { return CSL_PP_STR(CSL_PP_SECOND(elem)); }

	CSL_PP_ITERATE(CSL_QUALIFIER_STR_IT,
		(glsl::Uniform, uniform),
		(glsl::In, in),
		(glsl::Out, out),
		(glsl::Inout, inout),
		(glsl::Triangles, triangles),
		(glsl::Equal_spacing, equal_spacing),
		(glsl::frag_common::Early_fragment_tests, early_fragment_tests),
		(glsl::Ccw, ccw),
		(glsl::Std140, std140),
		(glsl::Rgba32f, rgba32f),
		(glsl::tcs_common::Patch, patch),
		(glsl::geom_common::Line_strip, line_strip)
	);


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
				((s += (Ns == 0 ? "[]" : "[" + std::to_string(Ns) + "]")), ...);
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
		static const std::string& get() {
			return T::get_type_str();
		}
	};

	template<> inline const std::string& GLSLTypeStr<void>::get() {
		static const std::string s = "void"; return s;
	}
	template<> inline const std::string& GLSLTypeStr<bool>::get() {
		static const std::string s = "bool"; return s;
	}
	template<> inline const std::string& GLSLTypeStr<int>::get() {
		static const std::string s = "int"; return s;
	}
	template<> inline const std::string& GLSLTypeStr<unsigned int>::get() {
		static const std::string s = "uint"; return s;
	}
	template<> inline const std::string& GLSLTypeStr<float>::get() {
		static const std::string s = "float"; return s;
	}
	template<> inline const std::string& GLSLTypeStr<double>::get() {
		static const std::string s = "double"; return s;
	}

	template<typename T>
	struct GLSLTypeStr<Scalar<T>> {
		static const std::string& get() {
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

	inline const std::unordered_map<SamplerFlags, std::string>& get_sampler_access_strs()
	{
		static const std::unordered_map<SamplerFlags, std::string> sampler_access_strs = {
				{ SamplerFlags::Sampler, "sampler" },
				{ SamplerFlags::Image, "image"}
		};
		return sampler_access_strs;
	}

	inline const std::unordered_map<SamplerFlags, std::string>& get_sampler_type_strs()
	{
		static const std::unordered_map<SamplerFlags, std::string> sampler_type_strs = {
			{ SamplerFlags::Basic, ""},
			{ SamplerFlags::Cube, "Cube"},
			{ SamplerFlags::Rectangle , "Rect"},
			{ SamplerFlags::Multisample, "MS"},
			{ SamplerFlags::Buffer, "Buffer"},
		};
		return sampler_type_strs;
	}

	template<typename T, std::size_t N, SamplerFlags Flags>
	struct GLSLTypeStr<Sampler< T, N, Flags>> {
		static const std::string& get() {
			static const std::string type_str = [] {
				return TypePrefixStr<T>::get() +
					get_sampler_access_strs().find(Flags & SamplerFlags::MaskAccessType)->second +
					(N != 0 ? std::to_string(N) + 'D' : "") +
					get_sampler_type_strs().find(Flags & SamplerFlags::MaskSamplerType)->second +
					(bool(Flags & SamplerFlags::Array) ? "Array" : "") +
					(bool(Flags & SamplerFlags::Shadow) ? "Shadow" : "");
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
		static const std::string& get() {
			return GLSLTypeStr<T>::get();
		}
	};

	template<typename T, typename Ds, typename ...Qs>
	struct GLSLTypeStr<ArrayInterface<T, Ds, Qs...>> {
		static const std::string& get() {
			return GLSLTypeStr<T>::get();
		}
	};

	template<typename T, typename ...Qualifiers>
	struct GLSLTypeDeclaration {
		static const std::string& get()
		{
			static const std::string s = []
			{
				using ArrayDimensions = typename ArrayInfos<Qualifiers...>::Dimensions;
				using QualifierList = RemoveArrayFromQualifiers<Qualifiers...>;

				std::string s;
				if constexpr (QualifierList::Size > 0)
					s += GLSLQualifier<QualifierList>::get() + " ";
				s += GLSLTypeStr<T>::get();
				if ()
					s += ArraySizePrinterGLSL<ArrayDimensions>::get();
				return s;
			}();
			return s;
		}
	};

	template<typename T, typename Dimensions = SizeList<>, typename Qualifiers = TList<>>
	struct GLSLDeclaration {
		static const std::string& prefix()
		{
			static const std::string s = [] {
				std::string s;
				if constexpr (Qualifiers::Size > 0)
					s += GLSLQualifier<Qualifiers>::get() + " ";
				s += GLSLTypeStr<T>::get();
				return s;
			}();
			return s;
		}

		static std::string get(const std::string& var_name) {
			std::string str = prefix() + " " + var_name;
			if constexpr (Dimensions::Size > 0)
				str += ArraySizePrinterGLSL<Dimensions>::get();
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
	struct InstructionGLSL<ForInstruction>
	{
		enum class Status { InitExpr, Condition, LoopExpr };

		static void header(const ForInstruction& i, GLSLData& data)
		{
			data << "for(";
			Status status = Status::InitExpr;
			for (const auto j : i.args->m_instructions) {
				const InstructionBase* arg_instruction = retrieve_instruction(j);
				const Expr arg_expr = safe_static_cast<const Statement*>(arg_instruction)->m_expr;
				auto ctor = safe_static_cast<ConstructorBase*>(retrieve_expr(arg_expr));
				if (status == Status::InitExpr && bool(ctor->m_flags & CtorFlags::Initialisation)) {
					arg_instruction->print_glsl(data);
					status = Status::Condition;
					continue;
				}
				if (status == Status::Condition) {
					arg_instruction->print_glsl(data);
					status = Status::LoopExpr;
					continue;
				}
				if (status == Status::LoopExpr) {
					ForIterationStatement(arg_expr).print_glsl(data);
				}
			}

			if (status == Status::InitExpr) {
				data << ";;";
			} else if (status == Status::Condition) {
				data << ";";
			}
			data << ")";
		}

		static void call(const ForInstruction& i, GLSLData& data) {
			data.endl().trail();
			header(i, data);
			data << " {";
			++data.trailing;
			for (const auto j : i.body->m_instructions) {
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
				if (bool(ctor->m_flags & CtorFlags::Temporary)) {
					return;
				}
				if (bool(ctor->m_flags & CtorFlags::Untracked)) {
					data.register_var_name(ctor->m_name, ctor->m_variable_id);
					return;
				}
				if (bool(ctor->m_flags & CtorFlags::FunctionArgument)) {
					retrieve_expr(i.m_expr)->print_glsl(data, Precedence::NoExtraParenthesis);
					return;
				}
			}

			data.endl().trail();
			retrieve_expr(i.m_expr)->print_glsl(data, Precedence::NoExtraParenthesis);
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
			data.endl().trail() << "switch(";
			retrieve_expr(i.m_condition)->print_glsl(data, Precedence::NoExtraParenthesis);
			data << ") {";
			++data.trailing;
			for (const auto& c : i.m_body->m_instructions) {
				retrieve_instruction(c)->print_glsl(data);
			}
			--data.trailing;
			data.endl().trail() << "}";
		}
	};

	template<>
	struct InstructionGLSL<SwitchCase> {
		static void call(const SwitchCase& i, GLSLData& data) {
			data.endl().trail();
			if (i.m_label) {
				data << "case ";
				retrieve_expr(i.m_label)->print_glsl(data, Precedence::NoExtraParenthesis);
			} else {
				data << "default";
			}
			data << " : {";
			++data.trailing;
			for (const auto& j : i.m_body->m_instructions) {
				retrieve_instruction(j)->print_glsl(data);
			}
			--data.trailing;
			data.endl().trail() << "}";
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

	template<> inline std::string SpecialStatementStr<Discard>() { return "discard"; }
	template<> inline std::string SpecialStatementStr<Break>() { return "break"; }
	template<> inline std::string SpecialStatementStr<Continue>() { return "continue"; }
	template<> inline std::string SpecialStatementStr<EmitVertexI>() { return "EmitVertex()"; }
	template<> inline std::string SpecialStatementStr<EndPrimitiveI>() { return "EndPrimitive()"; }

	template<typename T>
	struct InstructionGLSL<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, GLSLData& data) {
			data.endl().trail() << SpecialStatementStr<T>() << ";";
		}
	};

	template<typename ...Qs>
	struct InstructionGLSL<SpecialStatement<ShaderStageOption<Qs...>>> {
		static void call(const SpecialStatement<ShaderStageOption<Qs...>>& i, GLSLData& data) {
			data.endl().trail() << GLSLQualifier<TList<Qs...>>::get() << ";";
		}
	};

	template<std::size_t NumOverloads>
	struct OverloadGLSL
	{
		template<typename T, std::size_t Id>
		struct Get {
			static void call(const std::array<FuncOverload, NumOverloads>& overloads, GLSLData& data, const std::string& fname) {
				data.endl().trail() << GLSLTypeStr<T>::get() << " " << fname << "(";
				const auto& args = overloads[Id].args->m_instructions;
				if (get_arg_evaluation_order() == ArgEvaluationOrder::LeftToRight) {
					if (!args.empty())
						retrieve_instruction(args.front())->print_glsl(data);

					for (std::size_t i = 1; i < args.size(); ++i) {
						data << ", ";
						retrieve_instruction(args[i])->print_glsl(data);
					}
				} else {
					if (!args.empty())
						retrieve_instruction(args.back())->print_glsl(data);

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
				data.endl();
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

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMemberGLSL {
		static void call(GLSLData& data) {
			data.endl().trail() << GLSLDeclaration<T>::get(S::get_member_name(Id)) << ";";
		}
	};

	template<typename S>
	struct InstructionGLSL<StructDeclaration<S>>
	{
		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberGLSL<S, T, Id>;

		static void call(const StructDeclaration<S>& s, GLSLData& data) {
			data.endl().trail() << "struct " << GLSLTypeStr<S>::get();
			data.endl().trail() << "{";
			++data.trailing;
			iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
			--data.trailing;
			data.endl().trail() << "};";
			data.endl();
		}
	};

	template<typename Interface, typename Dimensions, typename Qualifiers>
	struct InstructionGLSL<NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>>
	{
		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberGLSL<Interface, T, Id>;

		static void call(const NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>& s, GLSLData& data) {
			data.endl().trail();
			if constexpr (Qualifiers::Size > 0) {
				data << GLSLQualifier<Qualifiers>::get() + " ";
			}
			data << GLSLTypeStr<Interface>::get() << " {";
			++data.trailing;
			iterate_over_typelist<typename Interface::MemberTList, StructMemberDeclaration>(data);
			--data.trailing;
			data.endl().trail() << "} " << s.m_name;
			if constexpr (Dimensions::Size > 0) {
				data << ArraySizePrinterGLSL<Dimensions>::get();
			}
			data << ";";
			data.endl();
		}
	};

	template<typename Interface, typename T, std::size_t Id>
	struct UnnamedInterfaceDeclarationMemberGLSL
	{
		static void call(const Interface& i, GLSLData& data) {
			data.endl().trail() << GLSLDeclaration<T>::get(i.m_names[Id]) << ";";
		}
	};

	template<typename ...Qs, typename ...Ts>
	struct InstructionGLSL<UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>>
	{
		using ArrayDimensions = typename ArrayInfos<Qs...>::Dimensions;
		using Qualifiers = RemoveArrayFromQualifiers<Qs...>;
		using Interface = UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>;

		template<typename T, std::size_t Id>
		using MemberDeclaration = UnnamedInterfaceDeclarationMemberGLSL<Interface, T, Id>;

		static void call(const Interface& s, GLSLData& data) {
			data.endl().trail();
			if constexpr (Qualifiers::Size > 0)
				data << GLSLQualifier<Qualifiers>::get() + " ";

			data << s.m_name << " {";
			++data.trailing;
			iterate_over_typelist<TList<Ts...>, MemberDeclaration>(s, data);
			--data.trailing;
			data.endl().trail() << "};";
			data.endl();
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
				for (std::size_t i = 1; i < N; ++i) {
					data << ", ";
					retrieve_expr(seq.m_args[i])->print_glsl(data, precedence);
				}
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

	template<typename T, std::size_t N, std::size_t...Ds, typename ...Qualifiers>
	struct OperatorGLSL<Constructor<T, N, SizeList<Ds...>, TList<Qualifiers...>>>
	{
		using Dimensions = SizeList<Ds...>;
		using QualifersList = TList<Qualifiers...>;

		static void temporary_str(const Constructor<T, N, Dimensions, QualifersList>& ctor, GLSLData& data)
		{
			if (ctor.arg_count() == 1) {
				retrieve_expr(ctor.first_arg())->print_glsl(data);
			} else {
				data << GLSLTypeStr<T>::get() << ArraySizePrinterGLSL<Dimensions>::get();
				OperatorGLSL<ArgSeq<N>>::call(ctor, data);
			}
		}

		static void call(const Constructor<T, N, Dimensions, QualifersList>& ctor, GLSLData& data, const Precedence precedence = Precedence::NoExtraParenthesis)
		{
			const CtorFlags ctor_flag = ctor.m_flags & CtorFlags::SwitchMask;
			switch (ctor_flag)
			{
			case CtorFlags::Declaration: {
				data << GLSLDeclaration<T, Dimensions, QualifersList>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
				break;
			}
			case CtorFlags::Initialisation: {
				data << GLSLDeclaration<T, Dimensions, QualifersList>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
				data << " = ";
				temporary_str(ctor, data);
				break;
			}
			case CtorFlags::Temporary: {
				temporary_str(ctor, data);
				break;
			}
			case CtorFlags::Unused: {
				retrieve_expr(ctor.first_arg())->print_glsl(data);
				break;
			}
			case CtorFlags::FunctionArgument: {
				data << GLSLDeclaration<T, Dimensions, QualifersList>::get(data.register_var_name(ctor.m_name, ctor.m_variable_id));
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
			auto obj = retrieve_expr(subscript.m_obj);
			assert(obj);
			obj->print_glsl(data, precedence);
			data << "[";
			retrieve_expr(subscript.m_index)->print_glsl(data, Precedence::NoExtraParenthesis);
			data << "]";
		}
	};

	template<>
	struct OperatorGLSL<SwizzlingBase> {
		static void call(const SwizzlingBase& swizzle, GLSLData& data, const Precedence precedence) { }
	};

	template<char ...chars>
	struct OperatorGLSL<Swizzling<chars...>> {
		static void call(const Swizzling<chars...>& swizzle, GLSLData& data, const Precedence precedence) {
			retrieve_expr(swizzle.m_obj)->print_glsl(data, Precedence::Swizzle);
			data << ".";
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
				if (static_cast<float>(static_cast<int>(litteral.value)) == litteral.value) {
					data << static_cast<int>(litteral.value) << ".0";
				} else if (static_cast<float>(static_cast<int>(10.0 * litteral.value)) == 10.0 * litteral.value) {
					if (litteral.value < 0) {
						data << "-";
					}
					data << static_cast<int>(std::abs(litteral.value)) << "." << std::abs(static_cast<int>(10.0 * litteral.value) - 10 * static_cast<int>(std::abs(litteral.value)));
				} else {
					data << std::scientific << litteral.value;
				}

				//std::stringstream ss;
				//ss << std::fixed << litteral.value;
				//std::string s = ss.str();
				//s.erase(s.find_last_not_of('0') + 1, std::string::npos);
				//if (s.back() == '.') {
				//	s += '0';
				//}
				//data << s;
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

	template<>
	struct OperatorGLSL<TernaryOperator> {
		static void call(const TernaryOperator& top, GLSLData& data, const Precedence precedence) {
			const bool inversion = (precedence < Precedence::Ternary);
			if (inversion) {
				data << "(";
			}
			retrieve_expr(top.m_condition)->print_glsl(data, Precedence::Ternary);
			data << " ? ";
			retrieve_expr(top.m_first)->print_glsl(data, Precedence::Ternary);
			data << " : ";
			retrieve_expr(top.m_second)->print_glsl(data, Precedence::Ternary);
			if (inversion) {
				data << ")";
			}
		}
	};

	template<typename From, typename To>
	struct OperatorGLSL<ConvertorOperator<From, To>> {
		static void call(const ConvertorOperator<From, To>& op, GLSLData& data, const Precedence precedence) {
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
	struct OperatorGLSL<CustomFunCall<F, ReturnType, N>> {
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
				if (bool(ctor->m_flags & CtorFlags::Temporary)) {
					ctor->print_glsl(data, precedence);
				} else {
					const auto it = data.var_names.find(ctor->m_variable_id);
					if (it == data.var_names.end()) {
						data << data.register_var_name(ctor->m_name, ctor->m_variable_id);
					} else {
						data << it->second;
					}
				}
			} else {
				auto accessor_wrapper = safe_static_cast<MemberAccessorBase*>(retrieve_expr(accessor.m_obj));
				accessor_wrapper->print_glsl(data, precedence);
			}
			data << "." << S::get_member_name(Id);
		}
	};

}

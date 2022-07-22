#pragma once

#include <csl/Types.hpp>
#include <csl/InstructionTree.hpp>
#include <csl/Swizzles.hpp>

#include <csl/glsl/ToGLSL.hpp>

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include <imgui.h>

namespace csl 
{

	struct ImGuiData {

		std::string unique(const std::string& s)
		{
			return s + "##" + std::to_string(counter++);
		}

		std::string glsl_from_expr(const csl::Expr expr) {
			glsl_data.stream.str("");
			retrieve_expr(expr)->print_glsl(glsl_data);
			return glsl_data.stream.str();
		}

		template<typename F>
		ImGuiData& node(const std::string& s, F&& f) {
			if (ImGui::TreeNode(unique(s).c_str())) {
				f();
				ImGui::TreePop();
			}
			return *this;
		}

		ImGuiData& leaf(const std::string& s) {
			ImGui::TreeNodeEx(s.c_str(), ImGuiTreeNodeFlags_Leaf);
			ImGui::TreePop();
			return *this;
		}

		template<typename T>
		ImGuiData& vector_node(const std::string& name, const std::vector<T>& vs)
		{
			if (!vs.empty()) {
				if (ImGui::TreeNode(unique(name).c_str())) {
					for (const auto& v : vs) {
						retrieve_instruction(v)->print_imgui(*this);
					}
					ImGui::TreePop();
				}
			}
			return *this;
		}

		template<std::size_t N>
		ImGuiData& expr_vector_node(const std::string& name, const std::array<Expr, N>& vs)
		{
			if constexpr (N > 0) {
				if (ImGui::TreeNode(unique(name).c_str())) {
					for (const auto& v : vs) {
						retrieve_expr(v)->print_imgui(*this);
					}
					ImGui::TreePop();
				}
			}
			return *this;
		}

		ImGuiData& operator<<(const std::string& str) {
			ImGui::TextWrapped(str.c_str());
			return *this;
		}

		GLSLData glsl_data;
		std::size_t counter = 0;
	};

	template<typename Delayed>
	struct ControllerImGui<Delayed, ShaderController>
	{
		static void call(const ShaderController& controller, ImGuiData& data)
		{
			for (const auto& i : controller.m_scope->m_instructions)
				retrieve_instruction(i)->print_imgui(data);
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct ImGuiQualifier {
		static void print(ImGuiData& data) {
			data << "unspecified qualifier";
		}
	};

	template<typename Q, typename ...Qs>
	struct ImGuiQualifier<TList<Q, Qs...>> {
		static void print(ImGuiData& data) {
			data << typeid(Q).name();
			(((data << ", "), data << typeid(Qs).name()), ...);
		}
	};

	template<typename>
	struct ArraySizePrinterImGui { };

	template<size_t ...Ns>
	struct ImGuiQualifier<ArraySizePrinterImGui<SizeList<Ns...>>> {
		static void print_glsl(ImGuiData& data) {
			std::stringstream s;
			((s << "[" << Ns << "]"), ...);
			data << s.str();
		}
	};

	///////////////////////////////////////////////////////////////////////////////////
	// instructions

	template<>
	struct InstructionImGui<IfInstruction> {
		static void call(const IfInstruction& i, ImGuiData& data) {

			std::string case_str;
			for (std::size_t k = 0; k < i.m_cases.size(); ++k) {
				if (k == 0) {
					case_str = "if(";
				} else if (k != i.m_cases.size() - 1) {
					case_str = "else if(";
				} else {
					case_str = "else ";
				}

				if (k == 0 || k != i.m_cases.size() - 1) {
					case_str += data.glsl_from_expr(i.m_cases[k].condition) + ")";
				}
				data.vector_node(case_str, i.m_cases[k].body->m_instructions);
			}

		}
	};

	template<>
	struct InstructionImGui<WhileInstruction> {
		static void call(const WhileInstruction& i, ImGuiData& data) {
			std::string while_str = "while(" + data.glsl_from_expr(i.m_condition) + ")";
			data.vector_node(while_str, i.m_body->m_instructions);
		}
	};

	template<>
	struct InstructionImGui<ForInstruction> {
		static void call(const ForInstruction& i, ImGuiData& data) {
			GLSLData args_glsl;
			InstructionGLSL<ForInstruction>::header(i, args_glsl);
			data.vector_node(args_glsl.stream.str(), i.body->m_instructions);
		}
	};

	template<>
	struct InstructionImGui<ForArgStatement> {
		static void call(const ForArgStatement& i, ImGuiData& data) {
		}
	};

	template<>
	struct InstructionImGui<ForIterationStatement> {
		static void call(const ForIterationStatement& i, ImGuiData& data) {
		}
	};

	template<>
	struct InstructionImGui<Statement> {
		static void call(const Statement& i, ImGuiData& data) {
			if (!i.m_expr) {
				data << "empty expr";
				return;
			}

			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(i.m_expr))) {
				if (bool(ctor->m_flags & CtorFlags::Temporary)) {
					return;
				}
				if (bool(ctor->m_flags & CtorFlags::Untracked) || bool(ctor->m_flags & CtorFlags::FunctionArgument)) {
					data.glsl_data.register_var_name(ctor->m_name, ctor->m_variable_id);
					return;
				}
			}

			retrieve_expr(i.m_expr)->print_imgui(data);
		}
	};

	template<>
	struct InstructionImGui<SwitchInstruction> {
		static void call(const SwitchInstruction& i, ImGuiData& data) {
			data.node("switch", [&] {
				data.node("condition : " + data.unique(data.glsl_from_expr(i.m_condition)), [&] {
					retrieve_expr(i.m_condition)->print_imgui(data);
					});
				for (const auto& c : i.m_body->m_instructions) {
					retrieve_instruction(c)->print_imgui(data);
				}
				});
		}
	};

	template<>
	struct InstructionImGui<SwitchCase> {
		static void call(const SwitchCase& i, ImGuiData& data) {

			std::string case_str;
			if (i.m_label) {
				case_str = "case " + data.glsl_from_expr(i.m_label);
			} else {
				case_str = "default";
			}
			case_str += " : ";
			data.vector_node(case_str, i.m_body->m_instructions);
		}
	};

	template<typename T>
	struct InstructionImGui<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, ImGuiData& data) { }
	};

	template<>
	struct InstructionImGui<ReturnStatement> {
		static void call(const ReturnStatement& i, ImGuiData& data) {
			std::string return_str = "return " + data.glsl_from_expr(i.m_expr) + ";";
			data.node(return_str, [&] {
				retrieve_expr(i.m_expr)->print_imgui(data);
				});
		}
	};

	template<>
	struct InstructionImGui<FuncDeclarationBase> {
		static void call(const FuncDeclarationBase& f, ImGuiData& data) { }
	};

	template<std::size_t NumOverloads>
	struct OverloadImGui {

		template<typename T, std::size_t Id>
		struct Get {
			static void call(const std::array<FuncOverload, NumOverloads>& overloads, ImGuiData& data, const std::string& fname) {

				data.glsl_data.stream.str("");
				data.glsl_data << GLSLTypeStr<T>::get() + " " + fname + "(";
				const auto& args = overloads[Id].args->m_instructions;
				if (get_arg_evaluation_order() == ArgEvaluationOrder::LeftToRight) {
					if (!args.empty()) {
						retrieve_instruction(args.front())->print_glsl(data.glsl_data);
					}
					for (std::size_t i = 1; i < args.size(); ++i) {
						data.glsl_data << ", ";
						retrieve_instruction(args[i])->print_glsl(data.glsl_data);
					}
				} else {
					if (!args.empty()) {
						retrieve_instruction(args.back())->print_glsl(data.glsl_data);
					}
					for (std::size_t i = 1; i < args.size(); ++i) {
						data.glsl_data << ", ";
						retrieve_instruction(args[args.size() - i - 1])->print_glsl(data.glsl_data);
					}
				}
				data.glsl_data << ");";

				data.node(data.glsl_data.stream.str() + "##" + std::to_string(Id), [&] {
					for (const auto& i : overloads[Id].body->m_instructions) {
						retrieve_instruction(i)->print_imgui(data);
					}
					});
			}
		};

	};

	template<typename ReturnTList, typename ...Fs>
	struct InstructionImGui<FuncDeclaration<ReturnTList, Fs...>> {
		static void call(const FuncDeclaration<ReturnTList, Fs...>& f, ImGuiData& data) {
			const std::string& name = data.glsl_data.register_var_name(f.m_name, f.m_id);
			iterate_over_typelist<ReturnTList, OverloadImGui<sizeof...(Fs)>::template Get>(f.m_overloads, data, name);
		}
	};

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMemberImGui {
		static void call(ImGuiData& data) {
			data << (GLSLDeclaration<T>::get(S::get_member_name(Id)) + ";");
		}
	};

	template<typename S>
	struct InstructionImGui<StructDeclaration<S>> {

		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberImGui<S, T, Id>;

		static void call(const StructDeclaration<S>& s, ImGuiData& data) {
			data.node(data.unique(GLSLTypeStr<S>::get()), [&] {
				iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
				});
		}
	};

	template<typename Interface, typename Dimensions, typename Qualifiers>
	struct InstructionImGui<NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>>
	{
		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberImGui<Interface, T, Id>;

		static void call(const NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>& s, ImGuiData& data)
		{
			std::string interface_name;
			if constexpr (Qualifiers::Size > 0)
				interface_name += GLSLQualifier<Qualifiers>::get() + " ";

			interface_name += GLSLTypeStr<Interface>::get() + " " + s.m_name;
			if constexpr (Dimensions::Size > 0)
				interface_name += ArraySizePrinterGLSL<Dimensions>::get();

			interface_name += ";";
			data.node(interface_name, [&] {
				iterate_over_typelist<typename Interface::MemberTList, StructMemberDeclaration>(data);
				});
		}
	};

	template<typename Interface, typename T, std::size_t Id>
	struct UnnamedInterfaceDeclarationMemberImGui {
		static void call(const Interface& i, ImGuiData& data) {
			data << (GLSLDeclaration<T>::get(i.m_names[1 + Id]) + ";");
		}
	};

	template<typename ...Qs, typename ...Ts>
	struct InstructionImGui<UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>> {

		using Qualifiers = RemoveArrayFromQualifiers<Qs...>;
		using Interface = UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>;

		template<typename T, std::size_t Id>
		using MemberDeclaration = UnnamedInterfaceDeclarationMemberImGui<Interface, T, Id>;

		static void call(const UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>& s, ImGuiData& data) {
			std::string interface_name;
			if constexpr (Qualifiers::Size > 0) {
				interface_name += GLSLQualifier<Qualifiers>::get() + " ";
			}
			interface_name += s.m_names[0];
			data.node(interface_name, [&] {
				iterate_over_typelist<TList<Ts...>, MemberDeclaration>(s, data);;
				});
		}
	};

	//////////////////////////////////////////////////////////
	// operators

	template<std::size_t N>
	struct OperatorImGui<ArgSeq<N>> {
		static void call(const ArgSeq<N>& seq, ImGuiData& data) {
			if constexpr (N > 0) {
				retrieve_expr(seq.m_args[0])->print_imgui(data);
			}
			for (std::size_t i = 1; i < N; ++i) {
				data << ", ";
				retrieve_expr(seq.m_args[i])->print_imgui(data);
			}
		}
	};

	template<>
	struct OperatorImGui<Reference> {
		static void call(const Reference& ref, ImGuiData& data) {
			data << ("$" + std::to_string(ref.m_id));
		}
	};

	template<>
	struct OperatorImGui<ConstructorBase> {
		static void call(const ConstructorBase& ctor, ImGuiData& data) {
			data << "Base ctor";
		}
	};

	template<typename T, std::size_t N, std::size_t...Ds, typename ...Qualifiers>
	struct OperatorImGui<Constructor<T, N, SizeList<Ds...>, TList<Qualifiers...>>>
	{
		static void call(const Constructor<T, N, SizeList<Ds...>, TList<Qualifiers...>>& ctor, ImGuiData& data) {

			static const std::unordered_map<CtorFlags, std::string> flag_strs = {
				{ CtorFlags::Declaration, "Declaration" },
				{ CtorFlags::Initialisation, "Initialisation" },
				{ CtorFlags::Temporary, "Temporary" },
				{ CtorFlags::Unused, "Unused" },
				{ CtorFlags::FunctionArgument, "FuncArg" },
			};

			data.glsl_data.stream.str("");
			ctor.print_glsl(data.glsl_data, csl::Precedence::NoExtraParenthesis);
			data.glsl_data << ";";

			const CtorFlags switch_flag = ctor.m_flags & CtorFlags::SwitchMask;

			//std::string ctor_str = flag_strs.at(switch_flag) + " " + std::string(typeid(T).name());
			//if (!ctor.m_name.empty()) {
			//	ctor_str += " " + ctor.m_name + " ";
			//}
			//ctor_str += std::string(" $") + std::to_string(ctor.m_variable_id);
			std::string ctor_str = data.glsl_data.stream.str();

			//using ArrayDimensions = typename T::ArrayDimensions;
			//using Qualifiers = typename T::Qualifiers;

			if constexpr (N == 0) {
				data.leaf(ctor_str);
			} else {
				data.expr_vector_node(ctor_str, ctor.m_args);
			}
		}
	};

	template<>
	struct OperatorImGui<ArraySubscript> {
		static void call(const ArraySubscript& subscript, ImGuiData& data) {
			data.glsl_data.stream.str("");
			OperatorGLSL<ArraySubscript>::call(subscript, data.glsl_data, Precedence::NoExtraParenthesis);

			data.node(data.glsl_data.stream.str(), [&] {
				data << "from ";
				retrieve_expr(subscript.m_obj)->print_imgui(data);
				data << "at index ";
				retrieve_expr(subscript.m_index)->print_imgui(data);
				});
		}
	};

	template<>
	struct OperatorImGui<SwizzlingBase> {
		static void call(const SwizzlingBase& swizzle, ImGuiData& data) { }
	};


	template<char ...chars>
	struct OperatorImGui<Swizzling<chars...>> {

		static void call(const Swizzling<chars...>& swizzle, ImGuiData& data) {
			data.node(data.glsl_from_expr(swizzle.m_obj), [&] {
				retrieve_expr(swizzle.m_obj)->print_imgui(data);
				});

			std::string swizzle_str = ".";
			((swizzle_str += chars), ...);
			data.leaf(swizzle_str);
		}
	};

	template<typename T>
	struct OperatorImGui<Litteral<T>> {
		static void call(const Litteral<T>& litteral, ImGuiData& data) {
			data.leaf("litteral " + std::string(typeid(typename Infos<T>::ScalarType).name()) + " " + std::to_string(litteral.value));
		}
	};

	template<>
	struct OperatorImGui<BinaryOperator> {
		static void call(const BinaryOperator& bop, ImGuiData& data) {
			data.node(data.glsl_from_expr(bop.m_lhs), [&] {
				retrieve_expr(bop.m_lhs)->print_imgui(data);
				});
			data << glsl_op_str(bop.m_op);
			data.node(data.glsl_from_expr(bop.m_rhs), [&] {
				retrieve_expr(bop.m_rhs)->print_imgui(data);
				});
		}
	};

	template<>
	struct OperatorImGui<UnaryOperator> {
		static void call(const UnaryOperator& uop, ImGuiData& data) {
			data.node(glsl_op_str(uop.m_op), [&] {
				retrieve_expr(uop.m_arg)->print_imgui(data);
				});
		}
	};

	template<typename From, typename To>
	struct OperatorImGui<ConvertorOperator< From, To>> {
		static void call(const ConvertorOperator< From, To>& op, ImGuiData& data) {
			data.node("convertor", [&] {
				retrieve_expr(op.m_args[0])->print_imgui(data);
				});
		}
	};

	template<std::size_t N>
	struct OperatorImGui<FunCall<N>> {
		static void call(const FunCall<N>& fun_call, ImGuiData& data) {
			data.expr_vector_node(glsl_op_str(fun_call.m_op), fun_call.m_args);
		}
	};

	template<typename F, typename ReturnType, std::size_t N>
	struct OperatorImGui<CustomFunCall< F, ReturnType, N>> {
		static void call(const CustomFunCall< F, ReturnType, N>& fun_call, ImGuiData& data) {
			data.node("custom function call", [&] {
				OperatorImGui<Reference>::call(fun_call, data);
				data.node("Args", [&] {
					if constexpr (N == 0) {
						data << "no arguments";
					}
					for (std::size_t i = 0; i < N; ++i) {
						retrieve_expr(fun_call.m_args[i])->print_imgui(data);
					}
					});
				});
		}
	};

	template<typename S, std::size_t Id>
	struct OperatorImGui<MemberAccessor<S, Id>> {
		static void call(const MemberAccessor<S, Id>& accessor, ImGuiData& data) {

			data.glsl_data.stream.str("");
			OperatorGLSL<MemberAccessor<S, Id>>::call(accessor, data.glsl_data, Precedence::NoExtraParenthesis);
			std::string member_str = data.glsl_data.stream.str();

			data.node(member_str, [&] {
				if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(accessor.m_obj))) {
					if (bool(ctor->m_flags & CtorFlags::Temporary)) {
						ctor->print_imgui(data);
					} else {
						data << ("$" + std::to_string(ctor->m_variable_id));
					}
				} else {
					auto accessor_wrapper = dynamic_cast<MemberAccessorBase*>(retrieve_expr(accessor.m_obj));
					accessor_wrapper->print_imgui(data);
				}
				data << S::get_member_name(Id);
				});
		}
	};

	template<>
	struct OperatorImGui<TernaryOperator> {
		static void call(const TernaryOperator& top, ImGuiData& data) {
			data.node("ternary", [&] {
				retrieve_expr(top.m_condition)->print_imgui(data);
				retrieve_expr(top.m_first)->print_imgui(data);
				retrieve_expr(top.m_second)->print_imgui(data);
				});
		}
	};
}

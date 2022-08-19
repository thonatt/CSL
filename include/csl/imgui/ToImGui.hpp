#pragma once

#include <csl/Types.hpp>
#include <csl/InstructionTree.hpp>
#include <csl/Swizzles.hpp>

#include <csl/glsl/Shaders.hpp>

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include <imgui.h>

namespace csl
{
	struct ImGuiData
	{
		GLSLData glsl_data;
		std::size_t counter = 0;

		std::string unique(const std::string& s)
		{
			return s + "##" + std::to_string(counter++);
		}

		std::string glsl_from_expr(const csl::Expr expr)
		{
			glsl_data.stream.str("");
			retrieve_expr(expr)->print_glsl(glsl_data);
			return glsl_data.stream.str();
		}

		template<typename F>
		ImGuiData& node(const std::string& s, F&& f)
		{
			if (ImGui::TreeNode(unique(s).c_str())) {
				f();
				ImGui::TreePop();
			}
			return *this;
		}

		ImGuiData& leaf(const std::string& s)
		{
			ImGui::TreeNodeEx(s.c_str(), ImGuiTreeNodeFlags_Leaf);
			ImGui::TreePop();
			return *this;
		}

		template<typename T>
		ImGuiData& vector_node(const std::string& name, const std::vector<T>& vs)
		{
			if (!vs.empty()) {
				if (ImGui::TreeNode(unique(name).c_str())) {
					for (const auto& v : vs)
						retrieve_instruction(v)->print_imgui(*this);
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
					for (const auto& v : vs)
						retrieve_expr(v)->print_imgui(*this);
					ImGui::TreePop();
				}
			}
			return *this;
		}

		ImGuiData& operator<<(const std::string& str)
		{
			ImGui::TextWrapped(str.c_str());
			return *this;
		}

		void print_expr(const Expr& expr)
		{
			retrieve_expr(expr)->print_imgui(*this);
		}

		void print_instruction(const InstructionIndex& index)
		{
			retrieve_instruction(index)->print_imgui(*this);
		}
	};

	inline GLSLData& get_glsl_data(ImGuiData& data)
	{
		return data.glsl_data;
	}

	template<typename T>
	struct ToImGui;

	template<typename T>
	void to_imgui(const T& t, ImGuiData& data)
	{
		ToImGui<T>::call(t, data);
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct ImGuiQualifier;

	template<typename Q, typename ...Qs>
	struct ImGuiQualifier<TList<Q, Qs...>>
	{
		static void print(ImGuiData& data) {
			data << typeid(Q).name();
			(((data << ", "), data << typeid(Qs).name()), ...);
		}
	};

	template<typename>
	struct ArraySizePrinterImGui { };

	template<size_t ...Ns>
	struct ImGuiQualifier<ArraySizePrinterImGui<SizeList<Ns...>>>
	{
		static void print_glsl(ImGuiData& data) {
			std::stringstream s;
			((s << "[" << Ns << "]"), ...);
			data << s.str();
		}
	};

	///////////////////////////////////////////////////////////////////////////////////
	// instructions

	template<>
	struct ToImGui<IfInstruction>
	{
		static void call(const IfInstruction& i, ImGuiData& data)
		{
			std::string case_str;
			for (std::size_t k = 0; k < i.m_cases.size(); ++k) {
				if (k == 0)
					case_str = "if(";
				else if (k != i.m_cases.size() - 1)
					case_str = "else if(";
				else
					case_str = "else ";

				if (k == 0 || k != i.m_cases.size() - 1)
					case_str += data.glsl_from_expr(i.m_cases[k].condition) + ")";

				data.vector_node(case_str, i.m_cases[k].body->m_instructions);
			}
		}
	};

	template<>
	struct ToImGui<WhileInstruction>
	{
		static void call(const WhileInstruction& i, ImGuiData& data) {
			const std::string while_str = "while(" + data.glsl_from_expr(i.m_condition) + ")";
			data.vector_node(while_str, i.m_body->m_instructions);
		}
	};

	template<>
	struct ToImGui<ForInstruction>
	{
		static void call(const ForInstruction& i, ImGuiData& data) {
			data.glsl_data.stream.str("");
			ToGLSL<ForInstruction>::header(i, data.glsl_data);
			data.vector_node(data.glsl_data.stream.str(), i.body->m_instructions);
		}
	};

	template<>
	struct ToImGui<ForArgStatement> {
		static void call(const ForArgStatement& i, ImGuiData& data) {
		}
	};

	template<>
	struct ToImGui<ForIterationStatement> {
		static void call(const ForIterationStatement& i, ImGuiData& data) {
		}
	};

	template<>
	struct ToImGui<Statement>
	{
		static void call(const Statement& i, ImGuiData& data)
		{
			assert(i.m_expr);

			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(i.m_expr))) {
				if (bool(ctor->m_flags & CtorFlags::Temporary))
					return;

				if (bool(ctor->m_flags & CtorFlags::Untracked) || bool(ctor->m_flags & CtorFlags::FunctionArgument)) {
					data.glsl_data.register_var_name(ctor->m_name, ctor->m_variable_id);
					return;
				}
			}

			data.print_expr(i.m_expr);
		}
	};

	template<>
	struct ToImGui<SwitchInstruction>
	{
		static void call(const SwitchInstruction& i, ImGuiData& data) {
			data.node("switch", [&] {
				data.node("condition : " + data.unique(data.glsl_from_expr(i.m_condition)), [&] {
					data.print_expr(i.m_condition);
					});
				for (const auto& c : i.m_body->m_instructions)
					data.print_instruction(c);
				});
		}
	};

	template<>
	struct ToImGui<SwitchCase>
	{
		static void call(const SwitchCase& i, ImGuiData& data)
		{
			std::string case_str;
			if (i.m_label)
				case_str = "case " + data.glsl_from_expr(i.m_label);
			else
				case_str = "default";

			case_str += " : ";
			data.vector_node(case_str, i.m_body->m_instructions);
		}
	};

	template<typename T>
	struct ToImGui<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, ImGuiData& data) {
			data << typeid(T).name();
		}
	};

	template<>
	struct ToImGui<ReturnStatement>
	{
		static void call(const ReturnStatement& i, ImGuiData& data) {
			std::string return_str = "return " + data.glsl_from_expr(i.m_expr) + ";";
			data.node(return_str, [&] {
				data.print_expr(i.m_expr);
				});
		}
	};

	template<std::size_t NumOverloads>
	struct OverloadImGui
	{
		template<typename T, std::size_t Id>
		struct Get
		{
			static void call(const std::array<FuncOverload, NumOverloads>& overloads, ImGuiData& data, const std::string& fname)
			{
				data.glsl_data.stream.str("");
				data.glsl_data << GLSLTypeStr<T>::get() + " " + fname + "(";
				const auto& args = overloads[Id].args->m_instructions;
				if (get_arg_evaluation_order() == ArgEvaluationOrder::LeftToRight) {
					if (!args.empty())
						retrieve_instruction(args.front())->print_glsl(data.glsl_data);
					for (std::size_t i = 1; i < args.size(); ++i) {
						data.glsl_data << ", ";
						retrieve_instruction(args[i])->print_glsl(data.glsl_data);
					}
				} else {
					if (!args.empty())
						retrieve_instruction(args.back())->print_glsl(data.glsl_data);
					for (std::size_t i = 1; i < args.size(); ++i) {
						data.glsl_data << ", ";
						retrieve_instruction(args[args.size() - i - 1])->print_glsl(data.glsl_data);
					}
				}
				data.glsl_data << ");";

				data.node(data.glsl_data.stream.str() + "##" + std::to_string(Id), [&] {
					for (const auto& i : overloads[Id].body->m_instructions)
						retrieve_instruction(i)->print_imgui(data);
					});
			}
		};

	};

	template<typename ReturnTList, typename ...Fs>
	struct ToImGui<FuncDeclaration<ReturnTList, Fs...>>
	{
		static void call(const FuncDeclaration<ReturnTList, Fs...>& f, ImGuiData& data) {
			const std::string& name = data.glsl_data.register_var_name(f.m_name, f.m_id);
			iterate_over_typelist<ReturnTList, OverloadImGui<sizeof...(Fs)>::template Get>(f.m_overloads, data, name);
		}
	};

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMemberImGui
	{
		static void call(ImGuiData& data) {
			data << (GLSLDeclaration<T>::get(S::get_member_name(Id)) + ";");
		}
	};

	template<typename S>
	struct ToImGui<StructDeclaration<S>>
	{
		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberImGui<S, T, Id>;

		static void call(const StructDeclaration<S>& s, ImGuiData& data) {
			data.node(data.unique(GLSLTypeStr<S>::get()), [&] {
				iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
				});
		}
	};

	template<typename Interface, typename Dimensions, typename Qualifiers>
	struct ToImGui<NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>>
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
	struct UnnamedInterfaceDeclarationMemberImGui
	{
		static void call(const Interface& i, ImGuiData& data) {
			data << (GLSLDeclaration<T>::get(i.m_names[Id]) + ";");
		}
	};

	template<typename ...Qs, typename ...Ts>
	struct ToImGui<UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>>
	{
		using Qualifiers = RemoveArrayFromQualifiers<Qs...>;
		using Interface = UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>;

		template<typename T, std::size_t Id>
		using MemberDeclaration = UnnamedInterfaceDeclarationMemberImGui<Interface, T, Id>;

		static void call(const UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>& s, ImGuiData& data) {
			std::string interface_name;
			if constexpr (Qualifiers::Size > 0)
				interface_name += GLSLQualifier<Qualifiers>::get() + " ";

			interface_name += s.m_name;
			data.node(interface_name, [&] {
				iterate_over_typelist<TList<Ts...>, MemberDeclaration>(s, data);;
				});
		}
	};

	//////////////////////////////////////////////////////////
	// operators

	template<std::size_t N>
	struct ToImGui<ArgSeq<N>>
	{
		static void call(const ArgSeq<N>& seq, ImGuiData& data) {
			if constexpr (N > 0)
				data.print_expr(seq.m_args[0]);

			for (std::size_t i = 1; i < N; ++i) {
				data << ", ";
				data.print_expr(seq.m_args[i]);
			}
		}
	};

	template<>
	struct ToImGui<Reference>
	{
		static void call(const Reference& ref, ImGuiData& data) {
			data << ("$" + std::to_string(ref.m_id));
		}
	};

	template<typename T, std::size_t N, std::size_t...Ds, typename ...Qualifiers>
	struct ToImGui<Constructor<T, N, SizeList<Ds...>, TList<Qualifiers...>>>
	{
		static void call(const Constructor<T, N, SizeList<Ds...>, TList<Qualifiers...>>& ctor, ImGuiData& data) 
		{
			static const std::unordered_map<CtorFlags, std::string> flag_strs = {
				{ CtorFlags::Declaration, "Declaration" },
				{ CtorFlags::Initialisation, "Initialisation" },
				{ CtorFlags::Temporary, "Temporary" },
				{ CtorFlags::Unused, "Unused" },
				{ CtorFlags::FunctionArgument, "FuncArg" },
			};

			data.glsl_data.stream.str("");
			ctor.print_glsl(data.glsl_data);
			data.glsl_data << ";";

			// const CtorFlags switch_flag = ctor.m_flags & CtorFlags::SwitchMask;

			//std::string ctor_str = flag_strs.at(switch_flag) + " " + std::string(typeid(T).name());
			//if (!ctor.m_name.empty()) {
			//	ctor_str += " " + ctor.m_name + " ";
			//}
			//ctor_str += std::string(" $") + std::to_string(ctor.m_variable_id);
			const std::string ctor_str = data.glsl_data.stream.str();

			//using ArrayDimensions = typename T::ArrayDimensions;
			//using Qualifiers = typename T::Qualifiers;

			if constexpr (N == 0)
				data.leaf(ctor_str);
			else
				data.expr_vector_node(ctor_str, ctor.m_args);
		}
	};

	template<>
	struct ToImGui<ArraySubscript>
	{
		static void call(const ArraySubscript& subscript, ImGuiData& data)
		{
			data.glsl_data.stream.str("");
			ToGLSL<ArraySubscript>::call(subscript, data.glsl_data);

			data.node(data.glsl_data.stream.str(), [&] {
				data << "from ";
				data.print_expr(subscript.m_obj);
				data << "at index ";
				data.print_expr(subscript.m_index);
				});
		}
	};

	template<char ...chars>
	struct ToImGui<Swizzling<chars...>>
	{
		static void call(const Swizzling<chars...>& swizzle, ImGuiData& data) {
			data.node(data.glsl_from_expr(swizzle.m_obj), [&] {
				data.print_expr(swizzle.m_obj);
				});

			std::string swizzle_str = ".";
			((swizzle_str += chars), ...);
			data.leaf(swizzle_str);
		}
	};

	template<typename T>
	struct ToImGui<Litteral<T>> {
		static void call(const Litteral<T>& litteral, ImGuiData& data) {
			data.leaf("litteral " + std::string(typeid(typename Infos<T>::ScalarType).name()) + " " + std::to_string(litteral.value));
		}
	};

	template<>
	struct ToImGui<BinaryOperator>
	{
		static void call(const BinaryOperator& bop, ImGuiData& data)
		{
			data.node(data.glsl_from_expr(bop.m_lhs), [&] {
				data.print_expr(bop.m_lhs);
				});
			data << glsl_op_str(bop.m_op);
			data.node(data.glsl_from_expr(bop.m_rhs), [&] {
				data.print_expr(bop.m_rhs);
				});
		}
	};

	template<>
	struct ToImGui<UnaryOperator>
	{
		static void call(const UnaryOperator& uop, ImGuiData& data) {
			data.node(glsl_op_str(uop.m_op), [&] {
				data.print_expr(uop.m_arg);
				});
		}
	};

	template<typename From, typename To>
	struct ToImGui<ConvertorOperator< From, To>>
	{
		static void call(const ConvertorOperator< From, To>& op, ImGuiData& data) {
			data.node("convertor", [&] {
				data.print_expr(op.m_args[0]);
				});
		}
	};

	template<std::size_t N>
	struct ToImGui<FunCall<N>>
	{
		static void call(const FunCall<N>& fun_call, ImGuiData& data) {
			data.expr_vector_node(glsl_op_str(fun_call.m_op), fun_call.m_args);
		}
	};

	template<typename F, typename ReturnType, std::size_t N>
	struct ToImGui<CustomFunCall< F, ReturnType, N>>
	{
		static void call(const CustomFunCall< F, ReturnType, N>& fun_call, ImGuiData& data) {
			data.node("custom function call", [&] {
				ToImGui<Reference>::call(fun_call, data);
				data.node("Args", [&] {
					if constexpr (N == 0)
						data << "no arguments";

					for (std::size_t i = 0; i < N; ++i)
						data.print_expr(fun_call.m_args[i]);
					});
				});
		}
	};

	template<typename S, std::size_t Id>
	struct ToImGui<MemberAccessor<S, Id>>
	{
		static void call(const MemberAccessor<S, Id>& accessor, ImGuiData& data) {

			data.glsl_data.stream.str("");
			ToGLSL<MemberAccessor<S, Id>>::call(accessor, data.glsl_data);
			std::string member_str = data.glsl_data.stream.str();

			data.node(member_str, [&] {
				if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(accessor.m_obj))) {
					if (bool(ctor->m_flags & CtorFlags::Temporary))
						ctor->print_imgui(data);
					else
						data << ("$" + std::to_string(ctor->m_variable_id));
				} else {
					auto accessor_wrapper = dynamic_cast<MemberAccessorBase*>(retrieve_expr(accessor.m_obj));
					accessor_wrapper->print_imgui(data);
				}
				data << "." + S::get_member_name(Id);
				});
		}
	};

	template<>
	struct ToImGui<TernaryOperator>
	{
		static void call(const TernaryOperator& top, ImGuiData& data) {
			data.node("ternary", [&] {
				data.print_expr(top.m_condition);
				data.print_expr(top.m_first);
				data.print_expr(top.m_second);
				});
		}
	};
}

#pragma once

#include <v2/Types.hpp>
#include <v2/InstructionTree.hpp>
#include <v2/Swizzles.hpp>

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include <imgui.h>

namespace v2 {

	const std::string& imgui_op_str(const Op op);

	struct ImGuiData {

		template<typename F>
		ImGuiData& node(const std::string& s, F&& f) {
			if (ImGui::TreeNode((s + "##" + std::to_string(counter++)).c_str())) {
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
		ImGuiData& node_vec(const std::string& s, const std::vector<T>& vs) {
			if (vs.empty()) {
			} else if (vs.size() == 1) {
				vs[0]->print_imgui(*this);
			} else {
				if (ImGui::TreeNode((s + "##" + std::to_string(counter++)).c_str())) {
					for (const auto& v : vs) {
						v->print_imgui(*this);
					}
					ImGui::TreePop();
				}
			}
			return *this;
		}

		ImGuiData& node_vec_expr(const std::string& s, const std::vector<Expr>& vs) {
			if (vs.empty()) {
			} else if (vs.size() == 1) {
				retrieve_expr(vs[0])->print_imgui(*this);
			} else {
				if (ImGui::TreeNode((s + "##" + std::to_string(counter++)).c_str())) {
					for (const auto& v : vs) {
						retrieve_expr(v)->print_imgui(*this);
					}
					ImGui::TreePop();
				}
			}
			return *this;
		}

		ImGuiData& node_vec_instruction(const std::string& s, const std::vector<InstructionIndex>& vs) {
			if (vs.empty()) {
			} else if (vs.size() == 1) {
				retrieve_instruction(vs[0])->print_imgui(*this);
			} else {
				if (ImGui::TreeNode((s + "##" + std::to_string(counter++)).c_str())) {
					for (const auto& v : vs) {
						const auto* i = retrieve_instruction(v);
						i->print_imgui(*this);
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

		std::size_t counter = 0;
	};

	template<typename Delayed>
	struct ControllerImGui<Delayed, ShaderController> {
		static void call(const ShaderController& controller, ImGuiData& data) {
			data.node("Declarations", [&] {
				for (const auto& i : controller.m_declarations->m_instructions) {
					retrieve_instruction(i)->print_imgui(data);
				}
			});

			data.node("Structs", [&] {
				for (const auto& i : controller.m_structs) {
					retrieve_instruction(i)->print_imgui(data);
				}
			});

			data.node("Fonctions", [&] {
				for (const auto& i : controller.m_functions) {
					retrieve_instruction(i)->print_imgui(data);
				}
			});
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
					case_str = "If";
				} else if (k != i.m_cases.size() - 1) {
					case_str = "Else If";
				} else {
					case_str = "Else ";
				}

				data.node(case_str, [&] {
					if (k == 0 || k != i.m_cases.size() - 1) {
						data.node("Condition", [&] {
							retrieve_expr(i.m_cases[k].condition)->print_imgui(data);
						});
					}

					for (const auto& j : i.m_cases[k].body->m_instructions) {
						retrieve_instruction(j)->print_imgui(data);
					}
				});
			}

		}
	};

	template<>
	struct InstructionImGui<WhileInstruction> {
		static void call(const WhileInstruction& i, ImGuiData& data) {
			data.node("While", [&] {
				data.node("Condition", [&] {
					retrieve_expr(i.m_condition)->print_imgui(data);
				});
				for (const auto& i : i.m_body->m_instructions) {
					retrieve_instruction(i)->print_imgui(data);
				}
			});
		}
	};

	template<>
	struct InstructionImGui<ForInstruction> {
		static void call(const ForInstruction& i, ImGuiData& data) {
			data.node("For", [&] {
				data.node_vec_instruction("Args", i.args->m_instructions);
				for (const auto& j : i.body->m_instructions) {
					retrieve_instruction(j)->print_imgui(data);
				}
			});
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
				if (ctor->m_flags && CtorFlags::Temporary) {
					return;
				}
			}

			retrieve_expr(i.m_expr)->print_imgui(data);
		}
	};

	template<>
	struct InstructionImGui<SwitchInstruction> {
		static void call(const SwitchInstruction& i, ImGuiData& data) {
			data.node("Switch", [&] {
				data.node("Condition", [&] {
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
				case_str = "Case";
				retrieve_expr(i.m_label)->print_imgui(data);
			} else {
				case_str = "Default";
			}

			data.node(case_str, [&] {
				for (const auto& j : i.m_body->m_instructions) {
					retrieve_instruction(j)->print_imgui(data);
				}
			});
		}
	};

	template<typename T>
	struct InstructionImGui<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, ImGuiData& data) { }
	};

	template<>
	struct InstructionImGui<ReturnStatement> {
		static void call(const ReturnStatement& i, ImGuiData& data) { }
	};

	template<>
	struct InstructionImGui<FuncDeclarationBase> {
		static void call(const FuncDeclarationBase& f, ImGuiData& data) { }
	};

	template<std::size_t NumOverloads>
	struct OverloadImGui {

		template<typename T, std::size_t Id>
		struct Get {
			static void call(const std::array<OverloadData, NumOverloads>& overloads, ImGuiData& data) {
				data.node(std::string("Overload ") + std::to_string(Id) + std::string(" returns ") + std::string(typeid(T).name()), [&] {
					data.node_vec_instruction("Args", overloads[Id].args->m_instructions);
					data.node("Body", [&] {
						if (overloads[Id].body->m_instructions.empty()) {
							data << "Empty";
						}
						for (const auto& i : overloads[Id].body->m_instructions) {
							retrieve_instruction(i)->print_imgui(data);
						}
					});
				});
			}
		};

	};

	template<typename ReturnTList, typename ...Fs>
	struct InstructionImGui<FuncDeclaration<ReturnTList, Fs...>> {
		static void call(const FuncDeclaration<ReturnTList, Fs...>& f, ImGuiData& data) {
			data.node("Function declaration $" + std::to_string(f.m_id), [&] {
				iterate_over_typelist<ReturnTList, OverloadImGui<sizeof...(Fs)>::template Get>(f.m_overloads, data);
			});
		}
	};

	template<>
	struct InstructionImGui<StructDeclarationBase> {
		static void call(const StructDeclarationBase& f, ImGuiData& data) { }
	};

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMemberImGui {
		static void call(ImGuiData& data) {
			data << (std::string(typeid(T).name()) + " " + S::get_member_name(Id));
		}
	};

	template<typename S>
	struct InstructionImGui<StructDeclaration<S>> {

		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMemberImGui<S, T, Id>;

		static void call(const StructDeclaration<S>& s, ImGuiData& data) {
			data.node(std::string("Struct declaration ") + std::string(typeid(S).name()), [&] {
				iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
			});
		}
	};

	template<typename Interface>
	struct InstructionImGui<NamedInterfaceDeclaration<Interface>> {
		static void call(const NamedInterfaceDeclaration<Interface>& s, ImGuiData& data) {

		}
	};

	template<typename ...Qs, typename ...Ts>
	struct InstructionImGui<UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>> {
		static void call(const UnnamedInterfaceDeclaration<TList<Qs...>, TList<Ts...>>& s, ImGuiData& data) {

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

	//template<>
	//struct OperatorImGui<ConstructorWrapper> {
	//	static void call(const ConstructorWrapper& wrapper, ImGuiData& data) {
	//		wrapper.m_ctor->print_imgui(data);
	//	}
	//};

	template<>
	struct OperatorImGui<ConstructorBase> {
		static void call(const ConstructorBase& ctor, ImGuiData& data) {
			data << "Base ctor";
		}
	};

	template<typename T, std::size_t N>
	struct OperatorImGui<Constructor<T, N>> {

		static void call(const Constructor<T, N>& ctor, ImGuiData& data) {

			static const std::unordered_map<CtorFlags, std::string> flag_strs = {
				{ CtorFlags::Declaration, "Declaration" },
				{ CtorFlags::Initialisation, "Initialisation" },
				{ CtorFlags::Temporary, "Temporary" },
				{ CtorFlags::Unused, "Unused" },
				{ CtorFlags::FunctionArgument, "FuncArg" },
			};

			const CtorFlags switch_flag = ctor.m_flags & CtorFlags::SwitchMask;

			std::string ctor_str = flag_strs.at(switch_flag) + " " + std::string(typeid(T).name());
			if (!ctor.m_name.empty()) {
				ctor_str += " " + ctor.m_name + " ";
			}
			ctor_str += std::string(" $") + std::to_string(ctor.m_variable_id);

			using ArrayDimensions = typename T::ArrayDimensions;
			using Qualifiers = typename T::Qualifiers;

			if constexpr (N == 0 && ArrayDimensions::Size == 0 && Qualifiers::Size == 0) {
				data.leaf(ctor_str);
			} else {
				data.node(ctor_str, [&] {
					if (ctor.m_flags != CtorFlags::Temporary) {
						if constexpr (ArrayDimensions::Size > 0) {
							data.node("array size", [&] {
								ImGuiQualifier<ArraySizePrinterImGui<ArrayDimensions>>::print_glsl(data);
							});

						}

						if constexpr (Qualifiers::Size > 0) {
							data.node("qualifiers", [&] {
								ImGuiQualifier<Qualifiers>::print(data);
							});
						}
					}

					data.node_vec_expr("Args", std::vector<Expr>(ctor.m_args.begin(), ctor.m_args.end()));
				});
			}
		}
	};

	template<>
	struct OperatorImGui<ArraySubscript> {
		static void call(const ArraySubscript& subscript, ImGuiData& data) {
			data.node("Array subscript", [&] {
				data << "from ";
				retrieve_expr(subscript.m_obj)->print_imgui(data);
				data << "at index ";
				retrieve_expr(subscript.m_index)->print_imgui(data);
			});
		}
	};

	//template<>
	//struct OperatorImGui<SwizzlingWrapper> {
	//	static void call(const SwizzlingWrapper& wrapper, ImGuiData& data) {
	//		wrapper.m_swizzle->print_imgui(data);
	//	}
	//};

	template<>
	struct OperatorImGui<SwizzlingBase> {
		static void call(const SwizzlingBase& swizzle, ImGuiData& data) { }
	};


	template<char c, char ... chars>
	struct OperatorImGui<Swizzling<Swizzle<c, chars...>>> {

		static void call(const Swizzling<Swizzle<c, chars...>>& swizzle, ImGuiData& data) {
			data.node("Swizzle", [&] {
				retrieve_expr(swizzle.m_obj)->print_imgui(data);
				std::string swizzle;
				swizzle += c;
				((swizzle += chars), ...);
				data << swizzle;
			});
		}
	};

	template<typename T>
	struct OperatorImGui<Litteral<T>> {
		static void call(const Litteral<T>& litteral, ImGuiData& data) {
			data << (std::string("Constant ") + std::string(typeid(typename Infos<T>::ScalarType).name()) + " " + std::to_string(litteral.value));
		}
	};

	template<>
	struct OperatorImGui<BinaryOperator> {
		static void call(const BinaryOperator& bop, ImGuiData& data) {
			data.node(imgui_op_str(bop.m_op), [&] {
				retrieve_expr(bop.m_lhs)->print_imgui(data);
				retrieve_expr(bop.m_rhs)->print_imgui(data);
			});
		}
	};

	template<>
	struct OperatorImGui<UnaryOperator> {
		static void call(const UnaryOperator& uop, ImGuiData& data) {
			data.node(imgui_op_str(uop.m_op), [&] {
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
			data.node(imgui_op_str(fun_call.m_op), [&] {
				OperatorImGui<ArgSeq<N>>::call(fun_call, data);
			});
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

	//template<>
	//struct OperatorImGui<MemberAccessorWrapper> {
	//	static void call(const MemberAccessorWrapper& wrapper, ImGuiData& data) {
	//		wrapper.m_member_accessor->print_imgui(data);
	//	}
	//};

	template<typename S, std::size_t Id>
	struct OperatorImGui<MemberAccessor<S, Id>> {
		static void call(const MemberAccessor<S, Id>& accessor, ImGuiData& data) {
			data.node("Member accessor", [&] {
				if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(accessor.m_obj))) {
					if (ctor->m_flags && CtorFlags::Temporary) {
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

	inline const std::string& imgui_op_str(const Op op) {
		static const std::unordered_map<Op, std::string> op_strs = {
			{ Op::CWiseMul, "*" },
			{ Op::MatrixTimesScalar, "*" },
			{ Op::MatrixTimesMatrix, "*" },
			{ Op::ScalarTimesMatrix, "*" },
			{ Op::CWiseAdd, " + " },
			{ Op::MatrixAddScalar, " + " },
			{ Op::ScalarAddMatrix, " + " },
			{ Op::CWiseSub, " - " },
			{ Op::MatrixSubScalar, " - " },
			{ Op::ScalarSubMatrix, " - " },
			{ Op::UnaryNegation, "!" },
			{ Op::Assignment, " = " }
		};

		auto it = op_strs.find(op);
		if (it == op_strs.end()) {
			static const std::string undefined = " undefined Op ";
			return undefined;
		} else {
			return it->second;
		}
	}

}

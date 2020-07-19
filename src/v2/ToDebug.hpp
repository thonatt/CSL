#pragma once

#include "InstructionTree.hpp"

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

namespace v2 {

	const std::string& op_str(const Op op);

	struct DebugData {
		std::stringstream stream;
		int trailing = 0;

		DebugData& trail() {
			for (int t = 0; t < trailing; ++t) {
				stream << "|  ";
			}
			return *this;
		}

		DebugData& endl() {
			stream << "\n";
			return *this;
		}

		template<typename T>
		DebugData& operator<<(T&& t) {
			stream << std::forward<T>(t);
			return *this;
		}

	};

	template<typename Delayed>
	struct ControllerDebug<Delayed, ShaderController> {
		static void call(const ShaderController& controller, DebugData& data) {
			for (const auto& i : controller.m_declarations->m_instructions) {
				retrieve_instruction(i)->print_debug(data);
			}

			for (const auto& f : controller.m_functions) {
				retrieve_instruction(f)->print_debug(data);
			}

			for (const auto& s : controller.m_structs) {
				retrieve_instruction(s)->print_debug(data);
			}
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct DebugQualifier {
		static void print_debug(DebugData& data) {
			data << "unspecified qualifier";
		}
	};

	template<typename Q, typename ...Qs>
	struct DebugQualifier<TList<Q, Qs...>> {
		static void print_debug(DebugData& data) {
			data << typeid(Q).name();
			(((data << ", "), data << typeid(Qs).name()), ...);

			//DebugQualifier<Q>::print_debug(data);
			//(((data << ", "), DebugQualifier<Qs>::print_debug(data)), ...);
		}
	};

	//template<>
	//struct DebugQualifier<Uniform> {
	//	static void print_debug(DebugData& data) {
	//		data.stream << "uniform";
	//	}
	//};

	//template<typename T, typename ...Ts>
	//struct DebugQualifier<Layout<T, Ts...>> {
	//	static void print_debug(DebugData& data) {
	//		data.stream << "layout(";
	//		DebugQualifier<T>::print_debug(data);
	//		(((data.stream << ", "), DebugQualifier<Ts>::print_debug(data)), ...);
	//		data.stream << ")";
	//	}
	//};

	//template<std::size_t N>
	//struct DebugQualifier<Binding<N>> {
	//	static void print_debug(DebugData& data) {
	//		data.stream << "binding = " << N;
	//	}
	//};

	template<typename>
	struct ArraySizePrinter { };

	template<size_t ...Ns>
	struct DebugQualifier<ArraySizePrinter<SizeList<Ns...>>> {
		static void print_debug(DebugData& data) {
			((data.stream << "[" << Ns << "]"), ...);
		}
	};

	///////////////////////////////////////////////////////////////////////////////////
	// instructions

	template<>
	struct InstructionDebug<IfInstruction> {
		static void call(const IfInstruction& i, DebugData& data) {

			for (std::size_t k = 0; k < i.m_cases.size(); ++k) {
				data.endl().trail();
				if (k == 0) {
					data << "If ";
					retrieve_expr(i.m_cases[k].condition)->print_debug(data);
				} else if (k != i.m_cases.size() - 1) {
					data << "Else If ";
					retrieve_expr(i.m_cases[k].condition)->print_debug(data);
				} else {
					data << "Else ";
				}

				++data.trailing;
				for (const auto& j : i.m_cases[k].body->m_instructions) {
					retrieve_instruction(j)->print_debug(data);
				}
				--data.trailing;
			}

		}
	};

	template<>
	struct InstructionDebug<WhileInstruction> {
		static void call(const WhileInstruction& i, DebugData& data) {
			data.endl().trail();
			data << "While ";
			retrieve_expr(i.m_condition)->print_debug(data);
			++data.trailing;
			for (const auto& i : i.m_body->m_instructions) {
				retrieve_instruction(i)->print_debug(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct InstructionDebug<ForInstruction> {
		static void call(const ForInstruction& i, DebugData& data) {
			data.endl().trail() << "For";
			++data.trailing;
			data.endl().trail() << "Args";
			++data.trailing;
			for (const auto& arg : i.args->m_instructions) {
				retrieve_instruction(arg)->print_debug(data);
			}
			--data.trailing;
			data.endl().trail() << "Body";
			++data.trailing;
			for (const auto& j : i.body->m_instructions) {
				retrieve_instruction(j)->print_debug(data);
			}
			--data.trailing;
			--data.trailing;
		}
	};

	template<>
	struct InstructionDebug<Statement> {
		static void call(const Statement& i, DebugData& data) {
			if (!i.m_expr) {
				data << "empty expr";
				return;
			}

			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(i.m_expr))) {
				if (ctor->m_flags & CtorFlags::Temporary) {
					return;
				}
			}

			data.endl().trail();
			retrieve_expr(i.m_expr)->print_debug(data);
		}
	};


	template<>
	struct InstructionDebug<ForArgStatement> {
		static void call(const ForArgStatement& i, DebugData& data) {
			retrieve_expr(i.m_expr)->print_debug(data);
		}
	};


	template<>
	struct InstructionDebug<ForIterationStatement> {
		static void call(const ForIterationStatement& i, DebugData& data) {
			retrieve_expr(i.m_expr)->print_debug(data);
		}
	};

	template<>
	struct InstructionDebug<SwitchInstruction> {
		static void call(const SwitchInstruction& i, DebugData& data) {
			data.endl().trail() << "Switch ";
			retrieve_expr(i.m_condition)->print_debug(data);
			++data.trailing;
			for (const auto& c : i.m_body->m_instructions) {
				retrieve_instruction(c)->print_debug(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct InstructionDebug<SwitchCase> {
		static void call(const SwitchCase& i, DebugData& data) {
			data.endl().trail();
			if (i.m_label) {
				data << "Case ";
				retrieve_expr(i.m_label)->print_debug(data);
			} else {
				data << "Default";
			}
			++data.trailing;
			for (const auto& j : i.m_body->m_instructions) {
				retrieve_instruction(j)->print_debug(data);
			}
			--data.trailing;
		}
	};


	template<typename T>
	struct InstructionDebug<SpecialStatement<T>> {
		static void call(const SpecialStatement<T>& i, DebugData& data) { }
	};

	template<>
	struct InstructionDebug<ReturnStatement> {
		static void call(const ReturnStatement& i, DebugData& data) {
			data.endl().trail() << "return";
		}
	};

	template<>
	struct InstructionDebug<FuncDeclarationBase> {
		static void call(const FuncDeclarationBase& f, DebugData& data) { }
	};


	template<std::size_t NumOverloads>
	struct OverloadDebug {

		template<typename T, std::size_t Id>
		struct Get {
			static void call(const std::array<OverloadData, NumOverloads>& overloads, DebugData& data) {
				data.endl().trail() << "Overload " << Id << " returns " << typeid(T).name();
				++data.trailing;
				data.endl().trail() << "Args";
				++data.trailing;

				if (overloads[Id].args->m_instructions.empty()) {
					data.endl().trail();
					data << "None";
				}
				for (const auto& i : overloads[Id].args->m_instructions) {
					retrieve_instruction(i)->print_debug(data);
				}
				--data.trailing;

				data.endl().trail() << "Body";
				++data.trailing;
				if (overloads[Id].body->m_instructions.empty()) {
					data.endl().trail();
					data << "Empty";
				}
				for (const auto& i : overloads[Id].body->m_instructions) {
					retrieve_instruction(i)->print_debug(data);
				}
				--data.trailing;
				--data.trailing;
			}
		};

	};

	template<typename ReturnTList, typename ...Fs>
	struct InstructionDebug<FuncDeclaration<ReturnTList, Fs...>> {
		static void call(const FuncDeclaration<ReturnTList, Fs...>& f, DebugData& data) {
			data.endl().trail() << "Function declaration $" << f.m_id;
			++data.trailing;
			iterate_over_typelist<ReturnTList, OverloadDebug<sizeof...(Fs)>::template Get>(f.m_overloads, data);
			--data.trailing;
		}
	};

	template<>
	struct InstructionDebug<StructDeclarationBase> {
		static void call(const StructDeclarationBase& f, DebugData& data) { }
	};

	template<typename S, typename T, std::size_t Id>
	struct StructDeclarationMember {
		static void call(DebugData& data) {
			data.endl().trail() << typeid(T).name() << " " << S::get_member_name(Id);
		}
	};

	template<typename S>
	struct InstructionDebug<StructDeclaration<S>> {

		template<typename T, std::size_t Id>
		using StructMemberDeclaration = StructDeclarationMember<S, T, Id>;

		static void call(const StructDeclaration<S>& s, DebugData& data) {
			data.endl().trail() << "Struct declaration " << typeid(S).name();
			++data.trailing;
			iterate_over_typelist<typename S::MemberTList, StructMemberDeclaration>(data);
			--data.trailing;
		}
	};
	// operators

	template<std::size_t N>
	struct OperatorDebug<ArgSeq<N>> {
		static void call(const ArgSeq<N>& seq, DebugData& data) {
			data << "(";
			if constexpr (N > 0) {
				retrieve_expr(seq.m_args[0])->print_debug(data);
			}
			for (std::size_t i = 1; i < N; ++i) {
				data << ", ";
				retrieve_expr(seq.m_args[i])->print_debug(data);
			}
			data << ")";
		}
	};

	template<>
	struct OperatorDebug<Reference> {
		static void call(const Reference& ref, DebugData& data) {
			data << "$" << ref.m_id;
		}
	};

	template<>
	struct OperatorDebug<ConstructorBase> {
		static void call(const ConstructorBase& ctor, DebugData& data) {
			data << " base ctor";
		}
	};

	template<typename T, std::size_t N>
	struct OperatorDebug<Constructor<T, N>> {

		static void call(const Constructor<T, N>& ctor, DebugData& data) {

			static const std::unordered_map<CtorFlags, std::string> flag_strs = {
				{ CtorFlags::Declaration, "Declaration" },
				{ CtorFlags::Initialisation, "Initialisation" },
				{ CtorFlags::Temporary, "Temporary" },
				{ CtorFlags::Unused, "Unused" },
				{ CtorFlags::FunctionArgument, "FunctionArgument" },
			};

			const CtorFlags without_const = ctor.m_flags && (~CtorFlags::Const);

			data << flag_strs.at(without_const) << " " << typeid(T).name() << " $" << ctor.m_variable_id;
			if (without_const != CtorFlags::Temporary) {
				using ArrayDimensions = typename T::ArrayDimensions;
				if constexpr (ArrayDimensions::Size > 0) {
					data.endl().trail() << "  array size : ";
					DebugQualifier<ArraySizePrinter<ArrayDimensions>>::print_debug(data);
				}

				using Qualifiers = typename T::Qualifiers;
				if constexpr (Qualifiers::Size > 0) {
					data.endl().trail() << "  qualifiers : ";
					DebugQualifier<Qualifiers>::print_debug(data);
				}
			}

			++data.trailing;
			for (std::size_t i = 0; i < N; ++i) {
				data.endl().trail();
				retrieve_expr(ctor.m_args[i])->print_debug(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<ArraySubscript> {
		static void call(const ArraySubscript& subscript, DebugData& data) {
			data << "Array subscript";
			++data.trailing;
			data.endl().trail() << "from ";
			retrieve_expr(subscript.m_obj)->print_debug(data);
			data.endl().trail() << "at index ";
			retrieve_expr(subscript.m_index)->print_debug(data);
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<SwizzlingBase> {
		static void call(const SwizzlingBase& swizzle, DebugData& data) { }
	};


	template<char c, char ... chars>
	struct OperatorDebug<Swizzling<Swizzle<c, chars...>>> {

		static void call(const Swizzling<Swizzle<c, chars...>>& swizzle, DebugData& data) {
			data << "Swizzle";
			++data.trailing;
			data.endl().trail();
			retrieve_expr(swizzle.m_obj)->print_debug(data);
			data.endl().trail() << c;
			((data << chars), ...);
			--data.trailing;
		}
	};

	template<typename T>
	struct OperatorDebug<Litteral<T>> {
		static void call(const Litteral<T>& litteral, DebugData& data) {
			data << "Constant " << typeid(typename Infos<T>::ScalarType).name() << " ";
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
	struct OperatorDebug<BinaryOperator> {
		static void call(const BinaryOperator& bop, DebugData& data) {
			data << op_str(bop.m_op);
			++data.trailing;
			data.endl().trail();
			retrieve_expr(bop.m_lhs)->print_debug(data);
			data.endl().trail();
			retrieve_expr(bop.m_rhs)->print_debug(data);
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<UnaryOperator> {
		static void call(const UnaryOperator& uop, DebugData& data) {
			data << op_str(uop.m_op);
			++data.trailing;
			data.endl().trail();
			retrieve_expr(uop.m_arg)->print_debug(data);
			--data.trailing;
		}
	};


	template<typename From, typename To>
	struct OperatorDebug<ConvertorOperator<From, To>> {
		static void call(const ConvertorOperator< From, To>& op, DebugData& data) {
			data << "convertor ";
			OperatorDebug<ArgSeq<1>>::call(op, data);
		}
	};


	template<std::size_t N>
	struct OperatorDebug<FunCall<N>> {
		static void call(const FunCall<N>& fun_call, DebugData& data) {
			data << imgui_op_str(fun_call.m_op);
			OperatorDebug<ArgSeq<N>>::call(fun_call, data);
		}
	};

	template<typename F, typename ReturnType, std::size_t N>
	struct OperatorDebug<CustomFunCall< F, ReturnType, N>> {
		static void call(const CustomFunCall< F, ReturnType, N>& fun_call, DebugData& data) {
			data << "custom function call ";
			OperatorDebug<Reference>::call(fun_call, data);
			++data.trailing;
			if constexpr (N == 0) {
				data.endl().trail() << "no arguments";
			}
			for (std::size_t i = 0; i < N; ++i) {
				data.endl().trail();
				retrieve_expr(fun_call.m_args[i])->print_debug(data);
			}
			--data.trailing;
		}
	};

	template<typename S, std::size_t Id>
	struct OperatorDebug<MemberAccessor<S, Id>> {
		static void call(const MemberAccessor<S, Id>& accessor, DebugData& data) {
			data << "Member accessor";
			++data.trailing;
			data.endl().trail();
			if (auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(accessor.m_obj))) {
				if (ctor->m_flags & CtorFlags::Temporary) {
					ctor->print_debug(data);
				} else {
					data << "$" << ctor->m_variable_id;
				}
			} else {
				auto accessor_wrapper = dynamic_cast<MemberAccessorBase*>(retrieve_expr(accessor.m_obj));
				accessor_wrapper->print_debug(data);
			}
			data.endl().trail() << S::get_member_name(Id);
			--data.trailing;
		}
	};

	inline const std::string& op_str(const Op op) {
		static const std::unordered_map<Op, std::string> op_strs = {
			{ Op::CWiseMul, "CWiseMul" },
			{ Op::MatrixTimesScalar, "MatrixTimesScalar" },
			{ Op::MatrixTimesMatrix, "MatrixTimesMatrix" },
			{ Op::ScalarTimesMatrix, "ScalarTimesMatrix" },
			{ Op::CWiseAdd, "CWiseAdd" },
			{ Op::MatrixAddScalar, "MatrixAddScalar" },
			{ Op::CWiseSub, "CWiseSub" },
			{ Op::MatrixSubScalar, "MatrixSubScalar" },
			{ Op::ScalarSubMatrix, "ScalarSubMatrix" },
			{ Op::UnaryNegation, "UnaryNegation" },
			{ Op::Assignment, "Assignment" }
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

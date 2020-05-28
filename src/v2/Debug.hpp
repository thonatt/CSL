#pragma once

#include "ShaderTree.hpp"

#include <sstream>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

namespace v2 {

	const std::string& op_str(const Op op);

	struct DebugData {
		std::stringstream stream;
		std::size_t trailing = 0;

		DebugData& add_trail() {
			for (std::size_t t = 0; t < trailing; ++t) {
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

	/////////////////////////////////////////////////////////////////////////////////////////
	// qualifiers

	template<typename QList>
	struct DebugQualifier {
		static void print_debug(DebugData& data) {
			data.stream << "unspecified qualifier";
		}
	};

	template<typename Q, typename ...Qs>
	struct DebugQualifier<TList<Q, Qs...>> {
		static void print_debug(DebugData& data) {
			DebugQualifier<Q>::print_debug(data);
			(((data.stream << ", "), DebugQualifier<Qs>::print_debug(data)), ...);
		}
	};

	template<>
	struct DebugQualifier<Uniform> {
		static void print_debug(DebugData& data) {
			data.stream << "uniform";
		}
	};

	template<typename T, typename ...Ts>
	struct DebugQualifier<Layout<T, Ts...>> {
		static void print_debug(DebugData& data) {
			data.stream << "layout(";
			DebugQualifier<T>::print_debug(data);
			(((data.stream << ", "), DebugQualifier<Ts>::print_debug(data)), ...);
			data.stream << ")";
		}
	};

	template<std::size_t N>
	struct DebugQualifier<Binding<N>> {
		static void print_debug(DebugData& data) {
			data.stream << "binding = " << N;
		}
	};

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
	struct InstructionDebug<SwitchInstruction> {
		static void call(const SwitchInstruction& i, DebugData& data) {

		}
	};

	template<>
	struct InstructionDebug<SwitchCase> {
		static void call(const SwitchCase& i, DebugData& data) {

		}
	};

	template<>
	struct InstructionDebug<IfInstruction> {
		static void call(const IfInstruction& i, DebugData& data) {

		}
	};

	template<>
	struct InstructionDebug<ForInstruction> {
		static void call(const ForInstruction& i, DebugData& data) {

		}
	};

	template<>
	struct InstructionDebug<Statement> {
		static void call(const Statement& i, DebugData& data) {
			if (!i.m_expr) {
				return;
			}

			if (auto ctor = std::dynamic_pointer_cast<OperatorWrapper<ConstructorWrapper>>(i.m_expr)) {
				if (ctor->m_operator.m_ctor->m_flags & CtorFlags::Temporary) {
					return;
				}
			}

			i.m_expr->print_debug(data);
			data.endl();
		}
	};


	template<>
	struct InstructionDebug<FuncDeclarationWrapper> {
		static void call(const FuncDeclarationWrapper& wrapper, DebugData& data) {
			wrapper.m_func->print_debug(data);
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
				data.add_trail() << "Overload " << Id << " " << typeid(T).name();
				++data.trailing;
				data.endl().add_trail() << "Args";
				++data.trailing;

				if (overloads[Id].args->m_instructions.size() == 0) {
					data.endl().add_trail();
					data << "None";
					data.endl();
				}
				for (const auto& i : overloads[Id].args->m_instructions) {
					data.endl().add_trail();
					i->print_debug(data);
				}
				--data.trailing;

				data.add_trail() << "Body";
				++data.trailing;
				if (overloads[Id].body->m_instructions.size() == 0) {
					data.endl().add_trail();
					data << "Empty";
					data.endl();
				}
				for (const auto& i : overloads[Id].body->m_instructions) {
					data.endl().add_trail();
					i->print_debug(data);
				}
				--data.trailing;
				--data.trailing;
			}
		};

	};


	template<typename ReturnTList, typename ...Fs>
	struct InstructionDebug<FuncDeclarationInstruction<ReturnTList, Fs...>> {
		static void call(const FuncDeclarationInstruction<ReturnTList, Fs...>& f, DebugData& data) {
			data.add_trail() << "Function declaration $" << f.m_id;
			++data.trailing;
			data.endl();

			iterate_over_typelist<ReturnTList, OverloadDebug<sizeof...(Fs)>::template Get>(f.m_overloads, data);
			--data.trailing;
		}
	};

	// operators


	template<>
	struct OperatorDebug<Reference> {
		static void call(const Reference& ref, DebugData& data) {
			data.stream << "$" << ref.m_id;
		}
	};

	template<>
	struct OperatorDebug<ConstructorWrapper> {
		static void call(const ConstructorWrapper& wrapper, DebugData& data) {
			wrapper.m_ctor->print_debug(data);
		}
	};

	template<>
	struct OperatorDebug<ConstructorBase> {
		static void call(const ConstructorBase& ctor, DebugData& data) { }
	};

	template<typename T, std::size_t N>
	struct OperatorDebug<Constructor<T, N>> {

		static void call(const Constructor<T, N>& ctor, DebugData& data) {

			static const std::unordered_map<CtorFlags, std::string> flag_strs = {
				{ CtorFlags::Declaration, "Declaration" },
				{ CtorFlags::Forward, "Forward" },
				{ CtorFlags::Initialisation, "Initialisation" },
				{ CtorFlags::Temporary, "Temporary" },
			};

			data << flag_strs.at(ctor.m_flags) << " " << typeid(T).name() << " $" << ctor.m_variable_id;

			if (ctor.m_flags != CtorFlags::Temporary) {
				using ArrayDimensions = typename T::ArrayDimensions;
				if constexpr (ArrayDimensions::Size > 0) {
					data.endl().add_trail() << "  array size : ";
					DebugQualifier<ArraySizePrinter<ArrayDimensions>>::print_debug(data);
				}

				using Qualifiers = typename T::Qualifiers;
				if constexpr (Qualifiers::Size > 0) {
					data.endl().add_trail() << "  qualifiers : ";
					DebugQualifier<Qualifiers>::print_debug(data);
				}
			}

			++data.trailing;
			for (std::size_t i = 0; i < N; ++i) {
				data.endl().add_trail();
				ctor.m_args[i]->print_debug(data);
			}
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<ArraySubscript> {
		static void call(const ArraySubscript& subscript, DebugData& data) {
			data << "Array subscript";
			++data.trailing;
			data.endl().add_trail() << "from ";
			subscript.m_obj->print_debug(data);
			data.endl().add_trail() << "at index ";
			subscript.m_index->print_debug(data);
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<SwizzlingWrapper> {
		static void call(const SwizzlingWrapper& wrapper, DebugData& data) {
			wrapper.m_swizzle->print_debug(data);
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
			data.endl().add_trail();
			swizzle.m_obj->print_debug(data);
			data.endl().add_trail() << c;
			((data << chars), ...);
			--data.trailing;
		}
	};

	template<typename T>
	struct OperatorDebug<Litteral<T>> {
		static void call(const Litteral<T>& litteral, DebugData& data) {
			data << "Constant " << typeid(typename Infos<T>::ScalarType).name() << " ";
			if (std::is_integral_v<T>) {
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
			data.endl().add_trail();
			bop.m_lhs->print_debug(data);
			data.endl().add_trail();
			bop.m_rhs->print_debug(data);
			--data.trailing;
		}
	};

	template<>
	struct OperatorDebug<UnaryOperator> {
		static void call(const UnaryOperator& uop, DebugData& data) {
			data << op_str(uop.m_op);
			++data.trailing;
			data.endl().add_trail();
			uop.m_arg->print_debug(data);
			--data.trailing;
		}
	};

	template<typename F, std::size_t N>
	struct OperatorDebug<CustomFunCall<F, N>> {
		static void call(const CustomFunCall<F, N>& fun_call, DebugData& data) {
			data << "custom function call ";
			OperatorDebug<Reference>::call(fun_call, data);
			++data.trailing;
			if constexpr (N == 0) {
				data.endl().add_trail() << "no arguments";
			}
			for (std::size_t i = 0; i < N; ++i) {
				data.endl().add_trail();
				fun_call.m_args[i]->print_debug(data);
			}
			--data.trailing;
		}
	};

	inline const std::string& op_str(const Op op) {
		static const std::unordered_map<Op, std::string> op_strs = {
			{ Op::CWiseMul, "CWiseMul" },
			{ Op::MatrixTimesScalar, "MatrixTimesScalar" },
			{ Op::MatrixTimesMatrix, "MatrixTimesMatrix" },
			{ Op::CWiseAdd, "CWiseAdd" },
			{ Op::MatrixAddScalar, "MatrixAddScalar" },
			{ Op::CWiseSub, "CWiseSub" },
			{ Op::MatrixSubScalar, "MatrixSubScalar" },
			{ Op::UnaryNegation, "UnaryNegation" }
		};

		return op_strs.at(op);
	}

}

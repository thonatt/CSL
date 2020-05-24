#pragma once

#include "ShaderTree.hpp"

#include <sstream>
#include <typeinfo>

namespace v2 {

	struct DebugData {
		std::stringstream stream;
		std::size_t trailing = 0;
	};

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
			if (i.m_expr) {
				i.m_expr->print_debug(data);
			}
			
		}
	};

	// operators

	template<>
	struct OperatorDebug<ConstructorBase> {
		static void call(const ConstructorBase& litteral, DebugData& data) {
		}
	};

	template<>
	struct OperatorDebug<Reference> {
		static void call(const Reference& ref, DebugData& data) {
			data.stream << "$" << ref.m_id;
		}
	};

	template<>
	struct OperatorDebug<ConstructorWrapper> {
		static void call(const ConstructorWrapper& wrapper, DebugData& data) {
			wrapper.m_ctor;
		}
	};

	template<typename T, std::size_t N>
	struct OperatorDebug<Constructor<T, N>> {
		static void call(const Constructor<T, N>& ctor, DebugData& data) {
			data.stream << "constructor " << typeid(T).name() << " " << static_cast<std::size_t>(ctor.m_flags) << "\n";
			++data.trailing;
			for (std::size_t i = 0; i < N; ++i) {
				for (std::size_t t = 0; t < data.trailing; ++t) {
					data.stream << "\t";
				}
				ctor.arguments[i]->print_debug(data);
				if (i != N - 1) {
					data.stream << "\n";
				}			
			}
			--data.trailing;
		}
	};

	template<typename T>
	struct OperatorDebug<Litteral<T>> {
		static void call(const Litteral<T>& litteral, DebugData& data) {
			data.stream << litteral.value;
		}
	};
}

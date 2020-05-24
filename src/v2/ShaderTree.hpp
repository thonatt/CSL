#pragma once

#include "Operators.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>

namespace v2 {

	struct InstructionBase {
		using Ptr = std::shared_ptr<InstructionBase>;
		virtual ~InstructionBase() = default;

		virtual void print_debug(DebugData& data) const { }
	};

	template<typename Instruction>
	struct InstructionDebug;

	struct DebugData;

	template<typename Instruction>
	struct InstructionWrapper : InstructionBase {
		using Ptr = std::shared_ptr<InstructionWrapper>;

		virtual void print_debug(DebugData& data) const override {
			InstructionDebug<Instruction>::call(m_instruction, data);
		}

		template<typename ...Args>
		InstructionWrapper(Args&& ...args) : m_instruction{ std::forward<Args>(args)... } {}

		Instruction m_instruction;
	};

	template <typename Instruction, typename ... Args>
	InstructionBase::Ptr make_instruction(Args&& ...args) 
	{
		return std::static_pointer_cast<InstructionBase>(std::make_shared<InstructionWrapper<Instruction>>(std::forward<Args>(args)...));
	}

	struct Block {
		using Ptr = std::shared_ptr<Block>;

		Block(const Block::Ptr& parent = {}) : m_parent(parent) {}

		virtual ~Block() = default;

		virtual void push_instruction(const InstructionBase::Ptr& i) {
			m_instructions.push_back(i);
		}

		std::vector<InstructionBase::Ptr> m_instructions;
		Block::Ptr m_parent;
	};

	struct MainBlock : Block {
		using Ptr = std::shared_ptr<MainBlock>;
	};

	struct ReturnBlockBase : Block {
		using Ptr = std::shared_ptr<ReturnBlockBase>;
		using Block::Block;

		virtual ~ReturnBlockBase() = default;
	};

	template<typename ReturnType>
	struct ReturnBlock : ReturnBlockBase {
		using Ptr = std::shared_ptr<ReturnBlock<ReturnType>>;

		ReturnBlock(const Block::Ptr& parent = {}) : ReturnBlockBase(parent) {}
		virtual ~ReturnBlock() = default;
	};

	struct Statement {
		virtual ~Statement() = default;

		Statement(const Expr& expr) : m_expr(expr) {}
		Expr m_expr;
	};

	inline InstructionBase::Ptr make_statement(const Expr& expr) {
		return std::static_pointer_cast<InstructionBase>(std::make_shared<InstructionWrapper<Statement>>(expr));
	}

	struct SpecialStatement : Statement {
		using Statement::Statement;
		virtual ~SpecialStatement() = default;
	};

	struct ReturnStatement : SpecialStatement {
		using Ptr = std::shared_ptr<ReturnStatement>;

		ReturnStatement(const Expr& expr) : SpecialStatement(expr) {}

	};

	struct ContinueStatement : SpecialStatement {
	};

	struct DiscardStatement : SpecialStatement {
	};

	struct BreakStatement : SpecialStatement {
	};

	struct EmitVertexInstruction {
	};

	struct EndPrimitiveInstruction {
	};

	struct OverloadData {
		OverloadData() {
			args = std::make_shared<Block>();
			body = std::make_shared<Block>();
		}
		Block::Ptr args, body;
	};

	struct FuncDeclarationBase {
		using Ptr = std::shared_ptr<FuncDeclarationBase>;

		FuncDeclarationBase(const std::string& func_name) : m_func_name(func_name) {
		}

		const std::string m_func_name;
		std::vector<OverloadData> m_overloads;
	};

	template<typename ReturnTList, typename ... Fs>
	struct FuncDeclarationInstruction : FuncDeclarationBase {
		using Ptr = std::shared_ptr<FuncDeclarationInstruction>;

		constexpr static std::size_t N = sizeof...(Fs);

		FuncDeclarationInstruction(const std::string& name) : FuncDeclarationBase(name) {
			m_overloads.resize(N);
		}

		//std::array<OverloadData, N> m_overloads
	};

	struct ForArgsBlock : Block {
		using Ptr = std::shared_ptr<ForArgsBlock>;
		using Block::Block;

		Expr m_stacked_condition;

	};

	struct ForInstruction {
		ForInstruction() {
			args = std::make_shared<ForArgsBlock>();
			body = std::make_shared<Block>();
		}

		ForArgsBlock::Ptr args;
		Block::Ptr body;
	};

	struct IfInstruction {
		using Ptr = std::shared_ptr<InstructionWrapper<IfInstruction>>;

		struct IfCase {
			Expr condition;
			Block::Ptr body;
		};

		IfInstruction(const Ptr& parent) : m_parent_if(parent) {}

		std::vector<IfCase> m_cases;
		Ptr m_parent_if;
		bool waiting_for_else = false;
	};

	struct WhileInstruction : InstructionBase {
		using Ptr = std::shared_ptr<InstructionWrapper<WhileInstruction>>;

		WhileInstruction(const Expr& expr, const Block::Ptr& parent) {
			m_condition = expr;
			m_body = std::make_shared<Block>(parent);
		}

		Expr m_condition;
		Block::Ptr m_body;
	};

	struct SwitchCase {
		using Ptr = std::shared_ptr<InstructionWrapper<SwitchCase>>;

		SwitchCase(const Expr& expr, const Block::Ptr& parent) {
			m_label = expr;
			m_body = std::make_shared<Block>(parent);
		}

		Expr m_label;
		Block::Ptr m_body;
	};

	struct SwitchInstruction {
		using Ptr = std::shared_ptr<InstructionWrapper<SwitchInstruction>>;

		SwitchInstruction(const Expr& expr, const Block::Ptr& parent, const Ptr& parent_switch) {
			m_condition = expr;
			m_body = std::make_shared<Block>(parent);
			m_parent_switch = m_parent_switch;
		}

		void add_case(const Expr& expr, Block::Ptr& current_block) {
			m_current_case = std::make_shared<InstructionWrapper<SwitchCase>>(expr, m_body);
			m_body->push_instruction(m_current_case);
			current_block = m_current_case->m_instruction.m_body;
		}

		Expr m_condition;
		Block::Ptr m_body;
		SwitchCase::Ptr m_current_case;
		SwitchInstruction::Ptr m_parent_switch;
	};


	template<typename Struct>
	struct StructDeclaration {
	};

	template<typename Interface>
	struct UnnamedInterfaceDeclaration  {
	};

	template<typename Interface>
	struct InterfaceDeclaration {
	};

}

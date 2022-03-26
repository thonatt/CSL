#pragma once

#include "Operators.hpp"

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace csl {

	struct InstructionBase
	{
		virtual ~InstructionBase() = default;

		virtual void print_imgui(ImGuiData& data) const = 0;
		virtual void print_glsl(GLSLData& data) const = 0;
	};

	template<typename Instruction>
	struct InstructionImGui;

	template<typename Instruction>
	struct InstructionGLSL;

	using InstructionIndex = Expr;

	template <typename Instruction, typename ... Args>
	InstructionIndex make_instruction(Args&& ...args);

	InstructionBase* retrieve_instruction(const InstructionIndex index);

	/////////////////////////////////////////

	struct Block {
		using Ptr = std::shared_ptr<Block>;

		Block(Block* parent = nullptr) : m_parent(parent) {}

		virtual ~Block() = default;

		virtual void push_instruction(const InstructionIndex i) {
			m_instructions.push_back(i);
		}

		std::vector<InstructionIndex> m_instructions;
		Block* m_parent;
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

		ReturnBlock(Block* parent = {}) : ReturnBlockBase(parent) {}
		virtual ~ReturnBlock() = default;
	};

	template<typename Delayed>
	struct StatementDelayed : InstructionBase {
		virtual ~StatementDelayed() = default;

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<StatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<StatementDelayed>::call(*this, data);
		}

		StatementDelayed(const Expr expr) : m_expr(expr) {}
		Expr m_expr;
	};
	using Statement = StatementDelayed<Dummy>;

	struct FunctionArgBlock : Block {
		using Ptr = std::shared_ptr<FunctionArgBlock>;
		void push_instruction(const InstructionIndex i) override {
			const Expr expr = dynamic_cast<Statement*>(retrieve_instruction(i))->m_expr;
			auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(expr));
			ctor->m_flags = CtorFlags::FunctionArgument;
			m_instructions.push_back(i);
		}
	};

	template<typename Delayed>
	struct ReturnStatementDelayed final : Statement {

		ReturnStatementDelayed() : Statement(Expr()) {}

		template<typename T>
		ReturnStatementDelayed(T&& t) : Statement(get_expr(std::forward<T>(t))) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ReturnStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<ReturnStatementDelayed>::call(*this, data);
		}
	};
	using ReturnStatement = ReturnStatementDelayed<Dummy>;

	template<typename Delayed>
	struct ForArgStatementDelayed final : Statement {
		ForArgStatementDelayed(const Expr expr) : Statement(expr) { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForArgStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<ForArgStatementDelayed>::call(*this, data);
		}
	};
	using ForArgStatement = ForArgStatementDelayed<Dummy>;

	template<typename Delayed>
	struct ForIterationStatementDelayed final : Statement {
		ForIterationStatementDelayed(const Expr& expr) : Statement(expr) { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForIterationStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<ForIterationStatementDelayed>::call(*this, data);
		}
	};
	using ForIterationStatement = ForIterationStatementDelayed<Dummy>;

	template<typename T>
	struct SpecialStatement final : InstructionBase {

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SpecialStatement>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<SpecialStatement>::call(*this, data);
		}
	};

	struct Continue {};

	struct Discard {};

	struct Break {};

	struct EmitVertexI {};

	struct EndPrimitiveI {};

	template<typename ...Qs>
	struct InInstruction {};

	template<typename ...Qs>
	struct OutInstruction {};

	struct OverloadData {
		OverloadData() {
			args = std::make_shared<FunctionArgBlock>();
			body = std::make_shared<Block>();
		}
		Block::Ptr args, body;
	};

	struct FuncDeclarationBase : InstructionBase {
		virtual ~FuncDeclarationBase() = default;

		FuncDeclarationBase(const std::string& name, const std::size_t fun_id) : m_name(name), m_id(fun_id) {
		}

		virtual void print_imgui(ImGuiData& data) const override {}
		virtual void print_glsl(GLSLData& data) const override {}

		virtual const OverloadData& get_overload(const std::size_t i) const = 0;
		virtual std::size_t overload_count() const = 0;

		std::string m_name;
		std::size_t m_id;
	};

	template<typename ReturnTList, typename ... Fs>
	struct FuncDeclaration final : FuncDeclarationBase {

		static constexpr std::size_t N = sizeof...(Fs);

		FuncDeclaration(const std::string& name, const std::size_t fun_id) : FuncDeclarationBase(name, fun_id) {
		}

		const OverloadData& get_overload(const std::size_t i) const override {
			return m_overloads[i];
		}

		virtual std::size_t overload_count() const override {
			return N;
		}

		void print_imgui(ImGuiData& data) const override {
			InstructionImGui<FuncDeclaration>::call(*this, data);
		}
		void print_glsl(GLSLData& data) const override {
			InstructionGLSL<FuncDeclaration>::call(*this, data);
		}

		std::array<OverloadData, N> m_overloads;
	};

	struct ForArgsBlock : Block {
		using Ptr = std::shared_ptr<ForArgsBlock>;
		using Block::Block;

		void push_instruction(const InstructionIndex i) override {
			const Expr expr = dynamic_cast<Statement*>(retrieve_instruction(i))->m_expr;
			//auto ctor = dynamic_cast<ConstructorBase*>(retrieve_expr(expr));
			//if (ctor->m_flags && CtorFlags::Declaration) {
			//	assert(m_instructions.empty());
			//	m_instructions.push_back(make_instruction<ForArgStatement>(expr));
			//} else {
			//	if (m_instructions.size() <= 1) {
			//		m_instructions.push_back(make_instruction<ForArgStatement>(expr));
			//	} else {
			//		m_instructions.push_back(make_instruction<ForIterationStatement>(expr));
			//	}
			//}

			m_instructions.push_back(make_instruction<ForArgStatement>(expr));

			//if (ctor->m_flags && CtorFlags::Initialisation) {
			//	if (m_instructions.size() <= 1) {
			//		m_instructions.push_back(make_instruction<ForArgStatement>(expr));
			//	} else {
			//		m_instructions.push_back(make_instruction<ForIterationStatement>(expr));
			//	}
			//} else {
			//	assert(false);
			//}
		}

		Expr m_stacked_condition;
	};

	template<typename Delayed>
	struct ForInstructionDelayed final : InstructionBase {
		ForInstructionDelayed() {
			args = std::make_shared<ForArgsBlock>();
			body = std::make_shared<Block>();
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<ForInstructionDelayed>::call(*this, data);
		}

		ForArgsBlock::Ptr args;
		Block::Ptr body;
	};
	using ForInstruction = ForInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct IfInstructionDelayed final : InstructionBase {

		struct IfCase {
			Expr condition;
			Block::Ptr body;
		};

		IfInstructionDelayed(const InstructionIndex parent_if) : m_parent_if(parent_if) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<IfInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<IfInstructionDelayed>::call(*this, data);
		}

		std::vector<IfCase> m_cases;
		InstructionIndex m_parent_if;
		bool waiting_for_else = false;
	};
	using IfInstruction = IfInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct WhileInstructionDelayed final : InstructionBase {

		WhileInstructionDelayed(const Expr expr, Block* parent_block) {
			m_condition = expr;
			m_body = std::make_shared<Block>(parent_block);
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<WhileInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<WhileInstructionDelayed>::call(*this, data);
		}

		Expr m_condition;
		Block::Ptr m_body;
	};
	using WhileInstruction = WhileInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct SwitchCaseDelayed final : InstructionBase {

		SwitchCaseDelayed(const Expr& expr, Block* parent) {
			m_label = expr;
			m_body = std::make_shared<Block>(parent);
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SwitchCaseDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<SwitchCaseDelayed>::call(*this, data);
		}

		Expr m_label;
		Block::Ptr m_body;
	};
	using SwitchCase = SwitchCaseDelayed<Dummy>;

	template<typename Delayed>
	struct SwitchInstructionDelayed final : InstructionBase {

		SwitchInstructionDelayed(const Expr expr, Block* parent, const InstructionIndex parent_switch) {
			m_condition = expr;
			m_body = std::make_shared<Block>(parent);
			m_parent_switch = parent_switch;
		}

		void add_case(const Expr expr, Block*& current_block) {
			m_current_case = make_instruction<SwitchCase>(expr, m_body.get());
			m_body->push_instruction(m_current_case);
			current_block = dynamic_cast<SwitchCase*>(retrieve_instruction(m_current_case))->m_body.get();
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SwitchInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<SwitchInstructionDelayed>::call(*this, data);
		}

		Expr m_condition;
		Block::Ptr m_body;
		InstructionIndex m_current_case;
		InstructionIndex m_parent_switch;
	};
	using SwitchInstruction = SwitchInstructionDelayed<Dummy>;

	struct StructDeclarationBase : InstructionBase {
		virtual ~StructDeclarationBase() = default;

		virtual void print_imgui(ImGuiData& data) const override { }
		virtual void print_glsl(GLSLData& data) const override { }
	};

	template<typename Struct>
	struct StructDeclaration final : StructDeclarationBase {
		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<StructDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<StructDeclaration>::call(*this, data);
		}
	};

	template<typename Interface>
	struct NamedInterfaceDeclaration final : InstructionBase {

		NamedInterfaceDeclaration(const std::string& name) : m_name(name) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<NamedInterfaceDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<NamedInterfaceDeclaration>::call(*this, data);
		}

		std::string m_name;
	};

	template<typename QualifierList, typename TypeList>
	struct UnnamedInterfaceDeclaration final : InstructionBase
	{
		template<typename ...Strings>
		UnnamedInterfaceDeclaration(Strings&& ...names) : m_names{ names ... } { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<UnnamedInterfaceDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			InstructionGLSL<UnnamedInterfaceDeclaration>::call(*this, data);
		}

		std::array<std::string, 1 + TypeList::Size> m_names;
	};



}

#pragma once

#include <csl/Operators.hpp>
#include <csl/TemplateHelpers.hpp>

#include <array>
#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace csl
{
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

	struct Scope
	{
		Scope(Scope* parent = nullptr) : m_parent(parent) {}
		virtual ~Scope() = default;

		virtual void push_instruction(const InstructionIndex i) {
			m_instructions.push_back(i);
		}

		std::vector<InstructionIndex> m_instructions;
		Scope* m_parent;
	};

	struct ReturnScopeBase : Scope
	{
		using Scope::Scope;
		virtual ~ReturnScopeBase() = default;
	};

	template<typename ReturnType>
	struct ReturnScope : ReturnScopeBase
	{
		ReturnScope(Scope* parent = {}) : ReturnScopeBase(parent) {}
		virtual ~ReturnScope() = default;
	};

	template<typename Delayed>
	struct StatementDelayed : InstructionBase
	{
		virtual ~StatementDelayed() = default;

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<StatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		StatementDelayed(const Expr expr) : m_expr(expr) {}
		Expr m_expr;
	};
	using Statement = StatementDelayed<Dummy>;

	struct FunctionArgScope : Scope
	{
		void push_instruction(const InstructionIndex i) override {
			const Expr expr = safe_static_cast<Statement*>(retrieve_instruction(i))->m_expr;
			assert(expr);
			auto ctor = safe_static_cast<ConstructorBase*>(retrieve_expr(expr));
			ctor->m_flags = CtorFlags::FunctionArgument;
			m_instructions.push_back(i);
		}
	};

	template<typename Delayed>
	struct ReturnStatementDelayed final : Statement 
	{
		ReturnStatementDelayed() : Statement(Expr()) {}

		template<typename T>
		ReturnStatementDelayed(T&& t) : Statement(get_expr(std::forward<T>(t))) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ReturnStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};
	using ReturnStatement = ReturnStatementDelayed<Dummy>;

	template<typename Delayed>
	struct ForArgStatementDelayed final : Statement
	{
		ForArgStatementDelayed(const Expr expr) : Statement(expr) { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForArgStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};
	using ForArgStatement = ForArgStatementDelayed<Dummy>;

	template<typename Delayed>
	struct ForIterationStatementDelayed final : Statement
	{
		ForIterationStatementDelayed(const Expr& expr) : Statement(expr) { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForIterationStatementDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};
	using ForIterationStatement = ForIterationStatementDelayed<Dummy>;

	template<typename T>
	struct SpecialStatement final : InstructionBase
	{
		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SpecialStatement>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	struct Continue {};

	struct Discard {};

	struct Break {};

	struct EmitVertexI {};

	struct EndPrimitiveI {};

	template<typename ...Qs>
	struct ShaderStageOption {};

	struct FuncOverload
	{
		FuncOverload() {
			args = std::make_unique<FunctionArgScope>();
			body = std::make_unique<Scope>();
		}
		std::unique_ptr<Scope> args, body;
	};

	struct FuncDeclarationBase : InstructionBase
	{
		virtual ~FuncDeclarationBase() = default;

		FuncDeclarationBase(const std::string& name, const std::size_t fun_id) : m_name(name), m_id(fun_id) {
		}

		virtual void print_imgui(ImGuiData& data) const override {}
		virtual void print_glsl(GLSLData& data) const override {}

		virtual const FuncOverload& get_overload(const std::size_t i) const = 0;
		virtual std::size_t overload_count() const = 0;

		std::string m_name;
		std::size_t m_id;
	};

	template<typename ReturnTList, typename ... Fs>
	struct FuncDeclaration final : FuncDeclarationBase
	{
		static constexpr std::size_t N = sizeof...(Fs);

		FuncDeclaration(const std::string& name, const std::size_t fun_id) : FuncDeclarationBase(name, fun_id) {
		}

		const FuncOverload& get_overload(const std::size_t i) const override {
			return m_overloads[i];
		}

		virtual std::size_t overload_count() const override {
			return N;
		}

		void print_imgui(ImGuiData& data) const override {
			InstructionImGui<FuncDeclaration>::call(*this, data);
		}
		void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		std::array<FuncOverload, N> m_overloads;
	};

	struct ForArgsScope : Scope
	{
		using Scope::Scope;

		void push_instruction(const InstructionIndex i) override {
			const Expr expr = safe_static_cast<Statement*>(retrieve_instruction(i))->m_expr;
			m_instructions.push_back(make_instruction<ForArgStatement>(expr));
		}

		Expr m_stacked_condition;
	};

	template<typename Delayed>
	struct ForInstructionDelayed final : InstructionBase
	{
		ForInstructionDelayed() {
			args = std::make_unique<ForArgsScope>();
			body = std::make_unique<Scope>();
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<ForInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		std::unique_ptr<ForArgsScope> args;
		std::unique_ptr<Scope> body;
	};
	using ForInstruction = ForInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct IfInstructionDelayed final : InstructionBase
	{
		struct IfCase
		{
			Expr condition;
			std::unique_ptr<Scope> body;
		};

		IfInstructionDelayed(const InstructionIndex parent_if) : m_parent_if(parent_if) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<IfInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		std::vector<IfCase> m_cases;
		InstructionIndex m_parent_if;
		bool waiting_for_else = false;
	};
	using IfInstruction = IfInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct WhileInstructionDelayed final : InstructionBase
	{
		WhileInstructionDelayed(const Expr expr, Scope* parent_block) {
			m_condition = expr;
			m_body = std::make_unique<Scope>(parent_block);
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<WhileInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_condition;
		std::unique_ptr<Scope> m_body;
	};
	using WhileInstruction = WhileInstructionDelayed<Dummy>;

	template<typename Delayed>
	struct SwitchCaseDelayed final : InstructionBase
	{
		SwitchCaseDelayed(const Expr& expr, Scope* parent) {
			m_label = expr;
			m_body = std::make_unique<Scope>(parent);
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SwitchCaseDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_label;
		std::unique_ptr<Scope> m_body;
	};
	using SwitchCase = SwitchCaseDelayed<Dummy>;

	template<typename Delayed>
	struct SwitchInstructionDelayed final : InstructionBase
	{
		SwitchInstructionDelayed(const Expr expr, Scope* parent, const InstructionIndex parent_switch) {
			m_condition = expr;
			m_body = std::make_unique<Scope>(parent);
			m_parent_switch = parent_switch;
		}

		void add_case(const Expr expr, Scope*& current_block) {
			m_current_case = make_instruction<SwitchCase>(expr, m_body.get());
			m_body->push_instruction(m_current_case);
			current_block = safe_static_cast<SwitchCase*>(retrieve_instruction(m_current_case))->m_body.get();
		}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<SwitchInstructionDelayed>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		Expr m_condition;
		std::unique_ptr<Scope> m_body;
		InstructionIndex m_current_case;
		InstructionIndex m_parent_switch;
	};
	using SwitchInstruction = SwitchInstructionDelayed<Dummy>;

	template<typename Struct>
	struct StructDeclaration final : InstructionBase
	{
		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<StructDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}
	};

	template<typename Interface, typename Dimensions, typename Qualifiers>
	struct NamedInterfaceDeclaration final : InstructionBase
	{
		NamedInterfaceDeclaration(const std::string& name) : m_name(name) {}

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<NamedInterfaceDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		const std::string m_name;
	};

	template<typename QualifierList, typename TypeList>
	struct UnnamedInterfaceDeclaration final : InstructionBase
	{
		template<typename ...Strings>
		UnnamedInterfaceDeclaration(const std::string& name, Strings&& ...names) : m_name{ name }, m_names{ names ... } { }

		virtual void print_imgui(ImGuiData& data) const override {
			InstructionImGui<UnnamedInterfaceDeclaration>::call(*this, data);
		}
		virtual void print_glsl(GLSLData& data) const override {
			to_glsl(*this, data);
		}

		const std::string m_name;
		const std::array<std::string, TypeList::Size> m_names;
	};
}

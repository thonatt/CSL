#pragma once

#include "InstructionTree.hpp"
#include "Functions.hpp"
#include "TemplateHelpers.hpp"

#include <cassert>

#include <map>

namespace csl
{
	struct ShaderController;

	namespace context
	{
		inline ShaderController* g_current_shader = nullptr;
		inline bool g_context_active = true;

		inline bool active()
		{
			return g_context_active;
		}

		inline bool shader_active()
		{
			return g_current_shader;
		}

		inline ShaderController& get()
		{
			assert(g_current_shader);
			return *g_current_shader;
		}
	}

	struct ControllerBase
	{
		void queue_expr(const Expr e) {
			current_block->push_instruction(make_instruction<Statement>(e));
		}

		Scope* current_block;

		virtual void push_instruction(const InstructionIndex i) {}

		virtual ~ControllerBase() = default;
	};

	struct FunctionController : virtual ControllerBase
	{
		FuncDeclarationBase& get_current_func()
		{
			return safe_static_cast<FuncDeclarationBase&>(*retrieve_instruction(current_func));
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func_internal(const std::string& name, const std::size_t fun_id)
		{
			current_func = make_instruction<FuncDeclaration<ReturnTList, Fs...>>(name, fun_id);
			current_func_overloads_num_args = { GetArgTList<Fs>::Size ... };
			current_func_parent = current_block;
			current_overload = 0;
			first_overload = true;
			next_overload();
		}

		void check_num_args()
		{
			if (current_func && current_overload_arg_counter == current_func_overloads_num_args[current_overload]) {
				current_block = get_current_func().get_overload(current_overload).body.get();
				feeding_args = false;
			}
		}

		void check_func_args()
		{
			if (current_func && feeding_args) {
				check_num_args();
				++current_overload_arg_counter;
			}
		}

		void next_overload()
		{
			if (!current_func)
				return;

			if (first_overload)
				first_overload = false;
			else
				++current_overload;

			current_overload_arg_counter = 0;
			feeding_args = true;
			if (current_overload < get_current_func().overload_count()) {
				current_block = get_current_func().get_overload(current_overload).args.get();
				check_num_args();
			}
		}

		void end_func()
		{
			current_block = current_func_parent;
			current_func_parent = {};
			current_func = {};
		}

		std::vector<std::size_t> current_func_overloads_num_args;
		Scope* current_func_parent;
		InstructionIndex current_func;
		std::size_t current_overload;
		std::size_t current_overload_arg_counter;
		bool feeding_args = true;
		bool first_overload = true;
	};

	struct ForController : virtual ControllerBase {

		ForInstruction& get()
		{
			return safe_static_cast<ForInstruction&>(*retrieve_instruction(current_for));
		}

		void begin_for()
		{
			if (!context::active())
				return;
			current_for = make_instruction<ForInstruction>();
			push_instruction(current_for);
		}

		void begin_for_args()
		{
			if (!context::active())
				return;
			get().body->m_parent = current_block;
			current_block = get().args.get();
		}

		void begin_for_body()
		{
			if (!context::active())
				return;
			current_block = get().body.get();
		}

		void end_for()
		{
			if (!context::active())
				return;
			current_block = current_block->m_parent;
		}

		void stacking_for_condition(const Expr& expr)
		{
			if (current_for)
				get().m_stacked_condition = expr;
		}

		InstructionIndex current_for;
	};


	struct IfController : virtual ControllerBase
	{
		IfInstruction& get_current_if()
		{
			return safe_static_cast<IfInstruction&>(*retrieve_instruction(current_if));
		}

		void begin_if(const Expr& expr)
		{
			current_if = make_instruction<IfInstruction>(current_if);
			IfInstruction::IfCase if_case{ expr, std::make_unique<Scope>(current_block) };
			Scope* future_current_block = if_case.body.get();
			get_current_if().m_cases.push_back(std::move(if_case));
			push_instruction(current_if);
			current_block = future_current_block;
		}

		void begin_else()
		{
			begin_else_if({});
			delay_end_if();
		}

		void begin_else_if(const Expr& expr)
		{
			if (!context::active())
				return;
			IfInstruction::IfCase if_case{ expr, std::make_unique<Scope>(current_block->m_parent) };
			current_block = if_case.body.get();
			get_current_if().m_cases.push_back(std::move(if_case));
		}

		void end_if_sub_block()
		{
			if (!context::active())
				return;
			if (get_current_if().waiting_for_else) {
				end_if();
				end_if_sub_block();
			} else
				get_current_if().waiting_for_else = true;
		}
		void end_if()
		{
			if (!context::active())
				return;
			current_if = get_current_if().m_parent_if;
			current_block = current_block->m_parent;
		}

		void delay_end_if()
		{
			if (!context::active())
				return;
			if (current_if)
				get_current_if().waiting_for_else = false;
		}

		void check_end_if() {
			if (!context::active())
				return;
			if (current_if && get_current_if().waiting_for_else)
				end_if();
		}

		InstructionIndex current_if;
	};

	struct WhileController : virtual ControllerBase
	{
		void begin_while(const Expr& expr) {
			auto current_while = make_instruction<WhileInstruction>(expr, current_block);
			push_instruction(current_while);
			current_block = safe_static_cast<WhileInstruction*>(retrieve_instruction(current_while))->m_body.get();
		}

		void end_while() {
			if (!context::active())
				return;
			current_block = current_block->m_parent;
		}
	};

	struct SwitchController : virtual ControllerBase
	{
		SwitchInstruction& get()
		{
			return safe_static_cast<SwitchInstruction&>(*retrieve_instruction(current_switch));
		}

		void begin_switch(const Expr& expr)
		{
			current_switch = make_instruction<SwitchInstruction>(expr, current_block, current_switch);
			push_instruction(current_switch);
			current_block = get().m_body.get();
		}

		void add_case(const Expr& expr)
		{
			if (current_switch)
				get().add_case(expr, current_block);
		}

		void end_switch()
		{
			if (!context::active())
				return;
			if (current_switch) {
				if (get().m_current_case)
					current_block = current_block->m_parent;

				current_block = current_block->m_parent;
				current_switch = get().m_parent_switch;
			}
		}

		InstructionIndex current_switch;
	};

	struct MainController :
		virtual FunctionController,
		virtual ForController,
		virtual WhileController,
		virtual IfController,
		virtual SwitchController
	{
		void begin_for()
		{
			if (!context::active())
				return;
			check_end_if();
			check_func_args();
			ForController::begin_for();
		}

		void end_for()
		{
			if (!context::active())
				return;
			check_end_if();
			ForController::end_for();
		}

		template<typename T>
		void begin_if(T&& t)
		{
			if (!context::active())
				return;
			IfController::begin_if(get_expr(std::forward<T>(t)));
		}

		template<typename T>
		void begin_else_if(T&& t)
		{
			if (!context::active())
				return;
			IfController::begin_else_if(get_expr(std::forward<T>(t)));
		}

		template<typename T>
		void begin_while(T&& t)
		{
			if (!context::active())
				return;
			check_func_args();
			WhileController::begin_while(get_expr(std::forward<T>(t)));
		}

		void end_while()
		{
			if (!context::active())
				return;
			check_end_if();
			WhileController::end_while();
		}

		template<typename T>
		void begin_switch(T&& t)
		{
			if (!context::active())
				return;
			check_end_if();
			check_func_args();
			SwitchController::begin_switch(get_expr(std::forward<T>(t)));
		}

		template<typename T>
		void add_case(T&& t)
		{
			if (!context::active())
				return;
			check_end_if();
			SwitchController::add_case(get_expr(std::forward<T>(t)));
		}

		void add_case()
		{
			if (!context::active())
				return;
			check_end_if();
			SwitchController::add_case({});
		}

		void end_switch()
		{
			if (!context::active())
				return;
			check_end_if();
			SwitchController::end_switch();
		}

		void end_func()
		{
			check_end_if();
			FunctionController::end_func();
		}

		void push_expression(const Expr expr)
		{
			check_end_if();
			check_func_args();
			queue_expr(expr);
		}

		virtual void push_instruction(const InstructionIndex i) override
		{
			check_end_if();
			check_func_args();
			current_block->push_instruction(i);
		}
	};

	struct ShaderController : MainController
	{
		std::unique_ptr<Scope> m_scope;

#ifdef NDEBUG
		using MemoryPool = PolymorphicMemoryPool<OperatorBase>;
#else
		using MemoryPool = PolymorphicMemoryPoolDebug<OperatorBase>;
#endif

		MemoryPool m_memory_pool;

#ifdef NDEBUG
		using InstructionPool = PolymorphicMemoryPool<InstructionBase>;
#else
		using InstructionPool = PolymorphicMemoryPoolDebug<InstructionBase>;
#endif

		InstructionPool m_instruction_pool;

		static MemoryPool& get_static_memory()
		{
			static MemoryPool static_memory;
			return static_memory;
		}

		ShaderController()
		{
			m_scope = std::make_unique<Scope>();
			current_block = m_scope.get();
		}

		virtual ~ShaderController() = default;

		ShaderController(ShaderController&& other) :
			m_scope(std::move(other.m_scope)),
			m_memory_pool(std::move(other.m_memory_pool)),
			m_instruction_pool(std::move(other.m_instruction_pool))
		{
			if (context::g_current_shader == &other)
				context::g_current_shader = this;
		}

		ShaderController& operator=(ShaderController&& other)
		{
			std::swap(m_scope, other.m_scope);
			std::swap(m_memory_pool, other.m_memory_pool);
			std::swap(m_instruction_pool, other.m_instruction_pool);
			if (context::g_current_shader == &other)
				context::g_current_shader = this;

			return *this;
		}

		static void set_current_shader(ShaderController* shader);
		static ShaderController* get_current_shader();

		template<typename F>
		void main(F&& f)
		{
			static_assert(std::is_same_v<typename LambdaInfos<F>::RType, void>, "main function must returns void");
			(void)define_function<void>("main", f);
		}

		virtual void print_imgui(ImGuiData& data)
		{
			auto previous_current_shader = get_current_shader();
			set_current_shader(this);
			for (const auto& i : m_scope->m_instructions)
				retrieve_instruction(i)->print_imgui(data);
			set_current_shader(previous_current_shader);
		}

		virtual void print_glsl(GLSLData& data)
		{
			auto previous_current_shader = get_current_shader();
			set_current_shader(this);
			for (const auto& i : m_scope->m_instructions)
				retrieve_instruction(i)->print_glsl(data);
			set_current_shader(previous_current_shader);
		}

		template<typename Struct>
		void add_struct()
		{
			m_scope->m_instructions.push_back(make_instruction<StructDeclaration<Struct>>());
		}

		template<typename Interface, typename Dimensions, typename Qualifiers>
		void add_named_interface_block(const std::string& name)
		{
			m_scope->m_instructions.push_back(make_instruction<NamedInterfaceDeclaration<Interface, Dimensions, Qualifiers>>(name));
		}

		template<typename QualifierList, typename TypeList, typename ...Strings>
		void add_unnamed_interface_block(const std::string& name, Strings&& ... names)
		{
			m_scope->m_instructions.push_back(make_instruction<UnnamedInterfaceDeclaration<QualifierList, TypeList>>(name, std::forward<Strings>(names)...));
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string& name, const std::size_t fun_id, const Fs& ... fs)
		{
			begin_func_internal<ReturnTList, Fs...>(name, fun_id);
			m_scope->m_instructions.push_back(current_func);
		}

		ReturnScopeBase* get_return_block()
		{
			Scope* test_block = current_block;
			while (test_block) {
				if (dynamic_cast<ReturnScopeBase*>(test_block))
					break;
				else
					test_block = test_block->m_parent;
			}
			return safe_static_cast<ReturnScopeBase*>(test_block);
		}

		template<typename S, typename ... Args>
		void add_statement(Args&& ... args)
		{
			if (context::active())
				push_instruction(make_instruction<S>(std::forward<Args>(args)...));
		}
	};
}

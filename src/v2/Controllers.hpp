#pragma once

#include "InstructionTree.hpp"
#include <cassert>

#include <map>

namespace v2 {

	template<typename T>
	struct ControllerDebug {
		static void call(const T& t, DebugData& data) { }
	};

	template<typename T>
	struct ControllerImGui {
		static void call(const T& t, ImGuiData& data) { }
	};

	template<typename T>
	struct ControllerGLSL {
		static void call(const T& t, GLSLData& data) { }
	};

	struct ControllerBase {

		void queue_expr(const Expr& e) {
			current_block->push_instruction(make_statement(e));
		}

		Block::Ptr current_block;

		virtual void push_instruction(const InstructionBase::Ptr& i) {}

		virtual ~ControllerBase() = default;

	};

	struct FunctionController : virtual ControllerBase {

		template<typename ReturnTList, typename ... Fs>
		void begin_func_internal(const std::string& name, const std::size_t fun_id) {
			current_func = std::make_shared<FuncDeclaration<ReturnTList, Fs...>>(name, fun_id);
			current_func_overloads_num_args = { GetArgTList<Fs>::Size ... };
			current_func_parent = current_block;
			current_overload = 0;
			first_overload = true;
			next_overload();
		}

		void check_num_args() {
			if (current_func && current_overload_arg_counter == current_func_overloads_num_args[current_overload]) {
				current_block = current_func->get_overload(current_overload).body;
				feeding_args = false;
			}
		}

		void check_func_args() {
			if (current_func && feeding_args) {
				check_num_args();
				++current_overload_arg_counter;
			}
		}

		void next_overload() {
			if (!current_func) {
				return;
			}

			if (first_overload) {
				first_overload = false;
			} else {
				++current_overload;
			}

			current_overload_arg_counter = 0;
			feeding_args = true;
			if (current_overload < current_func->overload_count()) {
				current_block = current_func->get_overload(current_overload).args;
				check_num_args();
			}
		}

		virtual void end_func() {
			current_block = current_func_parent;
			current_func_parent = {};
			current_func = {};
		}

		FuncDeclarationBase::Ptr current_func;
		Block::Ptr current_func_parent;
		std::vector<std::size_t> current_func_overloads_num_args;
		std::size_t current_overload;
		std::size_t current_overload_arg_counter;
		bool feeding_args = true;
		bool first_overload = true;
	};

	struct ForController : virtual ControllerBase {

		ForInstruction& get() {
			return current_for->m_instruction;
		}

		virtual void begin_for() {
			current_for = std::make_shared<InstructionWrapper<ForInstruction>>();
			push_instruction(current_for);
		}

		void begin_for_args() {
			get().body->m_parent = current_block;
			current_block = get().args;
		}

		void begin_for_body() {
			current_block = get().body;
		}

		virtual void end_for() {
			current_block = current_block->m_parent;
		}

		void stacking_for_condition(const Expr& expr) {
			if (current_for) {
				get().args->m_stacked_condition = expr;
			}
		}

		InstructionWrapper<ForInstruction>::Ptr current_for;
	};


	struct IfController : virtual ControllerBase {

		IfInstruction& get() {
			return current_if->m_instruction;
		}

		void begin_if(const Expr& expr) {
			current_if = std::make_shared<InstructionWrapper<IfInstruction>>(current_if);
			IfInstruction::IfCase if_case{ expr, std::make_shared<Block>(current_block) };
			get().m_cases.push_back(if_case);
			push_instruction(current_if);
			current_block = if_case.body;
		}

		void begin_else() {
			IfInstruction::IfCase if_case{ {}, std::make_shared<Block>(current_block->m_parent) };
			get().m_cases.push_back(if_case);
			current_block = if_case.body;
			delay_end_if();
		}

		void begin_else_if(const Expr& expr) {
			IfInstruction::IfCase if_case{ expr, std::make_shared<Block>(current_block->m_parent) };
			get().m_cases.push_back(if_case);
			current_block = if_case.body;
		}

		void end_if_sub_block() {
			if (get().waiting_for_else) {
				end_if();
				end_if_sub_block();
			} else {
				get().waiting_for_else = true;
			}

		}
		void end_if() {
			current_if = get().m_parent_if;
			current_block = current_block->m_parent;
		}
		void delay_end_if() {
			if (current_if) {
				get().waiting_for_else = false;
			}
		}
		void check_begin_if() {
			if (current_if && get().waiting_for_else) {
				end_if();
			}
		}
		void check_end_if() {
			if (current_if && get().waiting_for_else) {
				end_if();
			}
		}

		InstructionWrapper<IfInstruction>::Ptr current_if;
	};

	struct WhileController : virtual ControllerBase {

		virtual void begin_while(const Expr& expr) {
			auto current_while = std::make_shared<InstructionWrapper<WhileInstruction>>(expr, current_block);
			push_instruction(current_while);
			current_block = current_while->m_instruction.m_body;
		}

		virtual void end_while() {
			current_block = current_block->m_parent;
		}
	};

	struct SwitchController : virtual ControllerBase {

		SwitchInstruction& get()
		{
			return current_switch->m_instruction;
		}

		virtual void begin_switch(const Expr& expr) {
			current_switch = std::make_shared<InstructionWrapper<SwitchInstruction>>(expr, current_block, current_switch);
			push_instruction(current_switch);
			current_block = get().m_body;
		}

		virtual void add_case(const Expr& expr) {
			if (current_switch) {
				get().add_case(expr, current_block);
			}
		}

		virtual void end_switch() {
			if (current_switch) {
				if (get().m_current_case) {
					current_block = current_block->m_parent;
				}
				current_block = current_block->m_parent;
				current_switch = get().m_parent_switch;
			}
		}

		SwitchInstruction::Ptr current_switch;
	};

	struct MainController :
		virtual FunctionController,
		virtual ForController,
		virtual WhileController,
		virtual IfController,
		virtual SwitchController
	{
		using Ptr = std::shared_ptr<MainController>;

		virtual void begin_for() override {
			check_end_if();
			check_func_args();
			ForController::begin_for();
		}

		virtual void end_for() override {
			check_end_if();
			ForController::end_for();
		}

		virtual void begin_while(const Expr& expr) override {
			check_func_args();
			WhileController::begin_while(expr);
		}

		virtual void end_while() override {
			check_end_if();
			WhileController::end_while();
		}

		virtual void begin_switch(const Expr& expr) override {
			check_end_if();
			check_func_args();
			SwitchController::begin_switch(expr);
		}

		virtual void add_case(const Expr& expr) override {
			check_end_if();
			SwitchController::add_case(expr);
		}

		virtual void end_switch() override {
			check_end_if();
			SwitchController::end_switch();
		}

		virtual void end_func() override {
			check_end_if();
			FunctionController::end_func();
		}

		void push_expression(const Expr& expr) {
			check_end_if();
			check_func_args();
			queue_expr(expr);
		}

		virtual void push_instruction(const InstructionBase::Ptr& i) override {
			check_end_if();
			check_func_args();
			current_block->push_instruction(i);
		}
	};

	struct ShaderController : MainController {
		using Ptr = std::shared_ptr<ShaderController>;

		MainBlock::Ptr m_declarations;
		std::vector<InstructionBase::Ptr> m_structs;
		std::vector<InstructionBase::Ptr> m_unnamed_interface_blocks;
		std::vector<InstructionBase::Ptr> m_functions;

		using BiggestType = Constructor<Matrix<float, 1, 1>, 4>;
		using PolyVector = PolymorphicVector<OperatorBase, sizeof(BiggestType), alignof(BiggestType)>;
		//std::shared_ptr<PolyVector> m_exprs;

		using Memory = PolymorphicMemoryManager<OperatorBase, 8, sizeof(BiggestType), alignof(BiggestType)>;

		//std::shared_ptr<Memory> m_memory;
		//std::map<std::size_t, std::size_t> m_expr_allocations;

		using MemoryPool = PolymorphicMemoryPool<OperatorBase>;
		std::shared_ptr<MemoryPool> m_memory_pool;

		//static Memory& get_static_memory() {
		//	static Memory static_memory;
		//	return static_memory;
		//}

		static MemoryPool& get_static_memory() {
			static MemoryPool static_memory;
			return static_memory;
		}

		ShaderController() {
			m_declarations = std::make_shared<MainBlock>();
			current_block = m_declarations;
			//m_exprs = std::make_shared<PolyVector>();
			//m_memory = std::make_shared<Memory>();
			m_memory_pool = std::make_shared<MemoryPool>();

			m_memory_pool->m_buffer.reserve(10000);
			m_memory_pool->m_objects_ids.reserve(100);
			//for (std::size_t i = 0; i < m_memory->m_buffers.size(); ++i) {
			//	m_memory->m_buffers[i].reserve(10 * (i + 1) * 8);
			//}
		}

		template<typename F>
		void main(F&& f) {
			static_assert(std::is_same_v<typename LambdaInfos<F>::RType, void>);
			(void)define_function<void>("main", f);
		}

		// template to delay instantiation
		template<typename Data>
		void print_debug(Data& data) const {
			ControllerDebug<ShaderController>::call(*this, data);
		}

		// template to delay instantiation
		template<typename Data>
		void print_imgui(Data& data) const {
			ControllerImGui<ShaderController>::call(*this, data);
		}

		// template to delay instantiation
		template<typename Data>
		void print_glsl(Data& data) const {
			ControllerGLSL<ShaderController>::call(*this, data);
		}

		template<typename Struct>
		void add_struct() {
			m_structs.push_back(make_instruction<StructDeclarationWrapper>(StructDeclarationWrapper::create<Struct>()));
		}

		template<typename Interface>
		void add_unnamed_interface_block() {
			m_unnamed_interface_blocks.push_back(std::static_pointer_cast<InstructionBase>(std::make_shared<UnnamedInterfaceDeclaration<Interface>>()));
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string& name, const std::size_t fun_id, const Fs& ... fs) {
			begin_func_internal<ReturnTList, Fs...>(name, fun_id);
			m_functions.push_back(make_instruction<FuncDeclarationWrapper>(current_func));
		}

		ReturnBlockBase::Ptr get_return_block() {
			Block::Ptr test_block = current_block;
			bool found_return_block = false;
			while (!found_return_block) {
				if (std::dynamic_pointer_cast<ReturnBlockBase>(test_block)) {
					found_return_block = true;
				} else if (test_block->m_parent) {
					test_block = test_block->m_parent;
				} else {
					break;
				}
			}

			assert(found_return_block);
			return std::dynamic_pointer_cast<ReturnBlockBase>(test_block);
		}

		template<typename S, typename ... Args>
		void add_statement(Args&& ... args) {
			push_instruction(make_instruction<S>(std::forward<Args>(args)...));
		}
	};
}

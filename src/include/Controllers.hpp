#pragma once

#include "InstructionTree.hpp"
#include "Functions.hpp"

#include <cassert>

#include <map>

namespace v2 {

	template<typename Delayed, typename T>
	struct ControllerImGui {

		template<typename Data>
		static void call(const T& t, Data& data) { }
	};

	template<typename Delayed, typename T>
	struct ControllerGLSL {

		template<typename Data>
		static void call(const T& t, Data& data) { }
	};

	struct ControllerBase {

		void queue_expr(const Expr e) {
			current_block->push_instruction(make_instruction<Statement>(e));
		}

		Block::Ptr current_block;

		virtual void push_instruction(const InstructionIndex i) {}

		virtual ~ControllerBase() = default;

	};

	struct FunctionController : virtual ControllerBase {

		FuncDeclarationBase& get_current_func()
		{
			return *reinterpret_cast<FuncDeclarationBase*>(retrieve_instruction(current_func));
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func_internal(const std::string& name, const std::size_t fun_id) {
			current_func = make_instruction<FuncDeclaration<ReturnTList, Fs...>>(name, fun_id);
			current_func_overloads_num_args = { GetArgTList<Fs>::Size ... };
			current_func_parent = current_block;
			current_overload = 0;
			first_overload = true;
			next_overload();
		}

		void check_num_args() {
			if (current_func && current_overload_arg_counter == current_func_overloads_num_args[current_overload]) {
				current_block = get_current_func().get_overload(current_overload).body;
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
			if (current_overload < get_current_func().overload_count()) {
				current_block = get_current_func().get_overload(current_overload).args;
				check_num_args();
			}
		}

		virtual void end_func() {
			current_block = current_func_parent;
			current_func_parent = {};
			current_func = {};
		}

		std::vector<std::size_t> current_func_overloads_num_args;
		Block::Ptr current_func_parent;
		InstructionIndex current_func;
		std::size_t current_overload;
		std::size_t current_overload_arg_counter;
		bool feeding_args = true;
		bool first_overload = true;
	};

	struct ForController : virtual ControllerBase {

		ForInstruction& get() {
			return *reinterpret_cast<ForInstruction*>(retrieve_instruction(current_for));
		}

		virtual void begin_for() {
			current_for = make_instruction<ForInstruction>();
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

		InstructionIndex current_for;
	};


	struct IfController : virtual ControllerBase {

		IfInstruction& get() {
			return *reinterpret_cast<IfInstruction*>(retrieve_instruction(current_if));
		}

		void begin_if(const Expr& expr) {
			current_if = make_instruction<IfInstruction>(current_if);
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

		InstructionIndex current_if;
	};

	struct WhileController : virtual ControllerBase {

		virtual void begin_while(const Expr& expr) {
			auto current_while = make_instruction<WhileInstruction>(expr, current_block);
			push_instruction(current_while);
			current_block = reinterpret_cast<WhileInstruction*>(retrieve_instruction(current_while))->m_body;
		}

		virtual void end_while() {
			current_block = current_block->m_parent;
		}
	};

	struct SwitchController : virtual ControllerBase {

		SwitchInstruction& get()
		{
			return *reinterpret_cast<SwitchInstruction*>(retrieve_instruction(current_switch));
		}

		virtual void begin_switch(const Expr& expr) {
			current_switch = make_instruction<SwitchInstruction>(expr, current_block, current_switch);
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

		InstructionIndex current_switch;
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

		void push_expression(const Expr expr) {
			check_end_if();
			check_func_args();
			queue_expr(expr);
		}

		virtual void push_instruction(const InstructionIndex i) override {
			check_end_if();
			check_func_args();
			current_block->push_instruction(i);
		}
	};

	struct ShaderController : MainController {
		using Ptr = std::shared_ptr<ShaderController>;

		MainBlock::Ptr m_declarations;
		std::vector<InstructionIndex> m_structs;
		std::vector<InstructionIndex> m_named_interface_blocks;
		std::vector<InstructionIndex> m_unnamed_interface_blocks;
		std::vector<InstructionIndex> m_functions;

		using BiggestType = Constructor<Matrix<float, 1, 1>, 4>;
		using PolyVector = PolymorphicVector<OperatorBase, sizeof(BiggestType), alignof(BiggestType)>;
		//std::shared_ptr<PolyVector> m_exprs;

		using Memory = PolymorphicMemoryManager<OperatorBase, 8, sizeof(BiggestType), alignof(BiggestType)>;

		//std::shared_ptr<Memory> m_memory;
		//std::map<std::size_t, std::size_t> m_expr_allocations;

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
			//m_memory_pool = std::make_shared<MemoryPool>();

			//m_memory_pool.m_buffer.reserve(10000);
			//m_memory_pool.m_objects_ids.reserve(100);

			//m_instruction_pool.m_buffer.reserve(5000);
			//m_instruction_pool.m_objects_ids.reserve(100);

			//for (std::size_t i = 0; i < m_memory->m_buffers.size(); ++i) {
			//	m_memory->m_buffers[i].reserve(10 * (i + 1) * 8);
			//}
		}

		ShaderController(ShaderController&& other) :
			m_declarations(std::move(other.m_declarations)),
			m_structs(std::move(other.m_structs)),
			m_named_interface_blocks(std::move(other.m_named_interface_blocks)),
			m_unnamed_interface_blocks(std::move(other.m_unnamed_interface_blocks)),
			m_functions(std::move(other.m_functions)),
			m_memory_pool(std::move(other.m_memory_pool)),
			m_instruction_pool(std::move(other.m_instruction_pool))
		{
			//std::cout << "ShaderController(ShaderController&&)" << std::endl;
		}

		ShaderController& operator=(ShaderController&& other) {
			std::swap(m_declarations, other.m_declarations);
			std::swap(m_structs, other.m_structs);
			std::swap(m_named_interface_blocks, other.m_named_interface_blocks);
			std::swap(m_unnamed_interface_blocks, other.m_unnamed_interface_blocks);
			std::swap(m_functions, other.m_functions);
			std::swap(m_memory_pool, other.m_memory_pool);
			std::swap(m_instruction_pool, other.m_instruction_pool);
			return *this;
		}

		static void set_current_shader(ShaderController* shader);
		static ShaderController* get_current_shader();

		template<typename F>
		void main(F&& f) {
			static_assert(std::is_same_v<typename LambdaInfos<F>::RType, void>, "main function must returns void");
			(void)define_function<void>("main", f);
		}

		// template to delay instantiation
		template<typename Delayed, typename Data>
		void print_imgui(Data& data) {
			auto previous_current_shader = get_current_shader();
			set_current_shader(this);
			ControllerImGui<Delayed, ShaderController>::call(*this, data);
			set_current_shader(previous_current_shader);
		}

		// template to delay instantiation
		template<typename Delayed, typename Data>
		void print_glsl(Data& data) {
			auto previous_current_shader = get_current_shader();
			set_current_shader(this);
			ControllerGLSL<Delayed, ShaderController>::call(*this, data);
			set_current_shader(previous_current_shader);
		}

		template<typename Struct>
		void add_struct() {
			m_structs.push_back(make_instruction<StructDeclaration<Struct>>());
		}

		template<typename Interface>
		void add_named_interface_block(const std::string& name) {
			m_named_interface_blocks.push_back(make_instruction<NamedInterfaceDeclaration<Interface>>(name));
		}

		template<typename QualifierList, typename TypeList, typename ...Strings>
		void add_unnamed_interface_block(Strings&& ... names) {
			m_unnamed_interface_blocks.push_back(make_instruction<UnnamedInterfaceDeclaration<QualifierList, TypeList>>(std::forward<Strings>(names)...));
		}

		template<typename ReturnTList, typename ... Fs>
		void begin_func(const std::string& name, const std::size_t fun_id, const Fs& ... fs) {
			begin_func_internal<ReturnTList, Fs...>(name, fun_id);
			m_functions.push_back(current_func);
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

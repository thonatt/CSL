#pragma once

#include <sstream>
#include <array>

#include "StringHelpers.hpp"

namespace csl {

	namespace core {

		inline stringPtr makeStringPtr(const string & s) {
			auto str_ptr = ::std::make_shared<string>(s);
			//std::cout << " str_cv : " <<  (int)(bool)str_ptr << " " << s << std::endl;
			return str_ptr;
		}

		enum NamedObjectFlags : uint {
			IS_USED = 1 << 1,
			IS_INIT = 1 << 2,
			IS_TRACKED = 1 << 3,
			ALWAYS_EXP = 1 << 4,
			IS_BASE = 1 << 5
		};

		enum CtorStatus { DECLARATION, INITIALISATION, TEMP, FORWARD };
		enum OperatorDisplayRule { NONE, IN_FRONT, IN_BETWEEN, IN_BETWEEN_NOSPACE, BEHIND };
		//enum ParenthesisRule { USE_PARENTHESIS, NO_PARENTHESIS };
		//enum CtorTypeDisplay { DISPLAY, HIDE};
		enum class CtorPosition { MAIN_BLOCK, INSIDE_BLOCK };

		enum OpFlags : uint {
			DISABLED = 1 << 1,
			DISPLAY_TYPE = 1 << 2,
			MULTIPLE_INITS = 1 << 3,
			PARENTHESIS = 1 << 4,
			MAIN_BLOCK = 1 << 5,
			MEMBER_DECLARATION = 1 << 6
		};

		enum OperatorPrecedence : uint {

			ALIAS = 10,
			INITIALIZER_LIST = 10,

			ARRAY_SUBSCRIPT = 20,
			FUNCTION_CALL = 20,
			FIELD_SELECTOR = 20,
			SWIZZLE = 20,
			POSTFIX = 20,

			PREFIX = 30,
			UNARY = 30,
			NEGATION = 30,
			ONES_COMPLEMENT = 30,

			DIVISION = 40,
			MULTIPLY = 42,
			MODULO = 41,

			ADDITION = 50,
			SUBSTRACTION = 50,

			BITWISE_SHIFT = 60,

			RELATIONAL = 70,

			EQUALITY = 80,

			BITWISE_AND = 90,

			BITWISE_XOR = 100,

			BITWISE_OR = 110,

			LOGICAL_AND = 120,

			LOGICAL_XOR = 130,

			LOGICAL_OR = 140,

			TERNARY = 150,

			ASSIGNMENT = 160,

			SEQUENCE = 170
		};

		struct OperatorBase;
		using Ex = std::shared_ptr<OperatorBase>;

		struct OperatorBase {

			OperatorBase(uint _flags = 0) : flags(_flags) {}
			virtual ~OperatorBase() = default;

			virtual string str(int trailing) const {
				return "no_str";
			}

			virtual uint rank() const {
				return 0;
			}

			const bool inversion(const Ex & other) const {
				return rank() < other->rank();
			}

			const bool equalRank(const Ex & other) const {
				return rank() == other->rank();
			}

			string checkForParenthesis(const Ex & exp) const {
				if (inversion(exp)) {
					return "(" + exp->str(0) + ")";
				}
				return exp->str(0);
			}

			void disable() {
				flags |= DISABLED;
			}

			bool disabled() const {
				return flags & DISABLED;
			}

			bool isMemberDeclaration() const {
				return flags & MEMBER_DECLARATION;
			}

			virtual Ex firstArg() { return Ex(); }

			uint flags = 0;
		};

		template<typename Operator, typename ... Args>
		Ex createExp(Args &&... args) {
			return std::static_pointer_cast<OperatorBase>(std::make_shared<Operator>(std::forward<Args>(args)...));
		}

		struct NamedOperator {
			NamedOperator(const string & str) : operator_str(str) {}
			const std::string& op_str() const { return operator_str; }

			string operator_str;
		};

		template<OperatorPrecedence precedence>
		struct Precedence : OperatorBase {
			virtual ~Precedence() = default;
			virtual uint rank() const { return (uint)precedence; }
		};

		struct Alias : Precedence<ALIAS> {
			Alias(const stringPtr& _obj_str_ptr) : obj_str_ptr(_obj_str_ptr) {}
			string str(int trailing) const override { return *obj_str_ptr; }

			stringPtr obj_str_ptr;
		};

		template<OperatorPrecedence precedence>
		struct MiddleOperator : Precedence<precedence>, NamedOperator {
			MiddleOperator(const string & op_str, const Ex& _lhs, const Ex& _rhs)
				: NamedOperator(op_str), lhs(_lhs), rhs(_rhs) {
			}

			string str(int trailing) const {
				return OperatorBase::checkForParenthesis(lhs) + NamedOperator::op_str() + OperatorBase::checkForParenthesis(rhs);
			}

			Ex lhs, rhs;
		};

		template<size_t N>
		struct ArgsCall {

			template<typename ... Args>
			ArgsCall(Args && ... _args) : args{ std::forward<Args>(_args)... } {}

			string args_str_body() const {
				string out = "";
				for (size_t i = 0; i < N; ++i) {
					out += args[i]->str(0) + ((i == N - 1) ? "" : ", ");
				}
				return out;
			}

			string args_str() const {
				return "(" + args_str_body() + ")";
			}

			std::array<Ex, N> args;
		};

		template<uint N>
		struct FunctionCall : Precedence<FUNCTION_CALL>, NamedOperator, ArgsCall<N> {

			template<typename ... Args>
			FunctionCall(const string & s, Args && ... _args)
				: NamedOperator(s), ArgsCall<N>(std::forward<Args>(_args)...) {
			}

			string str(int trailing) const {
				return op_str() + ArgsCall<N>::args_str();
			}
		};

		template<typename ... Args>
		Ex createFCallExp(const string & f_name, Args && ... args) {
			return createExp<FunctionCall<sizeof...(Args)>>(f_name, std::forward<Args>(args)...);
		}

		struct ConstructorBase : OperatorBase {
			ConstructorBase(CtorStatus _status = INITIALISATION, uint _flags = 0)
				: OperatorBase(_flags), ctor_status(_status) { }

			virtual ~ConstructorBase() = default;

			CtorPosition position() const {
				if (flags & MAIN_BLOCK) {
					return CtorPosition::MAIN_BLOCK;
				} else {
					return CtorPosition::INSIDE_BLOCK;
				}
			}

			void setTemp() {
				if (status() == INITIALISATION) { // || status() == DECLARATION || status() == FORWARD ) {
					ctor_status = TEMP;
				}
			}
			void setInit() {
				if (status() == FORWARD) {
					ctor_status = INITIALISATION;
				}
			}

			CtorStatus status() const {
				return ctor_status;
			}

			virtual bool is_bool_ctor() const {
				return false;
			}

			virtual string obj_name() const {
				return "base_ctor_obj_name";
			}

			virtual Ex firstArg() override { return Ex(); }

			CtorStatus ctor_status;
		};

		template<typename T, uint N = 0>
		struct Constructor : ArgsCall<N>, ConstructorBase {

			virtual uint rank() const override {
				if ((N == 0) || (flags & DISPLAY_TYPE)) {
					return FUNCTION_CALL;
				} else {
					return ArgsCall<N>::args[0]->rank();
				}
			}

			template<typename ... Args>
			Constructor(const stringPtr& _obj_name_ptr, CtorStatus _status, uint _flags, Args && ...  args)
				: ArgsCall<N>(std::forward<Args>(args)...), ConstructorBase(_status, _flags), obj_name_ptr(_obj_name_ptr)
			{
			}

			virtual ~Constructor() = default;

			virtual string obj_name() const override {
				return *obj_name_ptr;
			}

			string lhs_type_str(int trailing) const {
				if (flags & MULTIPLE_INITS) {
					return "";
				}
				return getTypeStr<T>(trailing);
			}

			virtual string rhs_type_str() const {
				return TypeStrRHS<T>::str();
			}

			virtual string lhs_str(int trailing) const {
				return DeclarationStr<T>::str(obj_name(), trailing);
			}

			string rhs_str() const {
				string str;
				if (flags & DISPLAY_TYPE) {
					str += rhs_type_str();
				}
				if (flags & PARENTHESIS) {
					str += ArgsCall<N>::args_str();
				} else {
					str += ArgsCall<N>::args_str_body();
				}
				return str;
			}

			virtual string str(int trailing) const override {
				if (status() == INITIALISATION) {
					return lhs_str(trailing) + " = " + rhs_str();
				} else if (status() == DECLARATION) {
					return lhs_str(trailing);
				} else if (status() == FORWARD) {
					return ArgsCall<N>::args[0]->str(0);
				} else {
					return rhs_str();
				}
			}

			virtual bool is_bool_ctor() const override {
				return is_same_v<T, Bool>;
			}

			virtual Ex firstArg() override {
				if (N > 0) {
					return ArgsCall<N>::args[0];
				} else {
					return Ex();
				}
			}

			stringPtr obj_name_ptr;
		};

		struct MemberAccessor : Precedence<FIELD_SELECTOR> {
			enum ObjStatus { NOT_TEMP = 0, TEMP = 1 };

			MemberAccessor(const Ex & _obj, const string & _member_str, ObjStatus _obj_is_temp = NOT_TEMP)
				: obj(_obj), member_str(_member_str), obj_is_temp(_obj_is_temp) {
			}

			string str(int trailing) const {
				string obj_str;
				if (obj_is_temp) {
					obj_str = OperatorBase::checkForParenthesis(obj);
				} else {
					if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(obj)) {
						if (auto accessor = std::dynamic_pointer_cast<MemberAccessor>(obj->firstArg())) {
							obj_str = OperatorBase::checkForParenthesis(accessor);
						} else {
							obj_str = ctor->obj_name();
						}
					} else {
						obj_str = obj->str(0);
					}
				}
				return obj_str + "." + member_str;
			}

			void make_obj_tmp() {
				if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(obj)) {
					ctor->setTemp();
					ctor->disable();
				}
				obj_is_temp = MemberAccessor::ObjStatus::TEMP;
			}

			Ex obj;
			string member_str;
			ObjStatus obj_is_temp;
		};

		template<uint N>
		struct MemberFunctionAccessor : FunctionCall<N> {
			template<typename ...Args>
			MemberFunctionAccessor(const Ex & _obj, const string & fun_name, Args && ... args)
				: FunctionCall<N>(fun_name, std::forward<Args>(args)...), obj(_obj) {
			}

			string str(int trailing) const {
				return OperatorBase::checkForParenthesis(obj) + "." + FunctionCall<N>::str(0);
			}

			Ex obj;
		};

		struct ArraySubscript : Precedence<ARRAY_SUBSCRIPT> {
			ArraySubscript(const Ex& _obj, const Ex& _arg) : obj(_obj), arg(_arg) {
			}

			string str(int trailing) const {
				return OperatorBase::checkForParenthesis(obj) + "[" + arg->str(0) + "]";
			}

			Ex obj, arg;
		};

		struct PrefixUnary : Precedence<PREFIX>, NamedOperator {
			PrefixUnary(const string & op_str, const Ex& _obj)
				: NamedOperator(op_str), obj(_obj) {
			}

			string str(int trailing) const {
				return op_str() + OperatorBase::checkForParenthesis(obj);
			}

			Ex obj;
		};

		struct PostfixUnary : Precedence<POSTFIX>, NamedOperator {
			PostfixUnary(const string & op_str, const Ex& _obj)
				: NamedOperator(op_str), obj(_obj) {
			}

			string str(int trailing) const {
				return OperatorBase::checkForParenthesis(obj) + op_str();
			}

			Ex obj;
		};

		struct Ternary : Precedence<TERNARY> {
			Ternary(const Ex& _condition, const Ex& _first, const Ex& _second)
				: condition(_condition), first(_first), second(_second) {
			}

			string str(int trailing) const {
				return OperatorBase::checkForParenthesis(condition) + " ? " + OperatorBase::checkForParenthesis(first) + " : " + OperatorBase::checkForParenthesis(second);
			}

			Ex condition, first, second;
		};

		template<uint N>
		struct InitializerList : Precedence<INITIALIZER_LIST>, ArgsCall<N>
		{
			template<typename ... Args>
			InitializerList(const Args & ... args) : ArgsCall<N>(args...)
			{
			}

			virtual string str(int trailing) const {
				return "{ " + ArgsCall<N>::args_str_body() + " }";
			}
		};

		struct EmptyExp : OperatorBase {
			static Ex get() { return std::static_pointer_cast<OperatorBase>(std::make_shared<EmptyExp>()); }

			string str(int trailing) const {
				return "";
			}
		};

		template<typename T>
		struct Litteral : OperatorBase {
			Litteral(T _i) : i(_i) {}

			virtual string str(int trailing) const {
				if (is_integral_v<T>) {
					return std::to_string(i);
				} else {
					std::stringstream ss;
					ss << std::fixed << i;
					string s = ss.str();
					s.erase(s.find_last_not_of('0') + 1, string::npos);
					if (s.back() == '.') {
						s += "0";
					}
					return s;
				}
			}

			T i;
		};


		template<> struct Litteral<bool> : OperatorBase {
			Litteral(const bool & _b) : b(_b) {}
			virtual string str(int trailing) const { return b ? "true" : "false"; }
			bool b;
		};

	}

} //namespace csl

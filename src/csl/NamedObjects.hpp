#pragma once

#include "Operators.hpp"

#define EX(type, var) getExp(std::forward<type>(var))

namespace csl {

	namespace core {

		///////////////////////////

		class NamedObjectBase {
		public:
			NamedObjectBase(const string & _name = "", ObjFlags obj_flags = ObjFlags::IS_USED | ObjFlags::IS_TRACKED)
				: flags(obj_flags)
			{
				namePtr = std::make_shared<string>(_name);
				//std::cout << "base " << *namePtr << " " << (int)(flags & IS_TRACKED) << std::endl;
			}

			bool isUsed() const {
				return flags & ObjFlags::IS_USED;
			}

			bool isTracked() const {
				return flags & ObjFlags::IS_TRACKED;
			}

			void setNotUsed() {
				flags &= ~(ObjFlags::IS_USED);
			}

			Ex alias() const {
				return createExp<Alias>(strPtr());
			}

			Ex getExRef()
			{
				return static_cast<const NamedObjectBase*>(this)->getExRef();
			}

			Ex getExRef() const
			{
				flags |= ObjFlags::IS_USED;
				if (flags & ObjFlags::ALWAYS_EXP) {
					return exp;
					//return getExTmp();
				}
				if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
					ctor->setInit();
				}
				return alias();
			}

			Ex getExTmp()
			{
				return static_cast<const NamedObjectBase*>(this)->getExTmp();
			}

			Ex getExTmp() const
			{
				flags |= ObjFlags::IS_USED;
				if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
					ctor->setTemp();
					ctor->disable();

					if (auto arg = ctor->firstArg()) {
						if (auto accessor = std::dynamic_pointer_cast<MemberAccessor>(arg)) {
							accessor->make_obj_tmp();
						}
					}
				}
				return exp;
			}

			Ex getEx() & { return getExRef(); }
			Ex getEx() const & { return getExRef(); }
			Ex getEx() && { return getExTmp(); }
			Ex getEx() const && { return getExTmp(); }

			void checkDisabling()
			{
				if (!(flags & ObjFlags::IS_TRACKED)) {
					//std::cout << "disabling " << str() << std::endl;
					exp->disable();
				}
			}

			~NamedObjectBase()
			{
				if (!isUsed()) {
					//std::cout << " ~ setTemp " << exp->str() << std::endl;
					if (auto ctor = std::dynamic_pointer_cast<ConstructorBase>(exp)) {
						ctor->setTemp();
					}
				}
				//should check inside scope
			}

			//static string typeStr() { return "dummyT"; }

		protected:
			stringPtr namePtr;
			Ex exp;
			mutable ObjFlags flags;

		public:

			stringPtr strPtr() const {
				return namePtr;
			}

			const string& str() const {
				return *strPtr();
			}

		};

		template<typename T>
		struct NamedObjectInit {
			NamedObjectInit(const Ex & _exp, const string & s) : exp(_exp), name(s) {}
			Ex exp;
			string name;
		};

		template<typename T, typename ... Args>
		Ex createInit(const stringPtr& name, CtorStatus status, OpFlags ctor_flags, ObjFlags obj_flags, Args&&... args);

		template<typename T, typename ... Args>
		Ex createDeclaration(const stringPtr & name, ObjFlags obj_flags, Args &&... args)
		{
			return createInit<T, Args...>(name, DECLARATION, OpFlags::NONE, obj_flags, std::forward<Args>(args)...);
		}

		template<typename T>
		class NamedObject : public NamedObjectBase {

		public:
			NamedObjectInit<T> operator<<(const string & s) const && {
				return { getExTmp() , s };
			}

		protected:
			NamedObject(
				const string & _name = "",
				ObjFlags obj_flags = ObjFlags::IS_TRACKED
			) : NamedObjectBase(_name, obj_flags)
			{
				checkName();
				exp = createDeclaration<T>(strPtr(), obj_flags);
				checkDisabling();
			}

			NamedObject(
				const string& _name,
				OpFlags ctor_flags 
			) : NamedObjectBase(_name, ObjFlags::IS_TRACKED)
			{
				checkName();
				exp = createInit<T>(strPtr(), INITIALISATION, ctor_flags, ObjFlags::IS_TRACKED);
				checkDisabling();
			}

			NamedObject(
				const Ex & _ex,
				OpFlags ctor_flags = OpFlags::NONE,
				ObjFlags obj_flags = ObjFlags::IS_TRACKED,
				const string & s = ""
			) : NamedObjectBase(s, obj_flags)
			{
				checkName();
				exp = createInit<T>(strPtr(), FORWARD, ctor_flags, obj_flags, _ex);
				checkDisabling();
			}

			template<typename ... Args>
			NamedObject(
				OpFlags ctor_flags,
				ObjFlags obj_flags,
				const string & s,
				Args &&... args
			) : NamedObjectBase(s, obj_flags)
			{
				checkName();
				exp = createInit<T>(strPtr(), INITIALISATION, ctor_flags, obj_flags, std::forward<Args>(args) ...);
				checkDisabling();
			}

			NamedObject(const NamedObjectInit<T> & obj_init)
				: NamedObjectBase(obj_init.name, ObjFlags::IS_TRACKED | ObjFlags::IS_USED)
			{
				checkName();
				exp = createInit<T>(strPtr(), INITIALISATION, OpFlags::NONE, ObjFlags::NONE, obj_init.exp);
			}

			void checkName()
			{
				using AutoName = typename AutoNaming<T>::Type;
				if (*namePtr == "") {
					namePtr = std::make_shared<string>(AutoName::getNextName());
				}

			}

			//static CounterData counterData;

		};

		template<typename T>
		NamedObjectInit<T> operator>>(const string& s, T&& t) {
			return { getExp(std::forward<T>(t)) , s };
		}

		//template<typename T> CounterData NamedObject<T>::counterData = {};

		template<typename T>
		inline Ex getExp(T && t)
		{
			return std::forward<T>(t).getEx();
		}

		template<> inline Ex getExp<const bool &>(const bool & b) {
			return createExp<Litteral<bool>>(b);
		}
		template<> inline Ex getExp<bool&>(bool & b) {
			return createExp<Litteral<bool>>(b);
		}
		template<> inline Ex getExp<bool>(bool && b) {
			return createExp<Litteral<bool>>(b);
		}

		template<> inline Ex getExp<const int &>(const int & i) {
			return createExp<Litteral<int>>(i);
		}
		template<> inline Ex getExp<const int>(const int && i) {
			return createExp<Litteral<int>>(i);
		}
		template<> inline Ex getExp<int&>(int & i) {
			return createExp<Litteral<int>>(i);
		}
		template<> inline Ex getExp<int>(int && i) {
			return createExp<Litteral<int>>(i);
		}

		template<> inline Ex getExp<uint>(uint && i) {
			return createExp<Litteral<uint>>(i);
		}
		template<> inline Ex getExp<uint&>(uint & i) {
			return createExp<Litteral<uint>>(i);
		}

		template<> inline Ex getExp<float>(float && d) {
			return createExp<Litteral<float>>(d);
		}
		template<> inline Ex getExp<float&>(float & d) {
			return createExp<Litteral<float>>(d);
		}

		template<> inline Ex getExp<const double &>(const double & d) {
			return createExp<Litteral<double>>(d);
		}
		template<> inline Ex getExp<double>(double && d) {
			return createExp<Litteral<double>>(d);
		}
		template<> inline Ex getExp<double&>(double & d) {
			return createExp<Litteral<double>>(d);
		}

	}

}

#undef EX
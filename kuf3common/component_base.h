#pragma once
#include "entity_enum.h"

class CEntity;

typedef std::shared_ptr<CEntity> EntityPtr;
typedef std::weak_ptr<CEntity> EntityWeakPtr;

template<typename CLASS>
class do_attr_Handler
{
public:
	virtual void SetAttr(CLASS* thisClass, const boost::any& value) {};
	virtual boost::any GetAttr(const CLASS* thisClass) const { return boost::any(); };
	virtual bool Do(CLASS* thisClass, const boost::any& param) { return false; }
};

template <typename TYPE, typename CLASS>
class _Attr : public do_attr_Handler<CLASS>
{
public:
	_Attr(TYPE CLASS::*ptrMem) : m_ptrMem(ptrMem) {}

public:
	virtual void SetAttr(CLASS* thisClass, const boost::any& value) override
	{
		thisClass->*(m_ptrMem) = boost::any_cast<TYPE>(value);
	}
	virtual boost::any GetAttr(const CLASS* thisClass) const override
	{
		return thisClass->*(m_ptrMem);
	}

private:
	TYPE CLASS::*m_ptrMem;
};

template <typename CLASS>
class _Do : public do_attr_Handler<CLASS>
{
private:
	typedef bool (CLASS::*DoFunc)(const boost::any&);

public:
	_Do(DoFunc DoFunc_) : m_DoFunc(DoFunc_) {}

public:
	virtual bool Do(CLASS* thisClass, const boost::any& param) override
	{
		return (thisClass->*(m_DoFunc))(param);
	}

private:
	DoFunc m_DoFunc;
};

template <typename TYPE, typename CLASS>
class Custom_Attr : public do_attr_Handler<CLASS>
{
private:
	typedef void (CLASS::*SetFunc)(TYPE);
	typedef void (CLASS::*GetFunc)(TYPE&) const;

public:
	Custom_Attr(SetFunc SetFunc_, GetFunc GetFunc_)
		: m_GetFunc(GetFunc_)
		, m_SetFunc(SetFunc_) {}

public:
	virtual void SetAttr(CLASS* thisClass, const boost::any& value) override
	{
		(thisClass->*(m_SetFunc))(boost::any_cast<TYPE>(value));
	}
	virtual boost::any GetAttr(const CLASS* thisClass) const override
	{
		TYPE value_;
		(thisClass->*(m_GetFunc))(value_);
		return boost::any(value_);
	}

private:
	GetFunc m_GetFunc;
	SetFunc m_SetFunc;
};


class CComponentBase
{
public:
	explicit CComponentBase(EntityPtr pEntity) : m_pEntity(pEntity.get()) {}
	virtual ~CComponentBase() = 0 {}

public:
	virtual bool Do(const std::string& strAction, const boost::any& param) { return false; }
	virtual void SetAttr(const std::string& strAttr, const boost::any& value) {}
	virtual boost::any GetAttr(const std::string& strAttr) const { return boost::any(); }
	virtual COMPONENT_ID GetComponentID() const = 0;

public:
	virtual void Update(float fElapsedTime) {}
	virtual void Render() {}

protected:
	CEntity* m_pEntity;
};

typedef std::unique_ptr<CComponentBase> CompPtr;

#define DEFINE_DO_ATTR(class_name) \
	template <typename TYPE> using a_h = _Attr<TYPE,class_name>;\
	template <typename TYPE> using ca_h = Custom_Attr<TYPE,class_name>;\
	typedef std::shared_ptr<do_attr_Handler<class_name>> DOATTRHANDLER_PTR;\
	static std::map< std::pair<const std::string,DO_ATTR>, DOATTRHANDLER_PTR> s_mapDoAttrtbl; \

#define BEGIN_DO_ATTR(class_name)\
	using _= class_name;\
	typedef std::map< std::pair<const std::string,DO_ATTR>, class_name::DOATTRHANDLER_PTR> mapDoAtt; \
	mapDoAtt class_name::s_mapDoAttrtbl = \
	{ \

#define ATT_ITEM(attr_name, attr_type, member ) \
	{ {attr_name,DO_ATTR::ATTR}, std::make_shared<a_h<attr_type>>(&_::member) }, \

#define CUSTOM_ATT_ITEM(attr_name, attr_type, setfunc,getfunc) \
	{ {attr_name,DO_ATTR::ATTR}, std::make_shared<ca_h<attr_type>>(&_::setfunc, &_::getfunc) }, \

#define DO_ITEM(do_name, dofunc ) \
	{ {do_name,DO_ATTR::DO}, std::make_shared<_Do<_>>(&_::dofunc) }, \

#define END_DO_ATTR };



#define SET_ATTR \
	virtual void SetAttr(const std::string& strAttr, const boost::any& value) override\
	{ \
		auto iter = s_mapDoAttrtbl.find(std::pair<const std::string,DO_ATTR>{strAttr,DO_ATTR::ATTR});\
		if (iter != s_mapDoAttrtbl.end()) \
			iter->second->SetAttr(this, value); \
	} 

#define GET_ATTR \
	virtual boost::any GetAttr(const std::string& strAttr) const override\
{\
	auto iter = s_mapDoAttrtbl.find(std::pair<const std::string,DO_ATTR>{strAttr,DO_ATTR::ATTR});\
	if (iter != s_mapDoAttrtbl.end())\
	{\
		return iter->second->GetAttr(this);\
	}\
	return boost::any();\
}

#define DO_ACTION \
	virtual bool Do(const std::string& strAction, const boost::any& param) \
		{\
	auto iter = s_mapDoAttrtbl.find(std::pair<const std::string,DO_ATTR>{strAction,DO_ATTR::DO});\
	if (iter != s_mapDoAttrtbl.end())\
		{\
		return iter->second->Do(this, param);\
		}\
	return false;\
}


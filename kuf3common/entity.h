#pragma once

#include "component_base.h"

typedef std::map<std::string, boost::any> dynamic;
typedef std::map<std::string, CompPtr> COMMAP;

class CEntity : public std::enable_shared_from_this <CEntity>
{
public:
	explicit CEntity()
		: m_bIsDelete(false)
		, m_uEntityID(0) {}

public:
	void Do(const std::string& strAction, const boost::any& param = boost::any(0))
	{
		for (auto& comp : m_Components)
		{
			if (comp.second->Do(strAction, param))
				return;
		}
	}
	void SetAttr(const std::string& strAttr, const boost::any& value)
	{
		for (auto& comp : m_Components)
		{
			comp.second->SetAttr(strAttr, value);
		}
	}
	boost::any GetAttr(const std::string& strAttr) const
	{
		boost::any value;

		for (auto& comp : m_Components)
		{
			value = comp.second->GetAttr(strAttr);
			if (!value.empty())
				break;
		}
		return value;
	}
	template <typename T> T GetAttr(const std::string& strAttr) const
	{
		boost::any value;
		for (auto& comp : m_Components)
		{
			value = comp.second->GetAttr(strAttr);
			if (!value.empty())
				break;
		}
		return boost::any_cast<T>(value);
	}
	template <typename T> T GetAttr(const std::string& strAttr, T def) const
	{
		boost::any value;
		for (auto& comp : m_Components)
		{
			value = comp.second->GetAttr(strAttr);
			if (!value.empty())
				break;
		}
		if (value.empty())
			return def;
		else
			return boost::any_cast<T>(value);
	}
public:
	void Update(float fElapsedTime)
	{
		for (auto& comp : m_Components)
			comp.second->Update(fElapsedTime);
	}
	void Render()
	{
		for (auto& comp : m_Components)
			comp.second->Render();
	}

public:
	bool IsDelete() const { return m_bIsDelete; }
	void SetIsDelete() { m_bIsDelete = true; }
	void SetObjType(OBJ_TYPE ObjType) { m_eObjType = ObjType; }
	OBJ_TYPE GetObjType() { return m_eObjType; }
	void SetEntityID(unsigned uEntityID) { m_uEntityID = uEntityID; }
	unsigned GetEntityID() { return m_uEntityID; }

public:
	void InsertComponent(std::string strComID, CompPtr pComponent)
	{
		m_Components.emplace(COMMAP::value_type(strComID, std::move(pComponent)));
	}

private:
	bool m_bIsDelete;
	OBJ_TYPE m_eObjType;
	COMMAP m_Components;
	unsigned m_uEntityID;
};

typedef std::shared_ptr<CEntity> EntityPtr;
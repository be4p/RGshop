class RG_ShopManagerComponentClass : ScriptComponentClass
{
}

class RG_ShopManagerComponent : ScriptComponent
{
	
	[Attribute(params: "conf")]
	protected ResourceName m_sConfig;
	protected ref array<ref SCR_ArsenalItemStandalone> m_aArsenalItems = new array<ref SCR_ArsenalItemStandalone>;
	protected ref map<string, SCR_ArsenalItemStandalone> m_mArsenalItemsByPath = new map<string, SCR_ArsenalItemStandalone>;
	
	protected static RG_ShopManagerComponent s_pInstance;

	override void OnPostInit(IEntity owner)
	{
		s_pInstance = this;

		if (m_sConfig.IsEmpty())
			return;

		Resource resource = Resource.Load(m_sConfig);
		if (!resource || !resource.IsValid())
		{
			PrintFormat("[RG Shop] Failed to load shop config: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
		{
			PrintFormat("[RG Shop] Shop config has no resource object: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		SCR_ArsenalItemListConfig arsenalConfig = SCR_ArsenalItemListConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resourceObject.ToBaseContainer()));
		if (!arsenalConfig)
		{
			PrintFormat("[RG Shop] Invalid shop config type: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		m_aArsenalItems.Clear();
		m_mArsenalItemsByPath.Clear();
		arsenalConfig.GetArsenalItems(m_aArsenalItems);

		foreach (SCR_ArsenalItemStandalone item : m_aArsenalItems)
		{
			if (!item)
				continue;

			Resource itemResource = item.GetItemResource();
			if (!itemResource || !itemResource.IsValid() || !itemResource.GetResource())
			{
				Print("[RG Shop] Shop config contains an invalid item resource", LogLevel.ERROR);
				continue;
			}

			string resourcePath = NormalizeResourcePath(itemResource.GetResource().GetResourceName().GetPath());
			if (resourcePath.IsEmpty())
				continue;

			if (m_mArsenalItemsByPath.Contains(resourcePath))
			{
				PrintFormat("[RG Shop] Duplicate shop item ignored: %1", resourcePath, level: LogLevel.ERROR);
				continue;
			}

			m_mArsenalItemsByPath.Insert(resourcePath, item);
		}
	}

	static RG_ShopManagerComponent GetInstance()
	{
		return s_pInstance;
	}

	SCR_ArsenalItemStandalone GetItemByPath(string resourcePath)
	{
		return m_mArsenalItemsByPath.Get(NormalizeResourcePath(resourcePath));
	}

	void RegisterShopItems(RG_ShopComponent shop, array<SCR_ArsenalItem> shopItems)
	{
		if (!shop)
			return;

		foreach (SCR_ArsenalItem arsenalItem : shopItems)
		{
			SCR_ArsenalItemStandalone item = SCR_ArsenalItemStandalone.Cast(arsenalItem);
			if (!item)
				continue;

			Resource itemResource = item.GetItemResource();
			if (!itemResource || !itemResource.IsValid() || !itemResource.GetResource())
				continue;

			string resourcePath = NormalizeResourcePath(itemResource.GetResource().GetResourceName().GetPath());
			SCR_ArsenalItemStandalone existingItem = m_mArsenalItemsByPath.Get(resourcePath);
			if (existingItem)
			{
				if (existingItem.GetSellValue() != item.GetSellValue())
					PrintFormat("[RG Shop] Conflicting SellValue for %1. Global catalog keeps the first value.", resourcePath, level: LogLevel.WARNING);

				continue;
			}

			m_mArsenalItemsByPath.Insert(resourcePath, item);
		}
	}

	static string NormalizeResourcePath(string resourcePath)
	{
		resourcePath.Replace("\\", "/");
		resourcePath.ToLower();
		return resourcePath;
	}
}

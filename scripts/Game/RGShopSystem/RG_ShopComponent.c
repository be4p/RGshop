class RG_ShopComponentClass : ScriptComponentClass
{
}
class RG_ShopComponent: ScriptComponent
{
	protected ref map<string, SCR_ArsenalItemStandalone> m_mShopItemsByPath = new map<string, SCR_ArsenalItemStandalone>;
	protected ref array<SCR_ArsenalItem> m_aShopItems = {};
	protected int m_iRegistrationAttempts;

	override void OnPostInit(IEntity owner)
	{
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(owner);
		if (!arsenalComponent)
		{
			Print("[RG Shop] RG_ShopComponent requires SCR_ArsenalComponent on its owner", LogLevel.ERROR);
			return;
		}

		arsenalComponent.GetFilteredOverwriteArsenalItems(m_aShopItems);
		foreach (SCR_ArsenalItem arsenalItem : m_aShopItems)
		{
			SCR_ArsenalItemStandalone item = SCR_ArsenalItemStandalone.Cast(arsenalItem);
			if (!item)
				continue;

			Resource itemResource = item.GetItemResource();
			if (!itemResource || !itemResource.IsValid() || !itemResource.GetResource())
				continue;

			string resourcePath = RG_ShopManagerComponent.NormalizeResourcePath(itemResource.GetResource().GetResourceName().GetPath());
			if (!m_mShopItemsByPath.Contains(resourcePath))
				m_mShopItemsByPath.Insert(resourcePath, item);
		}

		RegisterWithManager();
	}

	protected void RegisterWithManager()
	{
		RG_ShopManagerComponent manager = RG_ShopManagerComponent.GetInstance();
		if (manager)
		{
			manager.RegisterShopItems(this, m_aShopItems);
			return;
		}

		m_iRegistrationAttempts++;
		if (m_iRegistrationAttempts < 20)
			GetGame().GetCallqueue().CallLater(RegisterWithManager, 100, false);
		else
			Print("[RG Shop] Failed to register shop: RG_ShopManagerComponent not found", LogLevel.ERROR);
	}

	SCR_ArsenalItemStandalone GetItem(ResourceName resourceName)
	{
		return m_mShopItemsByPath.Get(RG_ShopManagerComponent.NormalizeResourcePath(resourceName.GetPath()));
	}

	int GetBuyPrice(ResourceName resourceName)
	{
		SCR_ArsenalItemStandalone item = GetItem(resourceName);
		if (!item)
			return 0;

		return item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
	}

	bool IsEntityAvailable(IEntity entity)
	{
		if (!entity || !entity.GetPrefabData())
			return false;

		if (GetItem(entity.GetPrefabData().GetPrefabName()))
			return true;

		BaseContainer prefab = entity.GetPrefabData().GetPrefab();
		while (prefab)
		{
			string path = RG_ShopManagerComponent.NormalizeResourcePath(prefab.GetName());
			if (m_mShopItemsByPath.Contains(path))
				return true;

			prefab = prefab.GetAncestor();
		}

		return false;
	}

}

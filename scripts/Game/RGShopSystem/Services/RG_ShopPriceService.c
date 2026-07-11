class RG_ShopPriceService
{
	static SCR_ArsenalItemStandalone FindItem(ResourceName resourceName)
	{
		SCR_ArsenalItemStandalone item = FindItemByPath(resourceName.GetPath());
		if (!item)
			return null;

		return item;
	}

	protected static SCR_ArsenalItemStandalone FindItemByPath(string resourcePath)
	{
		RG_ShopManagerComponent manager = RG_ShopManagerComponent.GetInstance();
		if (!manager)
			return null;

		return manager.GetItemByPath(resourcePath);
	}

	static int GetBuyPrice(ResourceName resourceName)
	{
		SCR_ArsenalItemStandalone item = FindItem(resourceName);
		if (!item)
			return 0;

		return item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
	}

	static int GetSellPrice(ResourceName resourceName)
	{
		SCR_ArsenalItemStandalone item = FindItem(resourceName);
		return GetConfiguredSellPrice(item);
	}

	static int GetBaseEntitySellPrice(IEntity entity)
	{
		if (!entity)
			return 0;

		EntityPrefabData prefabData = entity.GetPrefabData();
		if (!prefabData)
			return 0;

		BaseContainer prefab = prefabData.GetPrefab();
		
		while (prefab)
		{
			string prefabPath = prefab.GetName();
			SCR_ArsenalItemStandalone configuredItem = FindItemByPath(prefabPath);
			if (configuredItem)
			{
				int price = GetConfiguredSellPrice(configuredItem);
				return price;
			}
			
			prefab = prefab.GetAncestor();
		}

		return 0;
	}

	static int GetItemSellPriceWithAttachments(SCR_InventorySlotUI slot)
	{
		if (!slot)
			return 0;

		InventoryItemComponent itemComponent = slot.GetInventoryItemComponent();
		if (!itemComponent)
			return 0;

		return GetEntitySellPriceWithAttachments(itemComponent.GetOwner());
	}

	static int GetEntitySellPriceWithAttachments(IEntity itemEntity)
	{
		if (!itemEntity)
			return 0;

		int basePrice = GetBaseEntitySellPrice(itemEntity);
		int attachmentsPrice = 0;

		BaseInventoryStorageComponent itemStorage = BaseInventoryStorageComponent.Cast(itemEntity.FindComponent(BaseInventoryStorageComponent));
		if (itemStorage)
		{
			array<IEntity> attachments = {};
			itemStorage.GetAll(attachments);

			foreach (IEntity attachment : attachments)
			{
				int attachmentPrice = GetBaseEntitySellPrice(attachment);
				attachmentsPrice += attachmentPrice;
			}
		}

		int totalPrice = basePrice + attachmentsPrice;
		return totalPrice;
	}

	static int GetEntitySellPriceWithAttachmentsForShop(IEntity itemEntity, RG_ShopComponent shop)
	{
		if (!itemEntity || !shop || !shop.IsEntityAvailable(itemEntity))
			return 0;

		int totalPrice = GetBaseEntitySellPrice(itemEntity);
		BaseInventoryStorageComponent itemStorage = BaseInventoryStorageComponent.Cast(itemEntity.FindComponent(BaseInventoryStorageComponent));
		if (!itemStorage)
			return totalPrice;

		array<IEntity> attachments = {};
		itemStorage.GetAll(attachments);
		foreach (IEntity attachment : attachments)
		{
			if (shop.IsEntityAvailable(attachment))
				totalPrice += GetBaseEntitySellPrice(attachment);
		}

		return totalPrice;
	}

	protected static int GetConfiguredSellPrice(SCR_ArsenalItemStandalone item)
	{
		if (!item)
			return 0;

		int sellPrice = item.GetSellValue();
		if (sellPrice <= 0)
			sellPrice = item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false) / 2;

		return sellPrice;
	}
}

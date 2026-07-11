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

		int totalPrice = GetBaseEntitySellPrice(itemEntity);
		array<BaseInventoryStorageComponent> storages = {};
		GetEntityStorages(itemEntity, storages);
		foreach (BaseInventoryStorageComponent storage : storages)
		{
			array<IEntity> containedItems = {};
			storage.GetAll(containedItems);
			foreach (IEntity containedItem : containedItems)
			{
				if (!IsDirectlyInsideStorage(containedItem, storage))
					continue;

				totalPrice += GetEntitySellPriceWithAttachments(containedItem);
			}
		}

		return totalPrice;
	}

	static int GetEntitySellPriceWithAttachmentsForShop(IEntity itemEntity, RG_ShopComponent shop)
	{
		if (!itemEntity || !shop)
			return 0;

		if (!shop.IsEntityAvailable(itemEntity))
		{
			int contentsPrice;
			array<BaseInventoryStorageComponent> containerStorages = {};
			GetEntityStorages(itemEntity, containerStorages);
			foreach (BaseInventoryStorageComponent containerStorage : containerStorages)
			{
				array<IEntity> containedItems = {};
				containerStorage.GetAll(containedItems);
				foreach (IEntity containedItem : containedItems)
				{
					if (!IsDirectlyInsideStorage(containedItem, containerStorage))
						continue;

					contentsPrice += GetEntitySellPriceWithAttachmentsForShop(containedItem, shop);
				}
			}

			return contentsPrice;
		}

		int totalPrice = GetBaseEntitySellPrice(itemEntity);
		array<BaseInventoryStorageComponent> itemStorages = {};
		GetEntityStorages(itemEntity, itemStorages);
		foreach (BaseInventoryStorageComponent itemStorage : itemStorages)
		{
			array<IEntity> attachments = {};
			itemStorage.GetAll(attachments);
			foreach (IEntity attachment : attachments)
			{
				if (!IsDirectlyInsideStorage(attachment, itemStorage) || !shop.IsEntityAvailable(attachment))
					continue;

				totalPrice += GetEntitySellPriceWithAttachmentsForShop(attachment, shop);
			}
		}

		return totalPrice;
	}

	static void GetEntityStorages(IEntity entity, out notnull array<BaseInventoryStorageComponent> storages)
	{
		if (!entity)
			return;

		array<Managed> components = {};
		entity.FindComponents(BaseInventoryStorageComponent, components);
		foreach (Managed component : components)
		{
			BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(component);
			if (storage)
				storages.Insert(storage);
		}
	}

	protected static bool IsDirectlyInsideStorage(IEntity item, BaseInventoryStorageComponent storage)
	{
		if (!item || !storage)
			return false;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return false;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();
		return parentSlot && parentSlot.GetStorage() == storage;
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

class RG_ShopTransactionService
{
	static void TryBuyItem_S(SCR_PlayerController controller, RplId resourceComponentRplId, RplId storageComponentRplId, ResourceName itemResource, float maxDistanceSq)
	{
		if (!controller)
			return;

		SCR_ChimeraCharacter buyer = SCR_ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!buyer)
			return;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(buyer.GetCharacterController().GetInventoryStorageManager());
		if (!inventoryManager || !storageComponentRplId.IsValid() || !resourceComponentRplId.IsValid())
			return;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(Replication.FindItem(storageComponentRplId));
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		if (!storage || !resourceComponent)
			return;

		IEntity storageOwner = storage.GetOwner();
		IEntity shopEntity = resourceComponent.GetOwner();
		if (!storageOwner || !shopEntity)
			return;

		RG_ShopComponent shopComponent = RG_ShopComponent.Cast(shopEntity.FindComponent(RG_ShopComponent));
		if (!shopComponent || !IsStorageOwnedByCharacter(storageOwner, buyer))
			return;

		IEntity characterParent = FindCharacterParent(storageOwner);
		if (!characterParent && vector.DistanceSq(buyer.GetOrigin(), storageOwner.GetOrigin()) > maxDistanceSq)
			return;

		if (vector.DistanceSq(shopEntity.GetOrigin(), storageOwner.GetOrigin()) > maxDistanceSq)
			return;

		if (!inventoryManager.ValidateStorageRequest(shopEntity))
			return;

		if (!SCR_ArsenalComponent.FindArsenalComponent(shopEntity))
			return;

		int price = shopComponent.GetBuyPrice(itemResource);
		if (price <= 0)
			return;

		RG_MoneyComponent moneyComponent = RG_MoneyComponent.Cast(buyer.FindComponent(RG_MoneyComponent));
		if (!moneyComponent || moneyComponent.GetMoney() < price)
			return;

		if (!inventoryManager.TrySpawnPrefabToStorage(itemResource, storage, cb: new SCR_PrefabSpawnCallback(storage)))
			return;

		moneyComponent.TrySpendMoney_S(price);
	}

	static void TrySellItem_S(SCR_PlayerController controller, RplId resourceComponentRplId, RplId inventoryItemRplId, float maxDistanceSq)
	{
		if (!controller || !inventoryItemRplId.IsValid() || !resourceComponentRplId.IsValid())
			return;

		SCR_ChimeraCharacter seller = SCR_ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!seller)
			return;

		InventoryItemComponent itemComponent = InventoryItemComponent.Cast(Replication.FindItem(inventoryItemRplId));
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.Cast(Replication.FindItem(resourceComponentRplId));
		if (!itemComponent || !resourceComponent)
			return;

		IEntity itemEntity = itemComponent.GetOwner();
		IEntity shopEntity = resourceComponent.GetOwner();
		if (!itemEntity || !shopEntity)
			return;

		RG_ShopComponent shopComponent = RG_ShopComponent.Cast(shopEntity.FindComponent(RG_ShopComponent));
		if (!shopComponent || !IsItemOwnedByCharacter(itemEntity, seller))
			return;

		IEntity characterParent = FindCharacterParent(itemEntity);
		if (!characterParent && vector.DistanceSq(seller.GetOrigin(), itemEntity.GetOrigin()) > maxDistanceSq)
			return;

		SCR_InventoryStorageManagerComponent inventoryManager = SCR_InventoryStorageManagerComponent.Cast(seller.GetCharacterController().GetInventoryStorageManager());
		if (!inventoryManager || !inventoryManager.ValidateStorageRequest(shopEntity))
			return;

		if (vector.DistanceSq(shopEntity.GetOrigin(), itemEntity.GetOrigin()) > maxDistanceSq)
			return;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(shopEntity);
		if (!arsenalComponent)
			return;

		RG_MoneyComponent moneyComponent = RG_MoneyComponent.Cast(seller.FindComponent(RG_MoneyComponent));
		if (!moneyComponent)
			return;

		if (!shopComponent.IsEntityAvailable(itemEntity))
		{
			int contentsPrice = SellAcceptedContentsRecursive_S(itemEntity, shopComponent, arsenalComponent, inventoryManager, controller);
			if (contentsPrice > 0)
				moneyComponent.AddMoney_S(contentsPrice);

			return;
		}

		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
		if (!attributes || !attributes.IsRefundable())
			return;

		int sellPrice = RG_ShopPriceService.GetEntitySellPriceWithAttachmentsForShop(itemEntity, shopComponent);
		if (sellPrice <= 0)
			return;

		SCR_ArsenalManagerComponent.OnItemRefunded_S(itemEntity, controller, arsenalComponent);
		if (!TryDeleteItem_S(itemEntity, null))
			return;

		moneyComponent.AddMoney_S(sellPrice);
	}

	protected static int SellAcceptedContentsRecursive_S(IEntity containerEntity, RG_ShopComponent shopComponent, SCR_ArsenalComponent arsenalComponent, SCR_InventoryStorageManagerComponent inventoryManager, PlayerController controller)
	{
		if (!containerEntity || !shopComponent || !arsenalComponent || !inventoryManager || !controller)
			return 0;

		int totalPrice;
		array<BaseInventoryStorageComponent> containerStorages = {};
		RG_ShopPriceService.GetEntityStorages(containerEntity, containerStorages);
		foreach (BaseInventoryStorageComponent containerStorage : containerStorages)
		{
			array<IEntity> containedItems = {};
			containerStorage.GetAll(containedItems);
			foreach (IEntity containedItem : containedItems)
			{
				InventoryItemComponent containedItemComponent;
				if (!IsDirectlyInsideStorage(containedItem, containerStorage, containedItemComponent))
					continue;

				if (!shopComponent.IsEntityAvailable(containedItem))
				{
					totalPrice += SellAcceptedContentsRecursive_S(containedItem, shopComponent, arsenalComponent, inventoryManager, controller);
					continue;
				}

				SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(containedItemComponent.GetAttributes());
				if (!attributes || !attributes.IsRefundable())
					continue;

				int itemPrice = RG_ShopPriceService.GetEntitySellPriceWithAttachmentsForShop(containedItem, shopComponent);
				if (itemPrice <= 0)
					continue;

				SCR_ArsenalManagerComponent.OnItemRefunded_S(containedItem, controller, arsenalComponent);
				if (inventoryManager.TryDeleteItem(containedItem))
					totalPrice += itemPrice;
			}
		}

		return totalPrice;
	}

	protected static bool TryDeleteItem_S(IEntity itemEntity, SCR_InventoryStorageManagerComponent fallbackManager)
	{
		if (!itemEntity)
			return false;

		IEntity parentEntity = itemEntity.GetParent();
		SCR_InventoryStorageManagerComponent inventoryManager;
		if (parentEntity)
			inventoryManager = SCR_InventoryStorageManagerComponent.Cast(parentEntity.FindComponent(SCR_InventoryStorageManagerComponent));

		if (!inventoryManager)
			inventoryManager = fallbackManager;

		if (inventoryManager)
			return inventoryManager.TryDeleteItem(itemEntity);

		RplComponent.DeleteRplEntity(itemEntity, false);
		return true;
	}

	protected static bool IsDirectlyInsideStorage(IEntity item, BaseInventoryStorageComponent storage, out InventoryItemComponent itemComponent)
	{
		if (!item || !storage)
			return false;

		itemComponent = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
		if (!itemComponent)
			return false;

		InventoryStorageSlot parentSlot = itemComponent.GetParentSlot();
		return parentSlot && parentSlot.GetStorage() == storage;
	}

	protected static IEntity FindCharacterParent(IEntity entity)
	{
		while (entity)
		{
			if (ChimeraCharacter.Cast(entity))
				return entity;

			entity = entity.GetParent();
		}

		return null;
	}

	protected static bool IsStorageOwnedByCharacter(IEntity storageOwner, ChimeraCharacter character)
	{
		IEntity characterParent = FindCharacterParent(storageOwner);
		return !characterParent || characterParent == character;
	}

	protected static bool IsItemOwnedByCharacter(IEntity item, ChimeraCharacter character)
	{
		IEntity characterParent = FindCharacterParent(item);
		return !characterParent || characterParent == character;
	}
}

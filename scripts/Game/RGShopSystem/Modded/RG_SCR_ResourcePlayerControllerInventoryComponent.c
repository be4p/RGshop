modded class SCR_ResourcePlayerControllerInventoryComponent
{
	void RpcAsk_RGShopRequestItem(RplId resourceComponentRplId, RplId storageComponentRplId, ResourceName itemResource)
	{
		Rpc(RpcAsk_RGShopRequestItem_S, resourceComponentRplId, storageComponentRplId, itemResource);
	}

	void RpcAsk_RGShopSellItem(RplId resourceComponentRplId, RplId inventoryItemRplId)
	{
		Rpc(RpcAsk_RGShopSellItem_S, resourceComponentRplId, inventoryItemRplId);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RGShopRequestItem_S(RplId resourceComponentRplId, RplId storageComponentRplId, ResourceName itemResource)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
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
		if (!shopComponent)
			return;

		IEntity storageParent = storageOwner;
		while (storageParent)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(storageParent);
			if (character)
			{
				if (character != buyer)
					return;

				break;
			}

			storageParent = storageParent.GetParent();
		}

		if (!storageParent && vector.DistanceSq(buyer.GetOrigin(), storageOwner.GetOrigin()) > MAX_REFUNDING_DISTANCE_SQ)
			return;

		if (vector.DistanceSq(shopEntity.GetOrigin(), storageOwner.GetOrigin()) > MAX_REFUNDING_DISTANCE_SQ)
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

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RGShopSellItem_S(RplId resourceComponentRplId, RplId inventoryItemRplId)
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetOwner());
		if (!controller)
			return;

		SCR_ChimeraCharacter seller = SCR_ChimeraCharacter.Cast(controller.GetControlledEntity());
		if (!seller || !inventoryItemRplId.IsValid() || !resourceComponentRplId.IsValid())
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
		if (!shopComponent)
			return;

		IEntity itemParent = itemEntity;
		while (itemParent)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(itemParent);
			if (character)
			{
				if (character != seller)
					return;

				break;
			}

			itemParent = itemParent.GetParent();
		}

		if (!itemParent && vector.DistanceSq(seller.GetOrigin(), itemEntity.GetOrigin()) > MAX_REFUNDING_DISTANCE_SQ)
			return;

		SCR_InventoryStorageManagerComponent sellerInventoryManager = SCR_InventoryStorageManagerComponent.Cast(seller.GetCharacterController().GetInventoryStorageManager());
		if (!sellerInventoryManager || !sellerInventoryManager.ValidateStorageRequest(shopEntity))
			return;

		if (vector.DistanceSq(shopEntity.GetOrigin(), itemEntity.GetOrigin()) > MAX_REFUNDING_DISTANCE_SQ)
			return;

		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.FindArsenalComponent(shopEntity);
		if (!arsenalComponent)
			return;

		RG_MoneyComponent moneyComponent = RG_MoneyComponent.Cast(seller.FindComponent(RG_MoneyComponent));
		if (!moneyComponent)
			return;

		if (!shopComponent.IsEntityAvailable(itemEntity))
		{
			int contentsSellPrice = RG_SellAcceptedContentsRecursive(itemEntity, shopComponent, arsenalComponent, sellerInventoryManager);

			if (contentsSellPrice > 0)
				moneyComponent.AddMoney_S(contentsSellPrice);

			return;
		}

		SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(itemComponent.GetAttributes());
		if (!attributes || !attributes.IsRefundable())
			return;

		int sellPrice = RG_ShopPriceService.GetEntitySellPriceWithAttachmentsForShop(itemEntity, shopComponent);
		if (sellPrice <= 0)
			return;

		SCR_ArsenalManagerComponent.OnItemRefunded_S(itemEntity, PlayerController.Cast(GetOwner()), arsenalComponent);

		IEntity parentEntity = itemEntity.GetParent();
		SCR_InventoryStorageManagerComponent inventoryManager;
		if (parentEntity)
			inventoryManager = SCR_InventoryStorageManagerComponent.Cast(parentEntity.FindComponent(SCR_InventoryStorageManagerComponent));

		if (inventoryManager)
		{
			if (!inventoryManager.TryDeleteItem(itemEntity))
				return;
		}
		else
		{
			RplComponent.DeleteRplEntity(itemEntity, false);
		}

		moneyComponent.AddMoney_S(sellPrice);
	}

	protected int RG_SellAcceptedContentsRecursive(IEntity containerEntity, RG_ShopComponent shopComponent, SCR_ArsenalComponent arsenalComponent, SCR_InventoryStorageManagerComponent inventoryManager)
	{
		if (!containerEntity || !shopComponent || !arsenalComponent || !inventoryManager)
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
				if (!containedItem)
					continue;

				InventoryItemComponent containedItemComponent = InventoryItemComponent.Cast(containedItem.FindComponent(InventoryItemComponent));
				if (!containedItemComponent)
					continue;

				InventoryStorageSlot parentSlot = containedItemComponent.GetParentSlot();
				if (!parentSlot || parentSlot.GetStorage() != containerStorage)
					continue;

				if (!shopComponent.IsEntityAvailable(containedItem))
				{
					totalPrice += RG_SellAcceptedContentsRecursive(containedItem, shopComponent, arsenalComponent, inventoryManager);
					continue;
				}

				SCR_ItemAttributeCollection attributes = SCR_ItemAttributeCollection.Cast(containedItemComponent.GetAttributes());
				if (!attributes || !attributes.IsRefundable())
					continue;

				int itemPrice = RG_ShopPriceService.GetEntitySellPriceWithAttachmentsForShop(containedItem, shopComponent);
				if (itemPrice <= 0)
					continue;

				SCR_ArsenalManagerComponent.OnItemRefunded_S(containedItem, PlayerController.Cast(GetOwner()), arsenalComponent);
				if (inventoryManager.TryDeleteItem(containedItem))
					totalPrice += itemPrice;
			}
		}

		return totalPrice;
	}
}

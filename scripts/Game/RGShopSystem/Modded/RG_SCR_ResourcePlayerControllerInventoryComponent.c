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
		RG_ShopTransactionService.TryBuyItem_S(
			SCR_PlayerController.Cast(GetOwner()),
			resourceComponentRplId,
			storageComponentRplId,
			itemResource,
			MAX_REFUNDING_DISTANCE_SQ
		);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_RGShopSellItem_S(RplId resourceComponentRplId, RplId inventoryItemRplId)
	{
		RG_ShopTransactionService.TrySellItem_S(
			SCR_PlayerController.Cast(GetOwner()),
			resourceComponentRplId,
			inventoryItemRplId,
			MAX_REFUNDING_DISTANCE_SQ
		);
	}
}

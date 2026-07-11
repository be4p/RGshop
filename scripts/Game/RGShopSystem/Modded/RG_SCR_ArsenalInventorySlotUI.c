modded class SCR_ArsenalInventorySlotUI
{
	override string SetSlotSize()
	{
		if (!IsShop())
			return super.SetSlotSize();

		switch (m_Attributes.GetItemSize())
		{
			case ESlotSize.SLOT_1x1:
				m_iSizeX = 1;
				m_iSizeY = 1;
				break;
			case ESlotSize.SLOT_2x1:
				m_iSizeX = 2;
				m_iSizeY = 1;
				break;
			case ESlotSize.SLOT_2x2:
				m_iSizeX = 2;
				m_iSizeY = 2;
				break;
			case ESlotSize.SLOT_3x3:
				m_iSizeX = 3;
				m_iSizeY = 3;
				break;
		}

		return "{3B35DE9811F55E29}UI/layouts/Menus/Inventory/RGshopInventoryItemSlot.layout";
	}

	bool IsShop()
	{
		SCR_InventoryStorageBaseUI storageUI = GetStorageUI();
		if (!storageUI)
			return false;

		BaseInventoryStorageComponent storage = storageUI.GetCurrentNavigationStorage();
		if (!storage)
			return false;

		IEntity storageEntity = storage.GetOwner();
		if (!storageEntity)
			return false;

		return RG_ShopComponent.Cast(storageEntity.FindComponent(RG_ShopComponent)) != null;
	}

	override float GetTotalResources()
	{
		if (!IsShop())
			return super.GetTotalResources();

		if (!m_pItem || !m_pItem.GetOwner())
			return 0;

		EntityPrefabData prefabData = m_pItem.GetOwner().GetPrefabData();
		if (!prefabData)
			return 0;

		m_fSupplyCost = RG_ShopPriceService.GetBuyPrice(prefabData.GetPrefabName());
		return m_fSupplyCost;
	}

	override void UpdateTotalResources(float totalResources)
	{
		if (!IsShop())
		{
			super.UpdateTotalResources(totalResources);
			return;
		}

		SetItemAvailability(true);
		if (m_CostResourceHolder)
			m_CostResourceHolder.SetVisible(true);

		if (m_CostResourceHolderText)
			m_CostResourceHolderText.SetText(string.Format("%1$", totalResources.ToString()));
	}
}

enum RG_EShopDealDirection
{
	NONE,
	BUY,
	SELL
}

modded class SCR_InventoryMenuUI
{ 	
	protected RG_EShopDealDirection m_eShopDealDirection;
	const protected ResourceName m_sSellPriceUIInfoPrefab = "{B15C5823BAF921FF}RGsellPrice_ItemHint.conf";
	const protected ResourceName m_sBuyPriceUIInfoPrefab = "{A043AE89BCAA50DC}RGbuyPrice_ItemHint.conf";
	protected ref RG_PriceItemHintUIInfo m_SellPriceUIInfo;
	protected ref RG_PriceItemHintUIInfo m_BuyPriceUIInfo;

	int GetSellPrice()
	{
		if (!m_pFocusedSlotUI)
			return 0;

		SCR_InventorySlotUI focusedSlot = SCR_InventorySlotUI.Cast(m_pFocusedSlotUI);
		return RG_ShopPriceService.GetItemSellPriceWithAttachments(focusedSlot);
	}
	override void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{			
		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
		if (!arsenalSlot)
		{
			if (m_SellPriceUIInfo)
			{
				m_SellPriceUIInfo.SetSupplyCost(GetSellPrice());
				hintsInfo.InsertAt(m_SellPriceUIInfo, 0);
			}

			super.GetGeneralItemHintsInfos(hintsInfo);
			return;
		}

		if (arsenalSlot.IsShop())
		{
			super.GetGeneralItemHintsInfos(hintsInfo);

			for (int i = hintsInfo.Count() - 1; i >= 0; i--)
			{
				SCR_SupplyCostItemHintUIInfo supplyCostInfo = SCR_SupplyCostItemHintUIInfo.Cast(hintsInfo[i]);
				if (supplyCostInfo)
					hintsInfo.Remove(i);
			}

			if (m_BuyPriceUIInfo)
			{
				m_BuyPriceUIInfo.SetSupplyCost(arsenalSlot.GetItemSupplyCost());
				hintsInfo.InsertAt(m_BuyPriceUIInfo, 0);
			}

			return;
		}

		super.GetGeneralItemHintsInfos(hintsInfo);
	
	}
	protected bool IsShop()
	{
		m_eShopDealDirection = RG_EShopDealDirection.NONE;

		IEntity entityShop;
		RG_ShopComponent shopComponent;
		
		SCR_ArsenalInventorySlotUI arsenalUi = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		if(arsenalUi)
		{
			SCR_ResourceComponent arsenalResource = arsenalUi.GetArsenalResourceComponent();
			if (arsenalResource)
				entityShop = arsenalResource.GetOwner();

			m_eShopDealDirection = RG_EShopDealDirection.BUY;
		}
		else
		{
			if (!m_pActiveHoveredStorageUI)
				return false;

			SCR_InventorySlotUI lastFocus = m_pActiveHoveredStorageUI.GetLastFocusedSlot();
			SCR_ResourceComponent res;
			SCR_ArsenalInventorySlotUI arsenalSlot;
			if(lastFocus)
				arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(lastFocus);
			if(arsenalSlot)
				res = arsenalSlot.GetArsenalResourceComponent();
			if(res)
				entityShop = res.GetOwner();
			m_eShopDealDirection = RG_EShopDealDirection.SELL;
		}
		if(entityShop)
			shopComponent = RG_ShopComponent.Cast(entityShop.FindComponent(RG_ShopComponent));
		if(shopComponent)
		{
			return true;
		}
		else
		{
			m_eShopDealDirection = RG_EShopDealDirection.NONE;
			return false;
		}
		
	}
	
	protected bool CanInsertItem(bool dragged)
	{
		bool canInsert = false;
		if(dragged)
		{
			if (!m_pActiveHoveredStorageUI || !m_pSelectedSlotUI)
				return false;

			BaseInventoryStorageComponent currentStorage = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
			if(!currentStorage)
				return false;
			IEntity storageEntity = currentStorage.GetOwner();
			if (!storageEntity)
				return false;

			if(m_eShopDealDirection == RG_EShopDealDirection.BUY)
			{
				RG_ShopComponent shopComponent = RG_ShopComponent.Cast(storageEntity.FindComponent(RG_ShopComponent));
				if(shopComponent)
					return false;
			}

			InventoryItemComponent selectedItemComponent = m_pSelectedSlotUI.GetInventoryItemComponent();
			if (!selectedItemComponent || !selectedItemComponent.GetOwner())
				return false;

			canInsert = m_InventoryManager.CanInsertItemInActualStorage(selectedItemComponent.GetOwner(), currentStorage, -1);
		}
		else
		{
			SCR_ArsenalInventorySlotUI arsenalInventorySlotUI;
		
			if (m_pSelectedSlotUI)
				 arsenalInventorySlotUI = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
			
			if (!m_pSelectedSlotUI || !arsenalInventorySlotUI)
				return false;
			if (!arsenalInventorySlotUI.IsAvailable())
				return false;

			InventoryItemComponent arsenalItemComponent = arsenalInventorySlotUI.GetInventoryItemComponent();
			if (!arsenalItemComponent)
				return false;

			IEntity slotEntity = arsenalItemComponent.GetOwner();
			if (!slotEntity)
				return false;

			BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForItem(slotEntity, EStoragePurpose.PURPOSE_ANY);
			if (!storageTo)
				return false;

			canInsert = m_InventoryManager.CanInsertItemInStorage(slotEntity, storageTo);
		}
		return canInsert;
	}
	
	protected SCR_ResourcePlayerControllerInventoryComponent GetShopInventoryRpcComponent()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;

		return SCR_ResourcePlayerControllerInventoryComponent.Cast(playerController.FindComponent(SCR_ResourcePlayerControllerInventoryComponent));
	}

	protected BaseInventoryStorageComponent GetOpenShopStorage()
	{
		BaseInventoryStorageComponent storage;
		if (m_pStorageLootUI)
			storage = m_pStorageLootUI.GetCurrentNavigationStorage();

		if (!storage && m_aOpenedStoragesUI)
			storage = GetOpenArsenalStorage();

		if (!storage || !storage.GetOwner() || !storage.GetOwner().FindComponent(RG_ShopComponent))
			return null;

		return storage;
	}

	protected bool RequestShopBuy(SCR_ArsenalInventorySlotUI arsenalSlot, BaseInventoryStorageComponent storageTo)
	{
		if (!arsenalSlot || !storageTo)
			return false;

		IEntity arsenalEntity = SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalSlot);
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
		SCR_ResourcePlayerControllerInventoryComponent rpcComponent = GetShopInventoryRpcComponent();
		if (!resourceComponent || !rpcComponent)
			return false;

		rpcComponent.RpcAsk_RGShopRequestItem(Replication.FindItemId(resourceComponent), Replication.FindItemId(storageTo), arsenalSlot.GetItemResource());
		return true;
	}

	protected bool RequestShopSell(SCR_InventorySlotUI itemSlot, BaseInventoryStorageComponent shopStorage)
	{
		if (!itemSlot || !shopStorage)
			return false;

		InventoryItemComponent itemComponent = itemSlot.GetInventoryItemComponent();
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(shopStorage.GetOwner());
		SCR_ResourcePlayerControllerInventoryComponent rpcComponent = GetShopInventoryRpcComponent();
		if (!itemComponent || !resourceComponent || !rpcComponent)
			return false;

		rpcComponent.RpcAsk_RGShopSellItem(Replication.FindItemId(resourceComponent), Replication.FindItemId(itemComponent));
		return true;
	}

	override protected void Action_Drop()
	{
		if (!IsShop() || !CanInsertItem(true))
		{
			super.Action_Drop();
			return;
		}

		if (m_eShopDealDirection == RG_EShopDealDirection.BUY)
		{
			MoveBetweenFromVicinity();
		}
		else if (m_eShopDealDirection == RG_EShopDealDirection.SELL)
		{
			MoveBetweenToVicinity_VirtualArsenal();
		}

		ResetHighlightsOnAvailableStorages();
	}

	override protected void MoveBetweenFromVicinity()
	{
		if (IsShop() &&  CanInsertItem(false))
		{
			m_eShopDealDirection = RG_EShopDealDirection.BUY;
			SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
			InventoryItemComponent itemComponent = arsenalSlot.GetInventoryItemComponent();
			BaseInventoryStorageComponent storageTo = m_InventoryManager.FindStorageForItem(itemComponent.GetOwner(), EStoragePurpose.PURPOSE_ANY);
			RequestShopBuy(arsenalSlot, storageTo);
			return;
		}
		super.MoveBetweenFromVicinity();

	}
	protected bool MoveToVicinityIsShop()
	{
		BaseInventoryStorageComponent storageComponent;
		if (m_pStorageLootUI)
			storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();

		RG_ShopComponent shopComponent;
 		if (!storageComponent && m_aOpenedStoragesUI)
			storageComponent = GetOpenArsenalStorage();
		
		if (!storageComponent || !IsStorageArsenal(storageComponent))	
			return false;
		
		
		//! Perform refund logic.
		IEntity arsenalEntity = storageComponent.GetOwner();
		if (arsenalEntity)		
			shopComponent = RG_ShopComponent.Cast(arsenalEntity.FindComponent(RG_ShopComponent));
		
		return shopComponent != null;
		
	}
	override protected bool MoveBetweenToVicinity_VirtualArsenal()
	{
		if(MoveToVicinityIsShop())
		{
			m_eShopDealDirection = RG_EShopDealDirection.SELL;
			RequestShopSell(SCR_InventorySlotUI.Cast(m_pSelectedSlotUI), GetOpenShopStorage());
			return true;
		}
		return super.MoveBetweenToVicinity_VirtualArsenal();
	}
	void SetMoneyText()
	{
		TextWidget moneyText = TextWidget.Cast( m_widget.FindAnyWidget("RGmoneyText"));
		RG_MoneyComponent moneyComp = RG_MoneyComponent.Cast(m_Player.FindComponent(RG_MoneyComponent));
		if (!moneyText || !moneyComp)
			return;

		string moneyFormat = string.Format("%1$", string.ToString(moneyComp.GetMoney()));
		moneyText.SetText(moneyFormat);
		
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		RG_MoneyComponent.GetOnMoneyChanged().Insert(SetMoneyText);
		SetMoneyText();
		m_SellPriceUIInfo = RG_PriceItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sSellPriceUIInfoPrefab));
		m_BuyPriceUIInfo = RG_PriceItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sBuyPriceUIInfoPrefab));
	}

	override void OnMenuClose()
	{
		RG_MoneyComponent.GetOnMoneyChanged().Remove(SetMoneyText);
		super.OnMenuClose();
	}

}

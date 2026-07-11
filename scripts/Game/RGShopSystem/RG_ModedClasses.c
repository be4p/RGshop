[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
modded class SCR_ArsenalItem
{
	[Attribute("10", desc: "", params: "0 inf 1")]
	protected int m_SellValue;
	int GetSellValue()
	{
		return m_SellValue;
	}
}

modded class SCR_ArsenalInventorySlotUI
{
	override string SetSlotSize()
	{
		if(IsShop())
		{
			string slotLayout = "{3B35DE9811F55E29}UI/layouts/Menus/Inventory/RGshopInventoryItemSlot.layout";
        
			switch ( m_Attributes.GetItemSize() ) 
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
	
			return slotLayout;
		}
		return super.SetSlotSize();
	}
	bool IsShop()
	{
		IEntity storageEnts = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		if(!storageEnts)
			return false;
		RG_ShopComponent shopComponent = RG_ShopComponent.Cast(storageEnts.FindComponent(RG_ShopComponent));
		if(shopComponent)
			return true;
		
		return false;
	}
	override float GetTotalResources()
	{
		
		if (IsShop())
		{
			m_fSupplyCost = 0;
			MyWorldManagerComponent manager = MyWorldManagerComponent.GetInstance();
			
			Resource itemResource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
			ResourceName targetResource = itemResource.GetResource().GetResourceName();
	
			foreach (SCR_ArsenalItemStandalone item : manager.arsenalItems)
			{
				if (!item)
					continue;
		
				Resource resource = item.GetItemResource();
				
				if (!resource)
					continue;
		
				if (resource.GetResource().GetResourceName() != targetResource)
					continue;
		
				m_fSupplyCost = item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
				return m_fSupplyCost;
				
			}
		}
		return super.GetTotalResources();
	}

	override void UpdateTotalResources(float totalResources)
	{
		if(IsShop())
		{
			SetItemAvailability(true);
			m_CostResourceHolder.SetVisible(true);
			string resText = string.Format("%1$",totalResources.ToString());
			m_CostResourceHolderText.SetText(resText);
			return;
		}
		super.UpdateTotalResources(totalResources);
	}
}

modded class SCR_InventoryMenuUI
{	
	protected bool m_pToShop;
	protected bool m_pFromShop;
	const protected ResourceName m_sPriceUIInfoPrefab = "{B15C5823BAF921FF}RGprice_ItemHint.conf";
	protected ref SCR_SupplyCostItemHintUIInfo 	m_PriceCostUIInfo;

	int GetPrice()
	{
		MyWorldManagerComponent manager = MyWorldManagerComponent.GetInstance();
		ResourceName targetResource = m_pFocusedSlotUI.GetItemResource();
		foreach (SCR_ArsenalItemStandalone item :  manager.arsenalItems)
		{
			if (!item)
				continue;
	
			Resource resource = item.GetItemResource();
			
			if (!resource)
				continue;
	
			if (resource.GetResource().GetResourceName() != targetResource)
				continue;
	
			return item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);	
		}
		return 0;
	}
	override void GetGeneralItemHintsInfos(out notnull array<SCR_InventoryItemHintUIInfo> hintsInfo)
	{			
		SCR_ArsenalInventorySlotUI arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
		if (!arsenalSlot)
		{
			m_PriceCostUIInfo.SetSupplyCost(GetPrice());
			hintsInfo.InsertAt(m_PriceCostUIInfo, 0);
			super.GetGeneralItemHintsInfos(hintsInfo);
			foreach (SCR_InventoryItemHintUIInfo hintUIInfo : hintsInfo)
			{
				SCR_SupplyCostItemHintUIInfo priceUIInfo = SCR_SupplyCostItemHintUIInfo.Cast(hintUIInfo);
				/*
				if (!hintUIInfo.CanBeShown(item, focusedSlot))
					continue;
				
				Widget createdWidget = workspace.CreateWidgets(m_sHintLayout, m_wHintWidget);
				if (!createdWidget)
					return;
				
				hintUIInfo.SetItemHintNameTo(item, RichTextWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintText")));
				hintUIInfo.SetIconTo(ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon")));
				*/
			}
			return;
		}
		super.GetGeneralItemHintsInfos(hintsInfo);
	
	}
	protected bool IsShop()
	{
		IEntity entityShop;
		RG_ShopComponent shopComponent;
		
		SCR_ArsenalInventorySlotUI arsenalUi = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		if(arsenalUi)
		{
			entityShop = arsenalUi.GetArsenalResourceComponent().GetOwner();
			m_pFromShop = true;
			m_pToShop = false;
		}
		else
		{
			SCR_InventorySlotUI lastFocus = m_pActiveHoveredStorageUI.GetLastFocusedSlot();
			SCR_ResourceComponent res;
			SCR_ArsenalInventorySlotUI arsenalSlot;
			if(lastFocus)
				arsenalSlot = SCR_ArsenalInventorySlotUI.Cast(lastFocus);
			if(arsenalSlot)
				res = arsenalSlot.GetArsenalResourceComponent();
			if(res)
				entityShop = res.GetOwner();
			m_pToShop = true;
			m_pFromShop = false;
		}
		if(entityShop)
			shopComponent = RG_ShopComponent.Cast(entityShop.FindComponent(RG_ShopComponent));
		if(shopComponent)
		{
			return true;
		}
		else
		{
			m_pToShop = false;
			m_pFromShop = false;
			return false;
		}
		
	}
	
	protected bool CanInsertItem(bool dragged)
	{
		bool canInsert = false;
		if(dragged)
		{
			BaseInventoryStorageComponent currentStorage = m_pActiveHoveredStorageUI.GetCurrentNavigationStorage();
			if(!currentStorage)
				return false;
			IEntity storageEntity = currentStorage.GetOwner();
			if(m_pFromShop)
			{
				RG_ShopComponent shopComponent = RG_ShopComponent.Cast(storageEntity.FindComponent(RG_ShopComponent));
				if(shopComponent)
					return false;
			}

			canInsert = m_InventoryManager.CanInsertItemInActualStorage(m_pSelectedSlotUI.GetInventoryItemComponent().GetOwner(),currentStorage,-1);
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
			IEntity slotEntity									= arsenalInventorySlotUI.GetInventoryItemComponent().GetOwner();
			BaseInventoryStorageComponent storageTo				= m_InventoryManager.FindStorageForItem(slotEntity, EStoragePurpose.PURPOSE_ANY);
			IEntity arsenalEntity								= SCR_InventoryStorageBaseUI.ARSENAL_SLOT_STORAGES.Get(arsenalInventorySlotUI);
			SCR_ResourceComponent resourceComponent				= SCR_ResourceComponent.FindResourceComponent(arsenalEntity);
			ResourceName resourceName							= arsenalInventorySlotUI.GetItemResource();
			SCR_ResourcePlayerControllerInventoryComponent resourceInventoryComponent = SCR_ResourcePlayerControllerInventoryComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_ResourcePlayerControllerInventoryComponent));

			canInsert = m_InventoryManager.CanInsertItemInStorage(slotEntity, storageTo);
		}
		return canInsert;
	}
	
	protected bool ShopDeal()
	{
		SCR_InventorySlotUI item = SCR_InventorySlotUI.Cast(m_DraggedSlot);
		if(!item)
			item = SCR_InventorySlotUI.Cast(m_pFocusedSlotUI);
		
		RG_MoneyComponent moneyComp = RG_MoneyComponent.Cast(m_Player.FindComponent(RG_MoneyComponent));
		int playerMoneyValue = moneyComp.money;
		bool dealError = false;
		BaseInventoryStorageComponent aa = item.GetAsStorage();
		array<IEntity> attachments = {};
		aa.GetAll(attachments);
		Print(attachments);
		InventoryItemComponent ia = item.GetInventoryItemComponent();
		IEntity as = ia.GetOwner();
		EntityPrefabData a1a = as.GetPrefabData();
		BaseContainer daf = a1a.GetPrefab();
		BaseContainer ad = daf.GetAncestor();
		Print(daf.GetName());
		Print(ad.GetName());
		if(m_pFromShop)
		{
			SCR_ArsenalInventorySlotUI arItem = SCR_ArsenalInventorySlotUI.Cast(item);
			int itemPrice = arItem.GetItemSupplyCost();
			int result = playerMoneyValue - itemPrice;
			if(result >= 0)
				moneyComp.SetMoney(result);		
			else
				dealError = true;
		}
		if(m_pToShop)
		{
			MyWorldManagerComponent manager = MyWorldManagerComponent.GetInstance();
			ResourceName targetResource = item.GetItemResource();
			int sellPrice = 0;
			foreach (SCR_ArsenalItemStandalone itemS :  manager.arsenalItems)
			{
				if (!itemS)
					continue;
				
				Resource resource = itemS.GetItemResource();
				
				if (!resource)
					continue;
		
				if (resource.GetResource().GetResourceName() != targetResource)
					continue;
		
				sellPrice = itemS.GetSellValue();
				
				if(sellPrice == 0)
				{
					int supplyCost = itemS.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
					sellPrice = supplyCost / 2;
					
				}			
			}
			if(sellPrice != 0)
			{
				int result = playerMoneyValue + sellPrice;
				moneyComp.SetMoney(result);		
			}
			else
				dealError = true;
			 
			
		}
		if(dealError)
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INV_DROP_ERROR);
			return false; 
		}
		SetMoneyText();
		return true; 
	}
	override protected void Action_Drop()
	{

		if (IsShop() && CanInsertItem(true))
		{	
			if(ShopDeal())
			{
				super.Action_Drop();
			}
			return;
		}
	
		super.Action_Drop();
	}
	override protected void MoveBetweenFromVicinity()
	{
		if (IsShop() &&  CanInsertItem(false))
		{
			m_pToShop = false;
			m_pFromShop = true;
			if(ShopDeal())
				super.MoveBetweenFromVicinity();
			return;
		}
		super.MoveBetweenFromVicinity();

	}
	protected bool MoveToVicinityIsShop()
	{
		BaseInventoryStorageComponent storageComponent = m_pStorageLootUI.GetCurrentNavigationStorage();
		RG_ShopComponent shopComponent;
 		if (!storageComponent && m_aOpenedStoragesUI)
			storageComponent = GetOpenArsenalStorage();
		
		if (!storageComponent || !IsStorageArsenal(storageComponent))	
			return false;
		
		
		//! Perform refund logic.
		IEntity arsenalEntity = storageComponent.GetOwner();
		if (arsenalEntity)		
			shopComponent = RG_ShopComponent.Cast(arsenalEntity.FindComponent(RG_ShopComponent));
		
		if(shopComponent)
			return true;
		else
			return false;
		
	}
	override protected bool MoveBetweenToVicinity_VirtualArsenal()
	{
		if(MoveToVicinityIsShop())
		{
			m_pToShop = true;
			m_pFromShop = false;
			if(ShopDeal())
				return super.MoveBetweenToVicinity_VirtualArsenal();
			else
				return false;

		}
		return super.MoveBetweenToVicinity_VirtualArsenal();
	}
	void SetMoneyText()
	{
		TextWidget moneyText = TextWidget.Cast( m_widget.FindAnyWidget("RGmoneyText"));
		RG_MoneyComponent moneyComp = RG_MoneyComponent.Cast(m_Player.FindComponent(RG_MoneyComponent));

		string moneyFormat = string.Format("%1$",string.ToString(moneyComp.money));
		moneyText.SetText(moneyFormat);
		
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SetMoneyText();
		m_PriceCostUIInfo = SCR_SupplyCostItemHintUIInfo.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sPriceUIInfoPrefab));
	}

}
modded class SCR_InventoryItemInfoUI
{
	override void SetItemHints(InventoryItemComponent item, array<SCR_InventoryItemHintUIInfo> itemHintArray, SCR_InventorySlotUI focusedSlot)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//~ Clear existing hints if any
		Widget hintChild = m_wHintWidget.GetChildren();
		Widget deleteHint;
		while(hintChild)
		{
			deleteHint = hintChild;
			hintChild = deleteHint.GetSibling();
		
			delete deleteHint;
		}
		
		foreach (SCR_InventoryItemHintUIInfo hintUIInfo : itemHintArray)
		{
			if (!hintUIInfo.CanBeShown(item, focusedSlot))
				continue;
			
			Widget createdWidget = workspace.CreateWidgets(m_sHintLayout, m_wHintWidget);
			if (!createdWidget)
				return;
			
			hintUIInfo.SetItemHintNameTo(item, RichTextWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintText")));
			ImageWidget test =  ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon"));
			test.SetVisible(false);
			test.SetEnabled(false);
			//hintUIInfo.SetIconTo(ImageWidget.Cast(createdWidget.FindAnyWidget("ItemInfo_hintIcon")));
		}

		SetItemHints(item, false);
	}
}

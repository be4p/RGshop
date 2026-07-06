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
		if (!IsShop())
			return super.GetTotalResources();
	
		SCR_ArsenalManagerComponent arsenalManager;
		SCR_ArsenalManagerComponent.GetArsenalManager(arsenalManager);
	
		Resource itemResource = Resource.Load(m_pItem.GetOwner().GetPrefabData().GetPrefabName());
		if (!itemResource)
			return super.GetTotalResources();
	
		IEntity storageEntity = GetStorageUI().GetCurrentNavigationStorage().GetOwner();
		if (!storageEntity)
			return super.GetTotalResources();
	
		SCR_ArsenalComponent arsenalComponent = SCR_ArsenalComponent.Cast(storageEntity.FindComponent(SCR_ArsenalComponent));
		if (!arsenalComponent)
			return super.GetTotalResources();
	
		SCR_ArsenalItemListConfig arsenalConfig = arsenalComponent.GetOverwriteArsenalConfig();
		if (!arsenalConfig)
			return super.GetTotalResources();
	
		array<ref SCR_ArsenalItemStandalone> arsenalItems = {};
		if (!arsenalConfig.GetArsenalItems(arsenalItems))
			return super.GetTotalResources();
	
		ResourceName targetResource = itemResource.GetResource().GetResourceName();
	
		foreach (SCR_ArsenalItemStandalone item : arsenalItems)
		{
			if (!item)
				continue;
	
			Resource resource = item.GetItemResource();
			if (!resource)
				continue;
	
			if (resource.GetResource().GetResourceName() != targetResource)
				continue;
	
			int supplyCost = item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
			m_fSupplyCost = supplyCost;
			return supplyCost;
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
	protected SCR_ArsenalItemListConfig config;
	array<ref SCR_ArsenalItemStandalone> arsenalItemsGG;
	
	override void ShowItemInfo( string sName = "", string sDescr = "", float sWeight = 0.0, SCR_InventoryUIInfo uiInfo = null )
	{
		ResourceName targetResource = m_pFocusedSlotUI.GetItemResource();
		foreach (SCR_ArsenalItemStandalone item : arsenalItemsGG)
		{
			if (!item)
				continue;
	
			Resource resource = item.GetItemResource();
			
			if (!resource)
				continue;
	
			if (resource.GetResource().GetResourceName() != targetResource)
				continue;
	
			int supplyCost = item.GetSupplyCost(SCR_EArsenalSupplyCostType.DEFAULT, false);
			Print(supplyCost);
			
		}
		if ( !m_pItemInfo )
		{
			//Widget parent = m_widget.FindAnyWidget( "SoldierInfo" );
			Widget infoWidget = GetGame().GetWorkspace().CreateWidgets(ITEM_INFO, m_widget);
			if ( !infoWidget )
				return;

			infoWidget.AddHandler( new SCR_InventoryItemInfoUI() );
			m_pItemInfo = SCR_InventoryItemInfoUI.Cast( infoWidget.FindHandler( SCR_InventoryItemInfoUI ) );
		}

		if( !m_pItemInfo )
			return;

		Widget w = m_pFocusedSlotUI.GetButtonWidget();

		m_pItemInfo.Show( 0.6, w, true );
		m_pItemInfo.SetName( sName );
		m_pItemInfo.SetDescription( sDescr );
		
		m_pItemInfo.SetWeight( sWeight );
		if (uiInfo && uiInfo.IsIconVisible())
			m_pItemInfo.SetIcon(uiInfo.GetIconPath(), uiInfo.GetIconColor());
		else
			m_pItemInfo.ShowIcon(false);
		
		array<SCR_InventoryItemHintUIInfo> hintsInfo = {};
		
		//~ Add hints
		if (uiInfo)
			uiInfo.GetItemHintArray(hintsInfo);
		
		//~ Add general hints that are not found on the item but are more generic depending where the item is (eg: arsenal) and the components on the item
		//~ These hints cover a wide arrange of items so that each item does not need to be set up individually
		GetGeneralItemHintsInfos(hintsInfo);

		//~ If has hints show them
		if (!hintsInfo.IsEmpty())
			m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent(), hintsInfo, m_pFocusedSlotUI);
		else
			m_pItemInfo.SetItemHints(m_pFocusedSlotUI.GetInventoryItemComponent());
			
		//~ Create Identity UI
		m_pItemInfo.CreateIdentityUI(SCR_IdentityInventoryItemComponent.Cast(m_pFocusedSlotUI.GetInventoryItemComponent()));

		UpdateItemInfoPosition();
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
			entityShop = SCR_ArsenalInventorySlotUI.Cast(lastFocus).GetArsenalResourceComponent().GetOwner();
			m_pToShop = true;
			m_pFromShop = false;
		}

	
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
			RG_ShopComponent shopComponent = RG_ShopComponent.Cast(storageEntity.FindComponent(RG_ShopComponent));
			if(shopComponent)
				return false;
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
		SCR_ArsenalInventorySlotUI item = SCR_ArsenalInventorySlotUI.Cast(m_pFocusedSlotUI);
		if(!item)
			item = SCR_ArsenalInventorySlotUI.Cast(m_pSelectedSlotUI);
		RG_MoneyComponent moneyComp = RG_MoneyComponent.Cast(m_Player.FindComponent(RG_MoneyComponent));
		int playerMoneyValue = moneyComp.money;
		bool dealError = false;
		
		if(m_pFromShop)
		{
			int itemPrice = item.GetItemSupplyCost();
			int result = playerMoneyValue - itemPrice;
			if(result >= 0)
				moneyComp.SetMoney(result);		
			else
				dealError = true;
		}
		if(m_pToShop)
		{
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
				super.Action_Drop();
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
			Print("da");
			return false;//если его нету в списке продаж для этого магаза 
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
	void SetConfig()
	{
		RG_MoneyComponent moneyComp = RG_MoneyComponent.Cast(m_Player.FindComponent(RG_MoneyComponent));
		config = moneyComp.getConfig();

		config.GetArsenalItems(arsenalItemsGG);
		
		
	}

	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		SetMoneyText();
		SetConfig();
	}

}

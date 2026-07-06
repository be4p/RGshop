/*
class SCR_TestOpenTwoStoragesAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.ResourceNamePicker, "Storage prefab Sell", "et")]
	protected ResourceName m_StoragePrefabSell;
	[Attribute("", UIWidgets.ResourceNamePicker, "Storage prefab Buy", "et")]
	protected ResourceName m_StoragePrefabBuy;

	protected BaseInventoryStorageComponent m_StorageShop;
	protected BaseInventoryStorageComponent m_StorageSell;
	protected BaseInventoryStorageComponent m_StorageBuy;
	
	protected IEntity boxSell;
	protected IEntity boxBuy;
	

	override bool GetActionNameScript(out string outName)
	{
		outName = "Open Two Test Storages";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!user)
			return false;

		SCR_InventoryStorageManagerComponent manager =
			SCR_InventoryStorageManagerComponent.Cast(
				user.FindComponent(SCR_InventoryStorageManagerComponent)
			);

		return manager != null;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_InventoryStorageManagerComponent manager =
			SCR_InventoryStorageManagerComponent.Cast(
				pUserEntity.FindComponent(SCR_InventoryStorageManagerComponent));
		
		RG_SpawnShopStorageComponent spawnComponent = 
			RG_SpawnShopStorageComponent.Cast(
				pOwnerEntity.FindComponent(RG_SpawnShopStorageComponent));
		
		IEntity shopEntity = spawnComponent.ShopEntity;
		
		m_StorageShop = BaseInventoryStorageComponent.Cast(
			shopEntity.FindComponent(BaseInventoryStorageComponent));
		
		RG_ShopManagerComponent shopManager = 
			RG_ShopManagerComponent.Cast(
				pUserEntity.FindComponent(RG_ShopManagerComponent));
		
		if (!manager)
			return;
		
		shopManager.GetShopVariables(boxSell,boxBuy);
		
		if(!boxSell)
		{
			Print("Spawn");
			boxSell = SpawnStorage(m_StoragePrefabSell,pUserEntity, "0 0 0");
			boxBuy = SpawnStorage(m_StoragePrefabBuy,pUserEntity, "0 0 0");
			shopManager.SetShopVariables(boxSell,boxBuy);
		}


		if (!boxSell || !boxBuy)
			return;
		
		m_StorageSell = BaseInventoryStorageComponent.Cast(
			boxSell.FindComponent(BaseInventoryStorageComponent)
		);
		
		m_StorageBuy = BaseInventoryStorageComponent.Cast(
			boxBuy.FindComponent(BaseInventoryStorageComponent)
		);

		if (!m_StorageSell || !m_StorageBuy)
			return;
		SCR_CharacterInventoryStorageComponent test = SCR_CharacterInventoryStorageComponent.Cast(
			pUserEntity.FindComponent(SCR_CharacterInventoryStorageComponent));
		
		manager.OpenInventory();
	
		GetGame().GetCallqueue().CallLater(OpenStoragesDelayed, 100, false);
	}

	protected void OpenStoragesDelayed()
	{
		SCR_InventoryMenuUI menu = SCR_InventoryMenuUI.GetInventoryMenu();

		if (!menu)
		{
			Print("Inventory menu not found");
			return;
		}

		menu.RG_SetShopMode(true);
		menu.RG_OpenStorageAsContainer(m_StorageShop,"OpenedStorages",6,6);
		menu.RG_OpenStorageAsContainer(m_StorageSell,"RGopeneStorages",6,4);
		menu.RG_OpenStorageAsContainer(m_StorageBuy,"RGopeneStorages",6,4);
		//menu.ShowStoragesList();
		//menu.ShowAllStoragesInList();
		//menu.RG_SetShopMode(true);
	}

	protected IEntity SpawnStorage(ResourceName m_StoragePrefab,IEntity user, vector offset)
	{
		if (m_StoragePrefab.IsEmpty())
		{
			Print("Storage prefab is empty");
			return null;
		}

		Resource resource = Resource.Load(m_StoragePrefab);

		if (!resource)
		{
			Print("Cannot load storage prefab: " + m_StoragePrefab);
			return null;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector mat[4];
		user.GetWorldTransform(mat);

		mat[3] = mat[3] + offset;
		params.Transform = mat;

		IEntity entity = GetGame().SpawnEntityPrefab(
			resource,
			GetGame().GetWorld(),
			params
		);

		if (!entity)
			Print("Failed to spawn storage prefab");

		return entity;

	}
}
*/
class MyWorldManagerComponentClass:ScriptComponentClass
{
}
class MyWorldManagerComponent : ScriptComponent
{
	
	[Attribute(params: "conf")]
	protected ResourceName m_sConfig;
	ref array<ref SCR_ArsenalItemStandalone> arsenalItems= new array<ref SCR_ArsenalItemStandalone>;
	
	static MyWorldManagerComponent s_Instance;

	override void OnPostInit(IEntity owner)
	{
		s_Instance = this;

		if (m_sConfig.IsEmpty())
		{
			Print("[RG Shop] Shop config is not assigned", LogLevel.ERROR);
			return;
		}

		Resource resource = Resource.Load(m_sConfig);
		if (!resource || !resource.IsValid())
		{
			PrintFormat("[RG Shop] Failed to load shop config: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		BaseResourceObject resourceObject = resource.GetResource();
		if (!resourceObject)
		{
			PrintFormat("[RG Shop] Shop config has no resource object: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		SCR_ArsenalItemListConfig arsenalConfig = SCR_ArsenalItemListConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resourceObject.ToBaseContainer()));
		if (!arsenalConfig)
		{
			PrintFormat("[RG Shop] Invalid shop config type: %1", m_sConfig, level: LogLevel.ERROR);
			return;
		}

		arsenalItems.Clear();
		arsenalConfig.GetArsenalItems(arsenalItems);
	}

	static MyWorldManagerComponent GetInstance()
	{
		return s_Instance;
	}
}

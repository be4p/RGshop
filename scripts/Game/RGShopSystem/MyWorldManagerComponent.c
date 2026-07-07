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
		
		Resource resource = Resource.Load(m_sConfig);

		SCR_ArsenalItemListConfig arsenalConfig = SCR_ArsenalItemListConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));

		arsenalConfig.GetArsenalItems(arsenalItems);

		
	}

	static MyWorldManagerComponent GetInstance()
	{
		return s_Instance;
	}
}
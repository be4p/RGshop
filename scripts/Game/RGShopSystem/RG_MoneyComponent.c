class RG_MoneyComponentClass : ScriptComponentClass
{
}
class RG_MoneyComponent: ScriptComponent
{
	[Attribute(params: "conf")]
	protected ResourceName m_sConfig;
	ref SCR_ArsenalItemListConfig arsenalConfig;
	int money;
	override void OnPostInit(IEntity owner)
	{
		money = 2010;
		arsenalConfig = new SCR_ArsenalItemListConfig;
	    Resource resource = Resource.Load(m_sConfig);

		arsenalConfig = SCR_ArsenalItemListConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(resource.GetResource().ToBaseContainer()));
		
		

	}
	void SetMoney(int value)
	{
		money = value;
	}
	SCR_ArsenalItemListConfig getConfig()
	{
		return arsenalConfig;
	}

}
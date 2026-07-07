class RG_MoneyComponentClass : ScriptComponentClass
{
}
class RG_MoneyComponent: ScriptComponent
{
	int money;
	override void OnPostInit(IEntity owner)
	{
		money = 3000;
	}
	void SetMoney(int value)
	{
		money = value;
	}
}
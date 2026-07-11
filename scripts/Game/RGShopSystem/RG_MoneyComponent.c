class RG_MoneyComponentClass : ScriptComponentClass
{
}
class RG_MoneyComponent: ScriptComponent
{
	[Attribute("3000", desc: "Starting player money", params: "0 inf 1")]
	protected int m_iStartingMoney;

	protected int m_iMoney;

	override void OnPostInit(IEntity owner)
	{
		m_iMoney = m_iStartingMoney;
	}

	int GetMoney()
	{
		return m_iMoney;
	}

	void SetMoney(int value)
	{
		if (value < 0)
			value = 0;

		m_iMoney = value;
	}
}

class RG_MoneyComponentClass : ScriptComponentClass
{
}
class RG_MoneyComponent: ScriptComponent
{
	protected static ref ScriptInvoker s_OnMoneyChanged = new ScriptInvoker();

	[Attribute("3000", desc: "Starting player money", params: "0 inf 1")]
	protected int m_iStartingMoney;

	[RplProp(onRplName: "OnMoneyChanged", condition: RplCondition.OwnerOnly)]
	protected int m_iMoney;

	override void OnPostInit(IEntity owner)
	{
		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rplComponent && rplComponent.IsProxy())
			return;

		m_iMoney = m_iStartingMoney;
	}

	int GetMoney()
	{
		return m_iMoney;
	}

	bool TrySpendMoney_S(int value)
	{
		if (value < 0 || m_iMoney < value)
			return false;

		m_iMoney -= value;
		Replication.BumpMe();
		s_OnMoneyChanged.Invoke();
		return true;
	}

	void AddMoney_S(int value)
	{
		if (value <= 0)
			return;

		m_iMoney += value;
		Replication.BumpMe();
		s_OnMoneyChanged.Invoke();
	}

	protected void OnMoneyChanged()
	{
		s_OnMoneyChanged.Invoke();
	}

	static ScriptInvoker GetOnMoneyChanged()
	{
		return s_OnMoneyChanged;
	}
}

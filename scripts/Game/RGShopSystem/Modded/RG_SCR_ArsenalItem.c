[BaseContainerProps(configRoot: true), BaseContainerCustomCheckIntTitleField("m_bEnabled", "Arsenal Data", "DISABLED - Arsenal Data", 1)]
modded class SCR_ArsenalItem
{
	[Attribute("0", desc: "Optional sell price. Leave 0 to use half of the buy price.", params: "0 inf 1")]
	protected int m_SellValue;

	int GetSellValue()
	{
		return m_SellValue;
	}
}

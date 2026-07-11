[BaseContainerProps(configRoot: true)]
class RG_PriceItemHintUIInfo : SCR_SupplyCostItemHintUIInfo
{
	override bool SetIconTo(ImageWidget imageWidget)
	{
		if (!imageWidget)
			return false;

		imageWidget.SetVisible(false);
		imageWidget.SetEnabled(false);
		return true;
	}
}

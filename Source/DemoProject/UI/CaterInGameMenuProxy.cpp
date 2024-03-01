#include "CaterInGameMenuProxy.h"

void UCaterInGameMenuProxy::Construct(UCaterGameInstance* CaterGameInstance, ULocalPlayer* LocalPlayer)
{
	check(CaterGameInstance);
	GameInstance = CaterGameInstance;
	PlayerOwner = LocalPlayer;
}

void UCaterInGameMenuProxy::OnPauseGame()
{
}

void UCaterInGameMenuProxy::OnResumeGame()
{
}

void UCaterInGameMenuProxy::OnMainMenu()
{
}

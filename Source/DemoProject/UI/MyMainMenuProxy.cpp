#include "MyMainMenuProxy.h"
#include "MyGameInstance.h"
#include "Engine/LocalPlayer.h"


void UMyMainMenuProxy::Construct(TWeakObjectPtr<UMyGameInstance> _GameInstance, TWeakObjectPtr<ULocalPlayer> _PlayerOwner)
{
	check(_GameInstance.IsValid());

	GameInstance = _GameInstance;
	PlayerOwner = _PlayerOwner;

	
}

void UMyMainMenuProxy::OnStartGame()
{
}

void UMyMainMenuProxy::OnQuitGame()
{
}

#include "CaterMainMenuProxy.h"
#include "CaterGameInstance.h"
#include "Engine/LocalPlayer.h"


void UCaterMainMenuProxy::Construct(TWeakObjectPtr<UCaterGameInstance> _GameInstance, TWeakObjectPtr<ULocalPlayer> _PlayerOwner)
{
	check(_GameInstance.IsValid());

	GameInstance = _GameInstance;
	PlayerOwner = _PlayerOwner;

	
}

void UCaterMainMenuProxy::OnStartGame()
{
	//log
	UE_LOG(LogTemp, Warning, TEXT("OnStartGame"));
}

void UCaterMainMenuProxy::OnQuitGame()
{
	//log
	UE_LOG(LogTemp, Warning, TEXT("OnQuitGame"));
}

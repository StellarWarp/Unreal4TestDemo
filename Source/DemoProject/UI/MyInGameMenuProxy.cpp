#include "MyInGameMenuProxy.h"

void UMyInGameMenuProxy::Construct(UMyGameInstance* MyGameInstance, ULocalPlayer* LocalPlayer)
{
	check(MyGameInstance);
	GameInstance = MyGameInstance;
	PlayerOwner = LocalPlayer;
}

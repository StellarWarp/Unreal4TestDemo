#pragma once
#include "CoreMinimal.h"
#include "MyGameInstance.h"
#include "MyInGameMenuProxy.generated.h"

UCLASS()
class UMyInGameMenuProxy :public UObject
{
	GENERATED_BODY()

	UMyGameInstance* GameInstance;
	ULocalPlayer* PlayerOwner;
public:
	void Construct(UMyGameInstance* MyGameInstance, ULocalPlayer* LocalPlayer);

};

#pragma once
#include "CoreMinimal.h"
#include "CaterGameInstance.h"
#include "CaterInGameMenuProxy.generated.h"

UCLASS()
class UCaterInGameMenuProxy :public UObject
{
	GENERATED_BODY()

	UCaterGameInstance* GameInstance;
	ULocalPlayer* PlayerOwner;
public:
	void Construct(UCaterGameInstance* CaterGameInstance, ULocalPlayer* LocalPlayer);

	void OnPauseGame();

	void OnResumeGame();

	void OnMainMenu();

};

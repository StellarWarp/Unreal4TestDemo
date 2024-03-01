#pragma once
#include "CaterMainMenuProxy.generated.h"

class ULocalPlayer;
class UCaterGameInstance;

UCLASS()
class UCaterMainMenuProxy : public UObject
{
	GENERATED_BODY()

public:
	void Construct(TWeakObjectPtr<UCaterGameInstance> _GameInstance, TWeakObjectPtr<ULocalPlayer> _PlayerOwner);

	TWeakObjectPtr<UCaterGameInstance> GameInstance;
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;


	UFUNCTION(BlueprintCallable, Category = "CaterMainMenuProxy")
	void OnStartGame();

	UFUNCTION(BlueprintCallable, Category = "CaterMainMenuProxy")
	void OnQuitGame();
};

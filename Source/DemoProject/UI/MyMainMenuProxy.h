#pragma once
#include "MyMainMenuProxy.generated.h"

class ULocalPlayer;
class UMyGameInstance;

UCLASS()
class UMyMainMenuProxy : public UObject
{
	GENERATED_BODY()

public:
	void Construct(TWeakObjectPtr<UMyGameInstance> _GameInstance, TWeakObjectPtr<ULocalPlayer> _PlayerOwner);

	TWeakObjectPtr<UMyGameInstance> GameInstance;
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;


	UFUNCTION(BlueprintCallable, Category = "MyMainMenuProxy")
	void OnStartGame();

	UFUNCTION(BlueprintCallable, Category = "MyMainMenuProxy")
	void OnQuitGame();
};

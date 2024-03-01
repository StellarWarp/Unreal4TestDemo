// Fill out your copyright notice in the Description page of Project Settings.


#include "MySpectatorPawn.h"


// Sets default values
AMySpectatorPawn::AMySpectatorPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMySpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMySpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMySpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}


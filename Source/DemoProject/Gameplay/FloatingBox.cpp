// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingBox.h"


// Sets default values
AFloatingBox::AFloatingBox()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AFloatingBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFloatingBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


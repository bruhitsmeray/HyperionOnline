// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HO_GameMode_Base.generated.h"

/**
 * 
 */
UCLASS()
class HYPERIONONLINE_API AHO_GameMode_Base : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHO_GameMode_Base();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;
};

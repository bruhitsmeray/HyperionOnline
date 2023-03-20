// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HyperionHealthComp.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "HO_Character.generated.h"

UCLASS()
class HYPERIONONLINE_API AHO_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHO_Character();
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Preferences")
	bool bUseBuiltInHealthSystem = true;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components")
	UCameraComponent* Camera;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components|Interaction")
	UPhysicsHandleComponent* PhysicsHandle;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components|Vitals")
	UHyperionHealthComp* Health;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category="Components|Flashlight")
	USpringArmComponent* MainSpring;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components|Flashlight")
	USpotLightComponent* InnerLight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components|Flashlight")
	USpotLightComponent* OuterLight;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Components|Interaction")
	UPrimitiveComponent* HitComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsWalkingV;
	bool IsWalkingH;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Interaction")
	float GrabDistance = 256.0f;
	float OutlineRange = GrabDistance;
	
	// Use the GrabDistance as the OutlineRadius too, if you need it.
	// Also, what the fuck was the OutlineRadius used for anyways?
	// Like- wasn't I using the OutlineRange for the radius too? Huh? O.O

	float ObjectRotation = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Guns")
	FVector GunOffset = FVector(100, 0, -10);

	UFUNCTION(BlueprintCallable, Category = "Character|Interaction")
	void FBeginGrab();
	UFUNCTION(BlueprintCallable, Category="Character|Interaction")
	void FGrabLocation();
	UFUNCTION(BlueprintCallable, Category="Character|Interaction")
	void FStopGrab();
	UFUNCTION(BlueprintCallable, Category="Character|Interaction")
	void ToggleGrab();
	UFUNCTION(BlueprintCallable, Category="Character|Interaction")
	void FRotateObject(float Axis);

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Character|Sensitivity|Mouse")
	float Sensitivity = 0.25f;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Character|Sensitivity|Gamepad")
	float SensitivityZ = 25.0f;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Character|Sensitivity|Gamepad")
	float SensitivityY = 45.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Preferences")
	bool bToggleUse = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Preferences|Gameplay")
	bool bUseSmoothCrouch = true;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Character|Camera")
	float BaseFoV = 90.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Speeds")
	float BaseWalkSpeed = 256.0f;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Character|Speeds")
	float BaseWalkSpeedMultiplier = 2.2f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character|Speeds")
	float BaseCrouchSpeed = 128.0f;

	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void FVerticalMove(float Value);
	UFUNCTION(BlueprintCallable, Category = "Character|Movement")
	void FHorizontalMove(float Value);
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void FVerticalLook(float Axis);
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void FVerticalLookOnController(float Axis);
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void FHorizontalLook(float Axis);
	UFUNCTION(BlueprintCallable, Category = "Character|Camera")
	void FHorizontalLookOnController(float Axis);

	UFUNCTION(BlueprintCallable, Category="Character|Movement")
	void BeginSprint();
	UFUNCTION(BlueprintCallable, Category="Character|Movement")
	void StopSprint();
	
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Character|Movement")
	void BeginSprintOnServer();
	UFUNCTION(Server, Reliable, BlueprintCallable, Category="Character|Movement")
	void StopSprintOnServer();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Preferences")
	bool bAllowFlash = true;
	
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Status")
	bool bIsCrouching = false;
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Status")
	bool bIsFlashOn = false;
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Status")
	bool bIsHolding = false;
	bool IsMoving();

protected:
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void ConnectToDiscord(const int64 clientID, const bool bRequireDiscordToRun);
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void DisconnectFromDiscord();

	UFUNCTION(BlueprintCallable, Category="Discord")
	static void SetDiscordState(FString State);
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void SetDiscordDetails(FString Details);
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void StartDiscordTimer();
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void EndDiscordTimer();
	UFUNCTION(BlueprintCallable, Category="Discord")
	static void UpdateDiscordActivity();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Camera")
	FName VerticalLook = "MouseY";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Camera")
	FName VerticalLookOnController = "LookRate";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Camera")
	FName HorizontalLook = "MouseX";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Camera")
	FName HorizontalLookOnController = "TurnRate";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Movement")
	FName MoveForward = "MoveForward";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings|Movement")
	FName MoveSide = "MoveRight";
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Character|Bindings")
	FName Grapple = "Grapple";
};

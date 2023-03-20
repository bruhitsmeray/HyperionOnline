// Fill out your copyright notice in the Description page of Project Settings.


#include "HO_Character.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "discord.h"

discord::Core* core{};
discord::Activity activity{};

// Sets default values
AHO_Character::AHO_Character()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanJump = true;
	GetCharacterMovement()->AirControl = 0.5f;
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	GetCharacterMovement()->MaxWalkSpeedCrouched = BaseCrouchSpeed;
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);

	JumpMaxCount = 2;
	JumpMaxHoldTime = 0.1f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->bUsePawnControlRotation = true;
	Camera->SetWorldLocation(FVector(0, 0, 70));

	MainSpring = CreateDefaultSubobject<USpringArmComponent>(TEXT("MainSpring"));
	MainSpring->SetupAttachment(Camera);
	MainSpring->TargetArmLength = 0.0f;
	MainSpring->bEnableCameraRotationLag = true;
	MainSpring->CameraRotationLagSpeed = 12.0f;

	InnerLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("InnerLight"));
	InnerLight->SetupAttachment(MainSpring);
	InnerLight->AttenuationRadius = 2500.0f;
	InnerLight->InnerConeAngle = 16.0f;
	InnerLight->OuterConeAngle = 24.0f;
	InnerLight->SetVisibility(false);

	OuterLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("OuterLight"));
	OuterLight->SetupAttachment(MainSpring);
	OuterLight->Intensity = 2500.0f;
	OuterLight->AttenuationRadius = 2500.0f;
	OuterLight->InnerConeAngle = 32.0f;
	OuterLight->OuterConeAngle = 48.0f;
	OuterLight->SetVisibility(false);
	
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));
}

// Called when the game starts or when spawned
void AHO_Character::BeginPlay(){
	Super::BeginPlay();
	if(bUseBuiltInHealthSystem){
		Health = CreateDefaultSubobject<UHyperionHealthComp>(TEXT("Health"));
		Health->SetIsReplicated(true);
	} else {
		UE_LOG(LogTemp, Warning, TEXT("The built-in Health system is disabled. Unless a custom Health system is used, please enable the built-in Health system."));
	}
}

void AHO_Character::FBeginGrab(){
	UWorld* World = GetWorld();
	if(IsValid(World)){
		FVector CamLoc;
		FRotator CamRot;
		FHitResult HitResult;

		GetController()->GetPlayerViewPoint(CamLoc, CamRot);

		FVector Start = CamLoc;
		FVector End = (CamRot.Vector() * GrabDistance) + Start;

		FCollisionQueryParams TraceParams;
		bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, TraceParams);

		if(bHit){
			UE_LOG(LogTemp, Warning, TEXT("The line trace hit an object."))
			if(IsValid(HitResult.GetComponent()) && HitResult.GetComponent()->IsSimulatingPhysics()){
				HitComponent = HitResult.GetComponent();
				PhysicsHandle->GrabComponentAtLocation(HitComponent, "None", HitComponent->GetComponentLocation());
				HitComponent->SetAngularDamping(PhysicsHandle->AngularDamping);
				bIsHolding = true;
			} else {
				bIsHolding = false;
				UE_LOG(LogTemp, Warning, TEXT("The component that was hit is not available or does not simulate physics."));
			}
		} else {
			bIsHolding = false;
			UE_LOG(LogTemp, Warning, TEXT("No hit registered."));
		}
	} else {
		UE_LOG(LogTemp, Warning, TEXT("World is not available."));
	}
}

void AHO_Character::FGrabLocation(){
	PhysicsHandle->SetTargetLocation(Camera->GetComponentLocation() + (Camera->GetForwardVector() * GrabDistance));
	if(bIsHolding && IsValid(HitComponent)){
		HitComponent->SetRelativeRotation(FRotator(0, ObjectRotation, 0), false, nullptr);
	}
}

void AHO_Character::FStopGrab(){
	if(IsValid(HitComponent)){
		HitComponent->SetAngularDamping(0.0f);
		PhysicsHandle->ReleaseComponent();
		bIsHolding = false;
	}
}

void AHO_Character::ToggleGrab(){
	if(bIsHolding){
		FStopGrab();
	} else {
		FBeginGrab();
	}
}

void AHO_Character::FRotateObject(float Axis){
	if(bIsHolding){
		FRotator SelfRotation = HitComponent->GetRelativeRotation();
		if(Axis < 0.0f) {
			ObjectRotation = SelfRotation.Yaw - 10.0f;
		} else if(Axis > 0.0f) {
			ObjectRotation = SelfRotation.Yaw + 10.0f;
		}
	}
}

void AHO_Character::FVerticalMove(float Value){
	if(Value != 0.0f) {
		AddMovementInput(GetActorForwardVector(), Value);
		IsWalkingV = true;
	} else {
		IsWalkingV = false;
	}
}

void AHO_Character::FHorizontalMove(float Value){
	if(Value != 0.0f) {
		AddMovementInput(GetActorRightVector(), Value);
		IsWalkingH = true;
	} else {
		IsWalkingH = false;
	}
}

void AHO_Character::FVerticalLook(float Axis){
	AddControllerPitchInput(Axis * Sensitivity);
	FGrabLocation();
}

void AHO_Character::FVerticalLookOnController(float Axis){
	AddControllerPitchInput(Axis * SensitivityY * GetWorld()->GetDeltaSeconds());
	FGrabLocation();
}

void AHO_Character::FHorizontalLook(float Axis){
	AddControllerYawInput(Axis * Sensitivity);
	FGrabLocation();
}

void AHO_Character::FHorizontalLookOnController(float Axis){
	AddControllerYawInput(Axis * SensitivityZ * GetWorld()->GetDeltaSeconds());
	FGrabLocation();
}

void AHO_Character::BeginSprint(){
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * BaseWalkSpeedMultiplier;
}

void AHO_Character::StopSprint(){
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
}

void AHO_Character::BeginSprintOnServer_Implementation(){
	BeginSprint();
}

void AHO_Character::StopSprintOnServer_Implementation(){
	StopSprint();
}

bool AHO_Character::IsMoving(){
	if((InputComponent->GetAxisValue(MoveForward) > 0 || InputComponent->GetAxisValue(MoveForward) < 0)
		|| (InputComponent->GetAxisValue(MoveSide) > 0 || InputComponent->GetAxisValue(MoveSide) < 0)) {
		return true;
		}
	return false;
}

void AHO_Character::ConnectToDiscord(const int64 clientID, const bool bRequireDiscordToRun){
	auto result = discord::Core::Create(clientID, bRequireDiscordToRun ? DiscordCreateFlags_Default : DiscordCreateFlags_NoRequireDiscord, &core);
	StartDiscordTimer();
}

void AHO_Character::DisconnectFromDiscord(){
	if(core) {
		activity.SetState("");
		activity.SetDetails("");
		EndDiscordTimer();
		
		delete core;
		core = nullptr;
	}
}

void AHO_Character::SetDiscordState(FString State){
	const char* state = TCHAR_TO_UTF8(*State);
	activity.SetState(state);
	UpdateDiscordActivity();
}

void AHO_Character::SetDiscordDetails(FString Details){
	const char* details = TCHAR_TO_UTF8(*Details);
	activity.SetDetails(details);
	UpdateDiscordActivity();
}

void AHO_Character::StartDiscordTimer(){
	activity.GetTimestamps().SetStart((FDateTime::UtcNow().ToUnixTimestamp()));
	activity.GetTimestamps().SetEnd(0);
	UpdateDiscordActivity();
}

void AHO_Character::EndDiscordTimer(){
	activity.GetTimestamps().SetEnd((FDateTime::UtcNow().ToUnixTimestamp()));
	UpdateDiscordActivity();
}

void AHO_Character::UpdateDiscordActivity(){
	if(core) {
		core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
}

// Called every frame
void AHO_Character::Tick(float DeltaTime){
	Super::Tick(DeltaTime);
	if(core) {
		::core->RunCallbacks();
	}
}

// Called to bind functionality to input
void AHO_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(MoveForward, this, &AHO_Character::FVerticalMove);
	PlayerInputComponent->BindAxis(MoveSide, this, &AHO_Character::FHorizontalMove);
	PlayerInputComponent->BindAxis(VerticalLook, this, &AHO_Character::FVerticalLook);
	PlayerInputComponent->BindAxis(VerticalLookOnController, this,
		&AHO_Character::FVerticalLookOnController);
	PlayerInputComponent->BindAxis(HorizontalLook, this, &AHO_Character::FHorizontalLook);
	PlayerInputComponent->BindAxis(HorizontalLookOnController, this,
		&AHO_Character::FHorizontalLookOnController);

	PlayerInputComponent->BindAxis("RotateInteraction", this, &AHO_Character::FRotateObject);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AHO_Character::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AHO_Character::StopJumping);

	if(bToggleUse)
	{
		PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AHO_Character::ToggleGrab);
	} else {
		PlayerInputComponent->BindAction("Use", IE_Pressed, this, &AHO_Character::FBeginGrab);
		PlayerInputComponent->BindAction("Use", IE_Released, this, &AHO_Character::FStopGrab);
	}
}


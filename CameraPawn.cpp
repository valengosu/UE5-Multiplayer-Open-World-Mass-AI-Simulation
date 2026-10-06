#include "CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"

ACameraPawn::ACameraPawn()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(false);  

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(RootComponent);
	CameraComponent->bUsePawnControlRotation = true;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingPawnMovement"));
	FloatingPawnMovement->MaxSpeed = NormalSpeed;

	// This pawn is possessed manually by PlayerController
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// WASD
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,this, &ACameraPawn::Move);
		// Mouse movement
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered,this, &ACameraPawn::Look);
		// Right mouse button
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Started,this, &ACameraPawn::RightMousePressed);
		EnhancedInputComponent->BindAction(RightMouseAction, ETriggerEvent::Completed, this, &ACameraPawn::RightMouseReleased);
	}
}

void ACameraPawn::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	
		const FRotator ControlRotation = Controller->GetControlRotation();
    	const FVector ForwardDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
    	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
    	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
    
    	AddMovementInput(ForwardDirection, MovementVector.Y);
    	AddMovementInput(RightDirection, MovementVector.X);
	
	
	/*if (HasAuthority() == false)
	{
		ServerMove(MovementVector);
	}*/
}


void ACameraPawn::ServerMove_Implementation(FVector2D MovementVector)
{
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FVector ForwardDirection = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::X);
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ACameraPawn::Look(const FInputActionValue& Value)
{
	if (!bRightMousePressed || Controller == nullptr)
	{
		return;
	}

	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}

void ACameraPawn::RightMousePressed()
{
	bRightMousePressed = true;
}

void ACameraPawn::RightMouseReleased()
{
	bRightMousePressed = false;
}

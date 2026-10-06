// Copyright Epic Games, Inc. All Rights Reserved.


#include "OpenWorldPlayerController.h"

#include "CameraPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "OpenWorld.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AOpenWorldPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogOpenWorld, Error, TEXT("Could not spawn mobile controls widget."));

		}
	}
}

void AOpenWorldPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Started, this, &AOpenWorldPlayerController::ToggleCameraPawn);
		}
		
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AOpenWorldPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AOpenWorldPlayerController::ToggleCameraPawn()
{
	ServerToggleCameraPawn();
}

void AOpenWorldPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (!HasAuthority() || CameraPawn != nullptr)
	{
		return;
	}

	CharacterPawn = GetPawn();
	
	const FVector CameraSpawnLocation  = CharacterPawn->GetActorLocation() + FVector(0.0f, 0.0f, 300.0f);
	const FRotator CameraSpawnRotation = GetControlRotation();FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	CameraPawn = GetWorld()->SpawnActor<ACameraPawn>(CameraPawnClass, CameraSpawnLocation, CameraSpawnRotation, SpawnParams);
}

void AOpenWorldPlayerController::ServerToggleCameraPawn_Implementation()
{
	if (GetPawn() == CharacterPawn)
	{
		Possess(CameraPawn);
	}
	else
	{
		Possess(CharacterPawn);
	}
}

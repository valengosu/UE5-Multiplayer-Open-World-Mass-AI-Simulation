// Copyright Epic Games, Inc. All Rights Reserved.

#include "OpenWorldGameMode.h"
#include "PopulationSubsystem.h"

AOpenWorldGameMode::AOpenWorldGameMode()
{
	// stub
}

void AOpenWorldGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	//GetWorld()->GetSubsystem<UPopulationSubsystem>()->AddPlayer(NewPlayer);
}

void AOpenWorldGameMode::Logout(AController* ExitingController)
{
	Super::Logout(ExitingController);
	//GetWorld()->GetSubsystem<UPopulationSubsystem>()->RemovePlayer((APlayerController*)ExitingController);
}

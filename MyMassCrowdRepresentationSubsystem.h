// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "MyMassCrowdRepresentationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class OPENWORLD_API UMyMassCrowdRepresentationSubsystem : public UMassCrowdRepresentationSubsystem
{
	GENERATED_BODY()
	
public:
	void ReleaseMassRepresentationForRealActorTakeover(FMassEntityManager& InEntityManager, const FMassEntityHandle MassAgent, bool bImmediate = true);
	bool CancelSpawningInternalTakeOver(int16 TemplateActorIndex, FMassActorSpawnRequestHandle& SpawnRequestHandle, const bool bImmediateActorRelease);
	bool ReleaseTemplateActorOrCancelSpawning(const FMassEntityHandle MassAgent, const int16 TemplateActorIndex, AActor* ActorToRelease, FMassActorSpawnRequestHandle& SpawnRequestHandle, bool bImmediate = false);
};

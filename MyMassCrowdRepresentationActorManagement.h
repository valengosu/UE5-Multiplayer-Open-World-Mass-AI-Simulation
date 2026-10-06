// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassCrowdRepresentationActorManagement.h"
#include "MyMassCrowdRepresentationActorManagement.generated.h"

/**
 * 
 */
UCLASS()
class OPENWORLD_API UMyMassCrowdRepresentationActorManagement : public UMassCrowdRepresentationActorManagement
{
	GENERATED_BODY()
	
public:
	virtual void OnPreActorSpawn(const FMassActorSpawnRequestHandle& SpawnRequestHandle, FConstStructView SpawnRequest, TSharedRef<FMassEntityManager> EntityManager) const override;
};

// Fill out your copyright notice in the Description page of Project Settings.

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "MassEntityZoneGraphSpawnPointsGenerator.h"
#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "CoreMinimal.h"
#endif // UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_6
#include "MassEntitySpawnDataGeneratorBase.h"

class AZoneGraphData;

#include "CoreMinimal.h"
#include "PopulationSpawnPointsGenerator.generated.h"

/**
 * 
 */

UCLASS()
class OPENWORLD_API UPopulationSpawnPointsGenerator : public UMassEntityZoneGraphSpawnPointsGenerator
{
	GENERATED_BODY()
	
public:
	virtual void Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes, int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const override;
	
private:
	float NPCDensity = 0.000005f * 1.f;
};

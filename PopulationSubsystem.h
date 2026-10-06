// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZoneGraphTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "PopulationSubsystem.generated.h"

/**
 * 
 */

class AMassSpawner;
class AOpenWorldCharacter;

enum class EPopulationUpdateState
{
	ReadyToScan,
	Counting,
	ResultReady
};

struct FLaneSectionPopulation
{
	FZoneGraphLaneSection Section;
	int Count;
};

UCLASS()
class OPENWORLD_API UPopulationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	void AddPlayer(AOpenWorldCharacter *Character);
	void RemovePlayer(AOpenWorldCharacter *Character);
	
	AMassSpawner* GetMassSpawner();
	
	void FindOverlapLaneSections(const FVector& Center, float Radius, TArray<FZoneGraphLaneSection>& OverlappingLaneSections);
	void FindOverlapLaneDistance(const FZoneGraphLaneHandle& LaneHandle, const FVector& Center, float Radius, TArray<FZoneGraphLaneSection>& OverlappingLaneSections);
	//void FindRadiusBoundary(const FVector& SegStart, const FVector& SegEnd, const FVector& Center, float Radius, float& SegT);
	TArray<FZoneGraphLaneSection> SubtractLaneSections(const TArray<FZoneGraphLaneSection>& Sections, const TArray<FZoneGraphLaneSection>& SubtractSections);
	void AddOrMergeLaneSection(TArray<FZoneGraphLaneSection>& OverlappingLaneSections, FZoneGraphLaneSection& NewSection);
	
private:
	void ShowDebugCircle(APlayerController *PlayerController);
	void ShowLaneSectionDebug(const TArray<FZoneGraphLaneSection>&);
	
public:
	TMap<TWeakObjectPtr<AOpenWorldCharacter>, bool> OpenWorldCharacters;
	
	TArray<FLaneSectionPopulation> LaneSectionPopulations;
	TArray<FZoneGraphLaneSection> BornPopulationSections;
	TArray<FZoneGraphLaneSection> PopulationOverlapLaneSections;
	
	TWeakObjectPtr<AMassSpawner> MassSpawner;
	int TotalNpcCount = 0;
	
	float ViewRadius = 5000.f;
	float PopulationRadius = 5000.f * 1.2f;
	
	float Accumulator = 0.0f;
	FRWLock PopulationLock;
	
	TAtomic<EPopulationUpdateState> PopulationUpdateState{ EPopulationUpdateState::ReadyToScan };
};

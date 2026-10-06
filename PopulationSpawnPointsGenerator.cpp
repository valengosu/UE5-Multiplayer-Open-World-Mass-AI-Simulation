// Fill out your copyright notice in the Description page of Project Settings.

#include "PopulationSpawnPointsGenerator.h"
#include "MassSpawnLocationProcessor.h"
#include "PopulationSubsystem.h"
#include "ZoneGraphSubsystem.h"

void GeneratePointsForZoneGraphLaneSection(TArray<FVector>& Locations, UWorld* World, float NPCDensity);

void UPopulationSpawnPointsGenerator::Generate(UObject& QueryOwner, TConstArrayView<FMassSpawnedEntityType> EntityTypes,
                                               int32 Count, FFinishedGeneratingSpawnDataSignature& FinishedGeneratingSpawnPointsDelegate) const
{
	const UZoneGraphSubsystem* ZoneGraph = UWorld::GetSubsystem<UZoneGraphSubsystem>(QueryOwner.GetWorld());
	if (ZoneGraph == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No zone graph subsystem found in world"));
		return;
	}

	TArray<FVector> Locations;
	GeneratePointsForZoneGraphLaneSection(Locations, GetWorld(), NPCDensity);
	UE_LOG(LogTemp, Warning, TEXT("LocationSize = %d"), Locations.Num());
	
	if (Locations.IsEmpty())
	{
		//UE_LOG(LogTemp, Error, TEXT("No locations found on zone graphs"));
			TArray<FMassEntitySpawnDataGeneratorResult> EmptyResults;
			FinishedGeneratingSpawnPointsDelegate.Execute(EmptyResults);
			return;
	}

	// Randomize them
	const FRandomStream RandomStream(GetRandomSelectionSeed());
	for (int32 I = 0; I < Locations.Num(); ++I)
	{
		const int32 J = RandomStream.RandHelper(Locations.Num());
		Locations.Swap(I, J);
	}
	
	// Build array of entity types to spawn.
	TArray<FMassEntitySpawnDataGeneratorResult> Results;
	BuildResultsFromEntityTypes(Locations.Num(), EntityTypes, Results);

	const int32 LocationCount = Locations.Num();
	int32 LocationIndex = 0;

	// Distribute points amongst the entities to spawn.
	for (FMassEntitySpawnDataGeneratorResult& Result : Results)
	{
		// @todo: Make separate processors and pass the ZoneGraph locations directly.
		Result.SpawnDataProcessor = UMassSpawnLocationProcessor::StaticClass();
		Result.SpawnData.InitializeAs<FMassTransformsSpawnData>();
		FMassTransformsSpawnData& Transforms = Result.SpawnData.GetMutable<FMassTransformsSpawnData>();

		Transforms.Transforms.Reserve(Result.NumEntities);
		for (int i = 0; i < Result.NumEntities; i++)
		{
			FTransform& Transform = Transforms.Transforms.AddDefaulted_GetRef();
			Transform.SetLocation(Locations[LocationIndex % LocationCount]);
			LocationIndex++;
		}
	}
	
	FinishedGeneratingSpawnPointsDelegate.Execute(Results);
}

void GeneratePointsForZoneGraphLaneSection(TArray<FVector>& Locations, UWorld* World, float NPCDensity)
{
	UPopulationSubsystem* PopulationSubsystem = World->GetSubsystem<UPopulationSubsystem>();
	
	float AllSectionArea = 0.f;
	for (auto LaneSection : PopulationSubsystem->PopulationOverlapLaneSections)
	{
		const FZoneGraphLaneSection Section = LaneSection;
		const AZoneGraphData* Data = World->GetSubsystem<UZoneGraphSubsystem>()->GetZoneGraphData(Section.LaneHandle.DataHandle);
		const FZoneGraphStorage& Storage = Data->GetStorage();
		const FZoneLaneData& Lane = Storage.Lanes[Section.LaneHandle.Index];
		
		AllSectionArea += (Section.EndDistanceAlongLane - Section.StartDistanceAlongLane) * Lane.Width;
	}
	
	AllSectionArea = FMath::Max(AllSectionArea, 0.00001);
	
	const float TotalNPCDensity = PopulationSubsystem->TotalNpcCount / AllSectionArea;
	if (TotalNPCDensity >= NPCDensity * 0.8)
		return;
	
	const FRandomStream RandomStream(FMath::Rand());
	const TArray<FLaneSectionPopulation>& LaneSectionPopulations = PopulationSubsystem->LaneSectionPopulations;
	
	for (auto LaneSectionPopulation : LaneSectionPopulations)
	{
		const FZoneGraphLaneSection Section = LaneSectionPopulation.Section;
		const AZoneGraphData* Data = World->GetSubsystem<UZoneGraphSubsystem>()->GetZoneGraphData(Section.LaneHandle.DataHandle);
		const FZoneGraphStorage& Storage = Data->GetStorage();
		const FZoneLaneData& Lane = Storage.Lanes[Section.LaneHandle.Index];
		
		const float SectionArea = (Section.EndDistanceAlongLane - Section.StartDistanceAlongLane) * Lane.Width;
		const float SectionNPCDensity = LaneSectionPopulation.Count / SectionArea;
		//UE_LOG(LogTemp, Warning, TEXT("SectionNPCDensity = %f"), SectionNPCDensity);
		
		//TODO...zheli shi bug!!!
		//UE_LOG(LogTemp, Warning, TEXT("NPCDensity = %.10f, NPCDensity * 0.8 = %.10f"), NPCDensity, NPCDensity * 0.8f);
		
		if (SectionNPCDensity >= NPCDensity * 0.8)
			continue;
		
		const int32 TargetNPCCount = FMath::RoundToInt(SectionArea * (NPCDensity - SectionNPCDensity));
		//UE_LOG(LogTemp, Warning, TEXT("TargetNPCCount = %d"), TargetNPCCount);
		
		for (int i = 0; i < TargetNPCCount; i++)
		{
			const float Distance = RandomStream.FRandRange(Section.StartDistanceAlongLane,Section.EndDistanceAlongLane);

			FZoneGraphLaneLocation LaneLocation;
			World->GetSubsystem<UZoneGraphSubsystem>()->CalculateLocationAlongLane(Section.LaneHandle, Distance,LaneLocation);

			const FVector Perp = LaneLocation.Direction ^ LaneLocation.Up;
			const float LaneHalfWidth = Lane.Width * 0.5f;

			Locations.Add(LaneLocation.Position + Perp * RandomStream.FRandRange(-LaneHalfWidth, LaneHalfWidth));
		}
	}
}
#include "PopulationSubsystem.h"

#include "EngineUtils.h"
#include "MassSpawner.h"
#include "OpenWorldCharacter.h"
#include "GameFramework/PlayerController.h"
#include "ZoneGraphSubsystem.h"

//extern void GeneratePointsForZoneGraphLaneSection(TArray<FVector>& Locations, UWorld* World);

void UPopulationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*for (const auto& Pair  : OpenWorldCharacters)
	{
		if (Pair.Key.IsValid() == true)
		{
			if (APlayerController* PlayerController = Cast<APlayerController>(Pair.Key.Get()->GetController()))
				ShowDebugCircle( PlayerController);
		}
	}*/
	
	if (GetWorld()->GetNetMode() == NM_Client)
		return;
	
	Accumulator += DeltaTime;
	if (Accumulator < 0.05f)
		return;
	
	Accumulator = 0.f;
	
	if (PopulationUpdateState == EPopulationUpdateState::ReadyToScan)
	{
		TArray<FZoneGraphLaneSection> ViewOverlapLaneSections;
		PopulationOverlapLaneSections.Reset();
		
		for (auto& Pair  : OpenWorldCharacters)
		{
			if (Pair.Key.IsValid() == false)
				continue;
			
			FindOverlapLaneSections(Pair.Key->GetActorLocation(), PopulationRadius, PopulationOverlapLaneSections);
			
			if (Pair.Value == false)
			{
				Pair.Value = true;
				continue;
			}
			
			FindOverlapLaneSections(Pair.Key->GetActorLocation(), ViewRadius, ViewOverlapLaneSections);
		}
		
		BornPopulationSections = SubtractLaneSections(PopulationOverlapLaneSections, ViewOverlapLaneSections);
		PopulationUpdateState = EPopulationUpdateState::Counting;
		return;
	}
	
	else if (PopulationUpdateState == EPopulationUpdateState::ResultReady)
	{
		if (AMassSpawner* MassSpawnerActor = GetMassSpawner())
		{
			MassSpawnerActor->DoSpawning();
			PopulationUpdateState = EPopulationUpdateState::ReadyToScan;	
		}
		return;
	}	
}

TStatId UPopulationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UPopulationSubsystem, STATGROUP_Tickables);
	return Super::GetStatId();
}

void UPopulationSubsystem::AddPlayer(AOpenWorldCharacter *Character)
{
	OpenWorldCharacters.Add(Character, false);
}

void UPopulationSubsystem::RemovePlayer(AOpenWorldCharacter *Character)
{
	OpenWorldCharacters.Remove(Character);
}

AMassSpawner* UPopulationSubsystem::GetMassSpawner()
{
	if (MassSpawner.IsValid() == true)
		return MassSpawner.Get();

	for (TActorIterator<AMassSpawner> It(GetWorld()); It; ++It)
	{
		MassSpawner = *It;
		return *It;
	}

	return nullptr;
}

void UPopulationSubsystem::ShowDebugCircle(APlayerController *PlayerController)
{
	FVector ViewLocation;
	FRotator ViewRotation; 

	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	ViewLocation = PlayerController->GetPawn()->GetActorLocation();
	
	DrawDebugCircle(GetWorld(),ViewLocation, ViewRadius, 64, FColor::Green, false,0.0f,
	0,3.0f,FVector(1, 0, 0),FVector(0, 1, 0),false);
	
	DrawDebugCircle(GetWorld(),ViewLocation, PopulationRadius, 64, FColor::Red, false,0.0f, 
		0,3.0f,FVector(1, 0, 0),FVector(0, 1, 0),false);
}

void UPopulationSubsystem::ShowLaneSectionDebug(const TArray<FZoneGraphLaneSection>& LaneSections)
{
	static const FColor Colors[] =
	{
		FColor::Red,
		FColor::Green,
		FColor::Blue,
		FColor::Yellow,
		FColor::Cyan,
		FColor::Magenta,
		FColor::Orange,
		FColor::Purple
	};
	
	UZoneGraphSubsystem* ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
	
	for (const auto &Lane : LaneSections)
	{
		FZoneGraphLaneLocation StartLocation;
		FZoneGraphLaneLocation EndLocation;

		bool bStartValid = ZoneGraphSubsystem->CalculateLocationAlongLane(Lane.LaneHandle, Lane.StartDistanceAlongLane,StartLocation);
		bool bEndValid = ZoneGraphSubsystem->CalculateLocationAlongLane(Lane.LaneHandle, Lane.EndDistanceAlongLane,EndLocation);
		
		if (bStartValid == true && bEndValid == true)
		{
			int32 ColorIndex = (int)StartLocation.Position.Length() % UE_ARRAY_COUNT(Colors);
			
			DrawDebugSphere(GetWorld(), StartLocation.Position, 20.f,12,Colors[ColorIndex],false,0.f);
			DrawDebugSphere(GetWorld(),EndLocation.Position,20.f,12,Colors[ColorIndex],false,0.f);
		}
	}
	
	//todo....
	//const AZoneGraphData* Data = GetWorld()->GetSubsystem<UZoneGraphSubsystem>()->GetZoneGraphData(ZoneGraphLaneSection.LaneHandle.DataHandle);
	//const FZoneGraphStorage& Storage = Data->GetStorage();
	//const FZoneLaneData& Lane = Storage.Lanes[ZoneGraphLaneSection.LaneHandle.Index];
	
	//int NPCNumber = UPopulationSubsystem::NPCDensity * 
	
	TArray<FVector> Locations;
	//GeneratePointsForZoneGraphLaneSection(Locations, GetWorld());

	for (const auto& Location : Locations)
	{
		DrawDebugSphere(GetWorld(), Location, 10.f,12,FColor::Purple,false,0.f);
	}
}	

void UPopulationSubsystem::FindOverlapLaneSections(const FVector& Center, float Radius, TArray<FZoneGraphLaneSection>& OverlappingLaneSections)
{
	UZoneGraphSubsystem* ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
	
	FZoneGraphTagFilter TagFilter;
	TagFilter.AnyTags.Add(ZoneGraphSubsystem->GetTagByName(TEXT("NPC")));
	
	static TArray<FZoneGraphLaneSection> OutLaneSections;
	OutLaneSections.Reset();
	ZoneGraphSubsystem->FindLaneOverlaps(Center, Radius, TagFilter, OutLaneSections);
	
	//OverlappingLaneSections.Reset();
	for (auto& LaneSection : OutLaneSections)
	{
		FindOverlapLaneDistance(LaneSection.LaneHandle, Center, Radius, OverlappingLaneSections);
	}
}

void UPopulationSubsystem::FindOverlapLaneDistance(const FZoneGraphLaneHandle& LaneHandle, const FVector& Center, float Radius, TArray<FZoneGraphLaneSection>& OverlappingLaneSections)
{
	const AZoneGraphData* Data = GetWorld()->GetSubsystem<UZoneGraphSubsystem>()->GetZoneGraphData(LaneHandle.DataHandle);
	const FZoneGraphStorage& Storage = Data->GetStorage();
	const FZoneLaneData& Lane = Storage.Lanes[LaneHandle.Index];
	
	for (int i = Lane.PointsBegin; i < Lane.PointsEnd - 1; i++)
	{
		const FVector& SegStart = Storage.LanePoints[i];
		const FVector& SegEnd = Storage.LanePoints[i + 1];
		
		const FVector ClosestPt = FMath::ClosestPointOnSegment(Center, SegStart, SegEnd);
		const float DistToSegSq = FVector::DistSquared(Center, ClosestPt);
		const float HalfWidth = 0.5f * Lane.Width;
		
		if (Radius >= HalfWidth && DistToSegSq <= FMath::Square(Radius - HalfWidth))
		{
			FZoneGraphLaneSection Section;
			Section.LaneHandle = LaneHandle;
			Section.StartDistanceAlongLane = Storage.LanePointProgressions[i];
			Section.EndDistanceAlongLane = Storage.LanePointProgressions[i + 1];
			
			OverlappingLaneSections.AddUnique(Section);
		}
	}
}

void UPopulationSubsystem::AddOrMergeLaneSection(TArray<FZoneGraphLaneSection>& OverlappingLaneSections, FZoneGraphLaneSection& NewSection)
{
	for (int i = OverlappingLaneSections.Num() - 1; i >= 0; --i)
	{
		const FZoneGraphLaneSection& Existing = OverlappingLaneSections[i];

		if (Existing.LaneHandle != NewSection.LaneHandle)
			continue;

		bool Overlap = !(NewSection.EndDistanceAlongLane < Existing.StartDistanceAlongLane || NewSection.StartDistanceAlongLane > Existing.EndDistanceAlongLane);

		if (Overlap)
		{
			NewSection.StartDistanceAlongLane = FMath::Min(NewSection.StartDistanceAlongLane,Existing.StartDistanceAlongLane);
			NewSection.EndDistanceAlongLane = FMath::Max(NewSection.EndDistanceAlongLane,Existing.EndDistanceAlongLane);
			OverlappingLaneSections.RemoveAtSwap(i);
		}
	}
	
	OverlappingLaneSections.Add(NewSection);
}

TArray<FZoneGraphLaneSection> UPopulationSubsystem::SubtractLaneSections(const TArray<FZoneGraphLaneSection>& Sections, const TArray<FZoneGraphLaneSection>& SubtractSections)
{
	TArray<FZoneGraphLaneSection> Result;

	for (const FZoneGraphLaneSection& Section : Sections)
	{
		bool bSubtract = false;

		for (const FZoneGraphLaneSection& SubtractSection : SubtractSections)
		{
			if (Section.LaneHandle == SubtractSection.LaneHandle &&
				Section.StartDistanceAlongLane == SubtractSection.StartDistanceAlongLane &&
				Section.EndDistanceAlongLane == SubtractSection.EndDistanceAlongLane)
			{
				bSubtract = true;
				break;
			}
		}

		if (!bSubtract)
		{
			FZoneGraphLaneSection NewSection = Section;
			AddOrMergeLaneSection(Result, NewSection);
		}
	}

	return Result;
}

/*
void UPopulationSubsystem::FindRadiusBoundary(const FVector& SegStart, const FVector& SegEnd, const FVector& Center, float Radius, float& SegT)
{
	FVector LowerStart = FVector::DistSquared(SegStart, Center) <= FVector::DistSquared(SegEnd, Center) ? SegStart : SegEnd;
	FVector HighEnd = FVector::DistSquared(SegStart, Center) <= FVector::DistSquared(SegEnd, Center) ? SegEnd : SegStart;
	FVector SearchMid;
	
	for (int i = 0; i < 12; i++)
	{
		SearchMid = (LowerStart + HighEnd) * 0.5f;
		float OffsetSqr = FVector::DistSquared(SearchMid, Center) - Radius * Radius;
		
		if (OffsetSqr > 0)
		{
			HighEnd = SearchMid;
			continue;
		}
		
		if (OffsetSqr < 0)
		{
			LowerStart = SearchMid;
			continue;
		}
	}
	
	SegT = FVector::Dist(SegStart, SearchMid) / FVector::Dist(SegEnd, SegStart);
}
*/

#include "UPopulationManagementProcessor.h"

#include "MassActorSubsystem.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassExecutionContext.h"
#include "PopulationSubsystem.h"

UPopulationManagementProcessor::UPopulationManagementProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
	bRequiresGameThreadExecution = false;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server);
}

bool UPopulationManagementProcessor::ShouldAllowQueryBasedPruning(const bool bRuntimeMode) const
{
	return false;
}

void UPopulationManagementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FMassZoneGraphLaneLocationFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

int UPopulationManagementProcessor::IsInsideLanes(const FMassZoneGraphLaneLocationFragment& Location, const TArray<FZoneGraphLaneSection>& Sections)
{
	for (int i = 0; i < Sections.Num(); i++)
	{
		if (Location.LaneHandle == Sections[i].LaneHandle &&
			Location.DistanceAlongLane >= Sections[i].StartDistanceAlongLane &&
			Location.DistanceAlongLane <= Sections[i].EndDistanceAlongLane)
		{
			return i;
		}
	}
	return -1;
}

void UPopulationManagementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UPopulationSubsystem* PopulationSubsystem = GetWorld()->GetSubsystem<UPopulationSubsystem>();
	if (PopulationSubsystem == nullptr)
		return;
	
	if (PopulationSubsystem->PopulationUpdateState != EPopulationUpdateState::Counting)
		return;
	
	PopulationSubsystem->LaneSectionPopulations.Reset();
	PopulationSubsystem->TotalNpcCount = 0;
	
	for (auto Section : PopulationSubsystem->BornPopulationSections)
	{
		PopulationSubsystem->LaneSectionPopulations.Add(FLaneSectionPopulation{ Section, 0 });
	}
	
	EntityQuery.ForEachEntityChunk
	(
		Context,
		[&](FMassExecutionContext& Context)
		{
			const TConstArrayView<FMassZoneGraphLaneLocationFragment> LaneLocations = Context.GetFragmentView<FMassZoneGraphLaneLocationFragment>();
			
			for (int32 i = 0; i < Context.GetNumEntities(); ++i)
			{
				const FMassZoneGraphLaneLocationFragment& LaneLocation = LaneLocations[i];
				
				if (IsInsideLanes(LaneLocation, PopulationSubsystem->PopulationOverlapLaneSections) >= 0)
					PopulationSubsystem->TotalNpcCount++;
					
				int index = IsInsideLanes(LaneLocation, PopulationSubsystem->BornPopulationSections);
				if (index >= 0)
				{  
					PopulationSubsystem->LaneSectionPopulations[index].Count++;
					continue;
				}
				
				const FMassEntityHandle EntityHandle = Context.GetEntity(i);
				FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
				
				if (ActorFragment != nullptr && ActorFragment->Get() != nullptr)
					continue;
				
				if (IsInsideLanes(LaneLocation, PopulationSubsystem->PopulationOverlapLaneSections) < 0)
				{
					Context.Defer().DestroyEntity(Context.GetEntity(i));
					continue;
				}
			}
		});
	
	PopulationSubsystem->PopulationUpdateState = EPopulationUpdateState::ResultReady;
}

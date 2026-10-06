#include "UGameEventProcessor.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "GameFramework/Character.h"
#include "MassNPC/FReactionFragment.h"
#include "MassZoneGraphNavigationFragments.h"
#include "UGameEventSystem.h"

UGameEventProcessor::UGameEventProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = false;
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server);
}

void UGameEventProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);

	EntityQuery.AddRequirement<FReactionFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UGameEventProcessor::HandleEvent(UUGameEventSystem *GameEventSystem, FMassEntityManager& EntityManager, const FMassEntityHandle& MassEntityHandle)
{
	const FTransformFragment& TransformFragment = EntityManager.GetFragmentDataChecked<FTransformFragment>(MassEntityHandle);
	FReactionFragment& ReactionFragment = EntityManager.GetFragmentDataChecked<FReactionFragment>(MassEntityHandle);
	const FVector EntityLocation = TransformFragment.GetTransform().GetLocation();
	
	const TArray<FGameEvent>& Events = GameEventSystem->GetReadBuffer();
	
	for (const FGameEvent& Event : Events)
	{
		switch (Event.Type)
		{
			case EGameEventType::GunShoot:
			{
					float DistanceSqr = FVector::DistSquared(EntityLocation, Event.GunShootEvent.Location);
					float RadiusSqr = FMath::Square(Event.GunShootEvent.EffectRadius);
					
					if (DistanceSqr <= RadiusSqr * 10)
					{
						ReactionFragment.EventType = Event.Type;
						ReactionFragment.EventLocation = Event.GunShootEvent.Location;
						ReactionFragment.EffectRadius = Event.GunShootEvent.EffectRadius;
						ReactionFragment.bPending = true;
						ReactionFragment.SourceCharacterHandle = Event.GunShootEvent.VictimEntity;
						
						UE_LOG(LogTemp, Warning, TEXT("[Reaction] Entity[%d:%d] Location=%s"), MassEntityHandle.Index, MassEntityHandle.SerialNumber, *EntityLocation.ToString());
						GameEventSystem->AddActorUpgradeRequest(MassEntityHandle);
					}
			break;
			}

		default:
			break;
		}
	}
}

void UGameEventProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
	if (GameEventSystem == nullptr)
		return;
	
	EntityQuery.ForEachEntityChunk
	(
		Context,
		[&](FMassExecutionContext& ChunkContext)
		{
			for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
			{
				const FMassEntityHandle EntityHandle = ChunkContext.GetEntity(i);
				
				FMassActorFragment* ActorFragment = EntityManager.GetFragmentDataPtr<FMassActorFragment>(EntityHandle);
				if (ActorFragment->Get() == nullptr)
				{
					HandleEvent(GameEventSystem,EntityManager, EntityHandle);	
				}
			}
		}
	);
}

#include "MassBulletHitProcessor.h"

#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassExecutionContext.h"
#include "UGameEventSystem.h"
#include "Components/SphereComponent.h"

UMassBulletHitProcessor::UMassBulletHitProcessor()
{
	bAutoRegisterWithProcessingPhases = true;
	bRequiresGameThreadExecution = true;
	ExecutionFlags = (int32)EProcessorExecutionFlags::Server;
}

void UMassBulletHitProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.Initialize(EntityManager);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassActorFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.RegisterWithProcessor(*this);
}

void UMassBulletHitProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UUGameEventSystem* GameEventSystem = GetWorld()->GetSubsystem<UUGameEventSystem>();
	if (GameEventSystem == nullptr)
		return;

	for (int32 BulletIndex = GameEventSystem->ServerBullets.Num() - 1; BulletIndex >= 0; --BulletIndex)
	{
		TWeakObjectPtr<AABullet> Bullet = GameEventSystem->ServerBullets[BulletIndex];
		
		EntityQuery.ForEachEntityChunk(Context, [&](FMassExecutionContext& ChunkContext)
		{
			if (Bullet.IsValid() == false)
				return;
			
			TConstArrayView<FTransformFragment> TransformFragments = ChunkContext.GetFragmentView<FTransformFragment>();
			TConstArrayView<FAgentRadiusFragment> RadiusFragments = ChunkContext.GetFragmentView<FAgentRadiusFragment>();
			TConstArrayView<FMassActorFragment> ActorFragments = ChunkContext.GetFragmentView<FMassActorFragment>();

			for (int32 i = 0; i < ChunkContext.GetNumEntities(); ++i)
			{
				if (ActorFragments[i].Get() != nullptr)
					continue;
				
				const FVector EntityLocation = TransformFragments[i].GetTransform().GetLocation();

				const FVector ClosestPoint = FMath::ClosestPointOnSegment(EntityLocation, Bullet->PreviousLocation, Bullet->GetActorLocation());
				const float DistanceSqr = FVector::DistSquared2D(EntityLocation, ClosestPoint);
				const float CollisionRadius  = Bullet->Collision->GetScaledSphereRadius() + RadiusFragments[i].Radius;
				
				if (DistanceSqr <= FMath::Square(CollisionRadius))
				{
					FGameEvent Event;
					Event.Type = EGameEventType::GunShoot;
					Event.GunShootEvent.VictimEntity = ChunkContext.GetEntity(i);
					Event.GunShootEvent.EffectRadius = 300.f;
					Event.GunShootEvent.Location = ClosestPoint;
					GameEventSystem->AddGameEvent(Event);
					
					UE_LOG(LogTemp, Warning, TEXT("GunShoot Mass: EntityLocation=%s, HitLocation=%s, Distance=%.2f"), 
													*EntityLocation.ToString(), *ClosestPoint.ToString(), FMath::Sqrt(DistanceSqr));
					
					Bullet->Destroy();
					break;
				}
			}
		});
		
		if (Bullet.IsValid() == true)
		{
			Bullet->PreviousLocation = Bullet->GetActorLocation();
		}
		else
		{
			GameEventSystem->ServerBullets.RemoveAtSwap(BulletIndex);
		}
	}
}
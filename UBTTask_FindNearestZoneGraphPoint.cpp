#include "UBTTask_FindNearestZoneGraphPoint.h"
#include "AIController.h"
#include "MassNPCCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "ZoneGraphSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_FindNearestZoneGraphPoint::UBTTask_FindNearestZoneGraphPoint()
{
	NodeName = TEXT("Find Nearest ZoneGraph Point");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FindNearestZoneGraphPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	check(AIController);
	
	APawn* Pawn = AIController->GetPawn();
	check(Pawn);

	UZoneGraphSubsystem* ZoneGraphSubsystem = Pawn->GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
	const FVector ActorLocation = Pawn->GetActorLocation();
	
	FZoneGraphLaneLocation LaneLocation;
	float DistanceSqr = 0.f;
	
	for (int i = 0; i < 4; i++)
	{
		float SearchDistance = 1000 * (i + 1);
		const FBox QueryBounds(ActorLocation - FVector(SearchDistance),ActorLocation + FVector(SearchDistance));
		
		bool bFound = ZoneGraphSubsystem->FindNearestLane(QueryBounds, TagFilter, LaneLocation,DistanceSqr);
		if (bFound == false)
			continue;
		
		EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(LaneLocation.Position, AcceptanceRadius, false);
		if (MoveResult == EPathFollowingRequestResult::Failed)
			continue;
		
		AMassNPCCharacter* Character = Cast<AMassNPCCharacter>(Pawn);
		Character->ReturnToMassLaneLocation = LaneLocation;
		
		return EBTNodeResult::InProgress;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Failed to find a valid ZoneGraph lane near the actor location!!!"));
	return EBTNodeResult::Failed;
}

void UBTTask_FindNearestZoneGraphPoint::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	check(AIController);

	APawn* Pawn = AIController->GetPawn();
	check(Pawn);
	
	AMassNPCCharacter* Character = Cast<AMassNPCCharacter>(Pawn);
	check(Character);
	
	const float Distance = FVector::Dist2D(Pawn->GetActorLocation(), Character->ReturnToMassLaneLocation.Position);
	const float SlowDownDistance = 300.f;
	const float MinSpeed = 50.f;
	const float MaxSpeed = 200.f;

	const float Alpha = FMath::Clamp(Distance / SlowDownDistance, 0.f, 1.f);
	const float Speed = FMath::Lerp(MinSpeed, MaxSpeed, Alpha);
	
	Character->GetCharacterMovement()->Velocity = Character->GetVelocity().GetSafeNormal() * Speed;
	
	const float DistanceSqr = FVector::DistSquared2D(Pawn->GetActorLocation(), Character->ReturnToMassLaneLocation.Position);
	if (DistanceSqr <= FMath::Square( AcceptanceRadius * 1.1f))
	{
		AIController->StopMovement();
		Character->MassState = EMassState::ReadyToHandBack;
		
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

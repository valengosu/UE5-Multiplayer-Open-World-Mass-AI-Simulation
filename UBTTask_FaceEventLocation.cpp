#include "UBTTask_FaceEventLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

UBTTask_FaceEventLocation::UBTTask_FaceEventLocation()
{
	NodeName = TEXT("Face Event Location");
	bNotifyTick = true;

	EventLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FaceEventLocation, EventLocationKey));
}

EBTNodeResult::Type UBTTask_FaceEventLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;
}

void UBTTask_FaceEventLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	check(AIController);

	APawn* Pawn = AIController->GetPawn();
	check(Pawn);

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	check(Blackboard);

	const FVector EventLocation = Blackboard->GetValueAsVector(EventLocationKey.SelectedKeyName);

	const FVector PawnLocation = Pawn->GetActorLocation();
	FVector Direction = EventLocation - PawnLocation;
	Direction.Z = 0.0f;

	FRotator TargetRotation = Direction.Rotation();
	FRotator CurrentRotation = Pawn->GetActorRotation();
	
	TargetRotation.Pitch = 0.0f;
	TargetRotation.Roll = 0.0f;
	CurrentRotation.Pitch = 0.0f;
	CurrentRotation.Roll = 0.0f;

	const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, InterpSpeed);
	Pawn->SetActorRotation(NewRotation);
	
	const float YawError = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRotation.Yaw,TargetRotation.Yaw));
	if (YawError <= AcceptableYawError)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
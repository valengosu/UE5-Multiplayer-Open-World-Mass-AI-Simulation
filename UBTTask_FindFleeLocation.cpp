#include "UBTTask_FindFleeLocation.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

UBTTask_FindFleeLocation::UBTTask_FindFleeLocation()
{
	NodeName = TEXT("Find Flee Location");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_FindFleeLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FFleeLocationTaskMemory* Memory = reinterpret_cast<FFleeLocationTaskMemory*>(NodeMemory);
	Memory->ElapsedTime = 0.0f;
	
	AAIController* AIController = OwnerComp.GetAIOwner();
	ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
	
	Memory->FleeStartLocation = Character->GetActorLocation();
	Memory->InitialMaxWalkSpeed = Character->GetCharacterMovement()->MaxWalkSpeed;
	return EBTNodeResult::InProgress;
}

void UBTTask_FindFleeLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	check(AIController);
	
	APawn* Pawn = AIController->GetPawn();
	check(Pawn);
	
	ACharacter* Character = Cast<ACharacter>(Pawn);
	
	float FleeTime = 5.f;
	float SlowDownStartTime = 2.f;
	
	FFleeLocationTaskMemory* Memory = reinterpret_cast<FFleeLocationTaskMemory*>(NodeMemory);
	if (Memory->ElapsedTime > FleeTime)
	{
		Memory->ElapsedTime = 0.f;
		AIController->StopMovement();
		Character->GetCharacterMovement()->MaxWalkSpeed = Memory->InitialMaxWalkSpeed;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	
	Memory->ElapsedTime += DeltaSeconds;
	
	float Alpha = FMath::Clamp((Memory->ElapsedTime - SlowDownStartTime) / (FleeTime - SlowDownStartTime),0.f,1.f);
	float CurrentSpeed = FMath::Lerp(Memory->InitialMaxWalkSpeed, 0.1f, Alpha);
	Character->GetCharacterMovement()->MaxWalkSpeed = CurrentSpeed;
	
	if (AIController->GetMoveStatus() == EPathFollowingStatus::Idle)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Pawn);
		check(NavSystem);
		
		for (int i = 0; i < 60; i++)
		{
			FNavLocation RandomLocation;
			bool bFound = NavSystem->GetRandomReachablePointInRadius(Pawn->GetActorLocation(), FleeDistance, RandomLocation);
			if (bFound == true)
			{
				if (FVector::Dist2D(RandomLocation.Location, Memory->FleeStartLocation) < FleeDistance * 0.6f)
					continue;
				
				FVector MoveDirection = (RandomLocation.Location - Character->GetActorLocation()).GetSafeNormal2D();
				FVector FromStartDirection = (Pawn->GetActorLocation() - Memory->FleeStartLocation).GetSafeNormal2D();
				
				if (!FromStartDirection.IsNearlyZero() && FVector::DotProduct(MoveDirection, FromStartDirection) < 0.0f)
					continue;
				
				AIController->MoveToLocation(RandomLocation.Location, 2.f);
				return;
			}
		}
		
		AIController->StopMovement();
		Character->GetCharacterMovement()->MaxWalkSpeed = Memory->InitialMaxWalkSpeed;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

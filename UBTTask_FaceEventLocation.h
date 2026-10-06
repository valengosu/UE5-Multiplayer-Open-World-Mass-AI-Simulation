#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "UBTTask_FaceEventLocation.generated.h"

UCLASS()
class UBTTask_FaceEventLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FaceEventLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector EventLocationKey;

	//UPROPERTY(EditAnywhere, Category = "Rotation")
	const float InterpSpeed = 8.0f;

	//UPROPERTY(EditAnywhere, Category = "Rotation")
	float AcceptableYawError = 1.0f;
};
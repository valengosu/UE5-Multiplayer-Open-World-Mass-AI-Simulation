#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "UBTTask_FindFleeLocation.generated.h"

USTRUCT()
struct FFleeLocationTaskMemory
{
	GENERATED_BODY()
	float ElapsedTime = 0.0f;
	float InitialMaxWalkSpeed = 0.f;
	FVector FleeStartLocation = FVector::ZeroVector;
};


UCLASS()
class OPENWORLD_API UBTTask_FindFleeLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindFleeLocation();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FFleeLocationTaskMemory);
	}
	
	const float FleeDistance = 600.0f;
};
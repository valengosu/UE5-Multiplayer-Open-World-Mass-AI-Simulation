#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ZoneGraphTypes.h"
#include "UBTTask_FindNearestZoneGraphPoint.generated.h"

UCLASS()
class UBTTask_FindNearestZoneGraphPoint : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_FindNearestZoneGraphPoint();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
public:
	const float AcceptanceRadius = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = "ZoneGraph")
	FZoneGraphTagFilter TagFilter;
};
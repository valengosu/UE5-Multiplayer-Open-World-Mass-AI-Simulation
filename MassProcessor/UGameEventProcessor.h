#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "UGameEventSystem.h"
#include "UGameEventProcessor.generated.h"

UCLASS()
class OPENWORLD_API UGameEventProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UGameEventProcessor();
	
protected:
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	void HandleEvent(UUGameEventSystem *GameEventSystem, FMassEntityManager& EntityManager, const FMassEntityHandle& MassEntityHandle);
private:
	FMassEntityQuery EntityQuery;
};
#pragma once

#include "CoreMinimal.h"
#include "MassObserverProcessor.h"
#include "UReplicationAgentCleanup.generated.h"

UCLASS()
class OPENWORLD_API UReplicationAgentCleanup : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UReplicationAgentCleanup();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;

	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
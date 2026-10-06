#pragma once
#include "MassProcessor.h"
#include "ClientMassDebugProcessor.generated.h"

UCLASS()
class UClientMassDebugProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UClientMassDebugProcessor();

protected:
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	FMassEntityQuery EntityQuery;
};
